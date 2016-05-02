/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Universita' Mediterranea di Reggio Calabria (UNIRC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Orazio Briante <orazio.briante@unirc.it>
 */

#include <ns3/core-module.h>
#include <ns3/mobility-module.h>
#include <ns3/spectrum-module.h>
#include <ns3/energy-module.h>
#include <ns3/network-module.h>
#include <ns3/capillary-network-module.h>
#include <ns3/capillary-aloha-module.h>
#include <ns3/config-store-module.h>
#include <ns3/internet-module.h>
#include <ns3/sixlowpan-module.h>
#include <ns3/applications-module.h>

#include <sys/types.h>
#include <sys/stat.h>

using namespace ns3;

class SimulationConfig : public Object
{
public:
  /** Simulation */
  Time stopAt;

  /** Network */
  uint32_t nDevices;

  /** Channel */
  double txPower;
  uint32_t channelNumber;

  static TypeId GetTypeId (void)
  {
    static TypeId tid =
      TypeId ("ns3::SimulationConfig").SetParent<Object> ()
      .AddAttribute ("txPower", "The transmission power", DoubleValue (0.1),
                     MakeDoubleAccessor (&SimulationConfig::txPower),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("channelNumber", "The channnel number",
                     UintegerValue (1),
                     MakeUintegerAccessor (&SimulationConfig::channelNumber),
                     MakeUintegerChecker<uint32_t> ())
      .AddAttribute ("stopAt", "The simulation time length", TimeValue (Seconds (10)),
                     MakeTimeAccessor (&SimulationConfig::stopAt),
                     MakeTimeChecker ())
      .AddAttribute ("devices", "The number of devices into the network",
                     UintegerValue (1),
                     MakeUintegerAccessor (&SimulationConfig::nDevices),
                     MakeUintegerChecker<uint32_t> ());
    return tid;
  }

};

NS_OBJECT_ENSURE_REGISTERED (SimulationConfig);

int main (int argc, char *argv[])
{
  std::cout << "Simulation Started." << std::endl;

  std::string configPath = "src/capillary-aloha/examples/";
  std::string savePath = "src/capillary-aloha/examples/";

  std::string configFile = configPath + "capillary-aloha-example.xml";

  bool debug = false;
  bool defaultConfig = false;

  CapillaryLogHelper logger = CapillaryLogHelper ();

  CommandLine cmd;
  cmd.AddValue ("debug", "Shows output debug.", debug);
  cmd.AddValue ("config", "The configuration file name to use", configFile);
  cmd.AddValue ("save_path", "The output path to save log", savePath);
  cmd.AddValue ("default", "To save the config file", defaultConfig);
  cmd.Parse (argc, argv);

  if (debug)
    {
      logger.EnableLogComponents (LOG_LEVEL_DEBUG);
    }

  // input config store: txt format
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (configFile));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));

  if (defaultConfig)
    {
      Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
    }
  else
    {
      Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
    }

  ConfigStore config;

  config.ConfigureDefaults ();

  Ptr<SimulationConfig> myConfig = CreateObject<SimulationConfig> ();
  NS_ASSERT (myConfig);

  //config.ConfigureAttributes();

  NodeContainer devices;
  devices.Create (myConfig->nDevices + 1);

  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator");
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (devices);

  // iterate our nodes and print their position.
  for (NodeContainer::Iterator j = devices.Begin ();
       j != devices.End (); ++j)
    {
      Ptr<Node> object = *j;
      Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
      NS_ASSERT (position != 0);
      Vector pos = position->GetPosition ();
      std::cout << "x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z << std::endl;
    }

  // Each device must be attached to the same channel
  SpectrumChannelHelper channelHelper = SpectrumChannelHelper::Default ();
  Ptr<SpectrumChannel> channel = channelHelper.Create ();

  // for the noise, we use the Power Spectral Density of thermal noise
  // at room temperature. The value of the PSD will be constant over the band of interest.
  const double k = 1.381e-23;               //Boltzmann's constant
  const double T = 290;               // temperature in Kelvin
  double noisePsdValue = k * T;               // watts per hertz

  WifiSpectrumValue5MhzFactory sf;
  Ptr<SpectrumValue> txPsd = sf.CreateTxPowerSpectralDensity (myConfig->txPower, myConfig->channelNumber);
  Ptr<SpectrumValue> noisePsd = sf.CreateConstant (noisePsdValue);

  CapillaryNetDeviceHelper deviceHelper = CapillaryNetDeviceHelper ();
  deviceHelper.SetChannel (channel);
  deviceHelper.SetTxPowerSpectralDensity (txPsd);
  deviceHelper.SetNoisePowerSpectralDensity (noisePsd);
  deviceHelper.SetControllerTypeId ("ns3::BasicController");
  NetDeviceContainer capillaryDevices = deviceHelper.Install (devices);

  uint32_t coordinatorIndex = (myConfig->nDevices / 2) + 1;
  Ptr<CapillaryNetDevice> coordinator = deviceHelper.SetCoordinator (capillaryDevices.Get (coordinatorIndex));

  /* energy source */
  BasicEnergySourceHelper basicSourceHelper;
  EnergySourceContainer sources = basicSourceHelper.Install (devices);

  CapillaryEnergyModelHelper capillaryEnergyModelHelper = CapillaryEnergyModelHelper ();

  DeviceEnergyModelContainer energyModels =
    capillaryEnergyModelHelper.Install (capillaryDevices, sources);

  SensorApplicationHelper sensor = SensorApplicationHelper ();
  for (uint32_t i = 0; i < devices.GetN (); i++)
    {
      if (i != coordinatorIndex)
        {
          ApplicationContainer sensors = sensor.Install (devices.Get (i));
          sensors.Start (Seconds (0));
          sensors.Stop (myConfig->stopAt);
        }
    }

  std::ostringstream outputSuffix;
  Ptr<FsalohaMac> mac = DynamicCast<FsalohaMac> (coordinator->GetMac ());
  NS_ASSERT (mac);
  outputSuffix << "-" << myConfig->nDevices << "_Devs-" << mac->GetNSlots () << "_Slots-" << myConfig->stopAt.GetSeconds () << "_sec";

  AsciiTraceHelper ascii;
  logger.EnableAsciiAll (ascii.CreateFileStream (savePath + "Packets" + outputSuffix.str ()));
  logger.EnableEnergyAscii (ascii.CreateFileStream (savePath + "Energy" + outputSuffix.str ()), energyModels);
  logger.EnableDCRAscii (ascii.CreateFileStream (savePath + "DCR" + outputSuffix.str ()), capillaryDevices);

  Simulator::Stop (myConfig->stopAt);

  Simulator::Run ();
  Simulator::Destroy ();

  std::cout << "Simulation Stopped." << std::endl;
  return 0;
}

