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

#include <iostream>

#include <ns3/core-module.h>
#include <ns3/mobility-module.h>
#include <ns3/spectrum-module.h>
#include <ns3/energy-module.h>
#include <ns3/network-module.h>
#include <ns3/capillary-network-module.h>
#include <ns3/config-store-module.h>
#include <ns3/applications-module.h>

#include <ns3/internet-module.h>
#include <ns3/sixlowpan-module.h>


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("fsaloha-capillary-test");

class FsalohaCapillaryTestCase : public TestCase
{
public:
  FsalohaCapillaryTestCase ();
  virtual ~FsalohaCapillaryTestCase ();

private:
  virtual void DoRun (void);

};

FsalohaCapillaryTestCase::FsalohaCapillaryTestCase () :
  TestCase ("Test FSALOHA Capillary classes")
{
}

FsalohaCapillaryTestCase::~FsalohaCapillaryTestCase ()
{
}

void FsalohaCapillaryTestCase::DoRun (void)
{
  NS_LOG_UNCOND ("Started..");

  CapillaryLogHelper logger = CapillaryLogHelper ();
  logger.EnableLogComponents (LOG_LEVEL_DEBUG);

  std::string savePath = "src/capillary-network/test/";

  // input config store: txt format
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (savePath + "fsaloha-capillary-test.xml"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));

  ConfigStore config;
  config.ConfigureDefaults ();


  uint32_t coordinatorPos = 5;
  uint32_t nDevices = 9;
  uint64_t nSlots = nDevices;
  double max = 1425 / 2;
  double min = -1425 / 2;

  Time stopTime = Seconds (10);


  NodeContainer devices;
  devices.Create (nDevices + 1);

  MobilityHelper mobility;


  // setup the grid itself: objects are layed out
  // started from (min, max) with 3 objects per row,
  // the x interval between each object is 20 meters
  // and the y interval between each object is 20 meters
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (min),
                                 "MinY", DoubleValue (max),
                                 "DeltaX", DoubleValue (20.0),
                                 "DeltaY", DoubleValue (20.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

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

  WifiSpectrumValue5MhzFactory sf;

  double txPower = 0.1; // Watts
  uint32_t channelNumber = 1;
  Ptr<SpectrumValue> txPsd =  sf.CreateTxPowerSpectralDensity (txPower, channelNumber);

  // for the noise, we use the Power Spectral Density of thermal noise
  // at room temperature. The value of the PSD will be constant over the band of interest.
  const double k = 1.381e-23; //Boltzmann's constant
  const double T = 290; // temperature in Kelvin
  double noisePsdValue = k * T; // watts per hertz
  Ptr<SpectrumValue> noisePsd = sf.CreateConstant (noisePsdValue);

  CapillaryNetDeviceHelper deviceHelper = CapillaryNetDeviceHelper ();
  deviceHelper.SetChannel (channel);
  deviceHelper.SetTxPowerSpectralDensity (txPsd);
  deviceHelper.SetNoisePowerSpectralDensity (noisePsd);
  deviceHelper.SetPhyAttribute ("Rate", DataRateValue (DataRate ("250kbps")));
  deviceHelper.SetMacAttribute ("slots", UintegerValue (nSlots));
  NetDeviceContainer capillaryDevices = deviceHelper.Install (devices);
  Ptr<CapillaryNetDevice> coordinator = deviceHelper.SetCoordinator (capillaryDevices.Get (coordinatorPos));

  /* energy source */
  BasicEnergySourceHelper energySourceHelper;

  /* configure energy source */
  //energySourceHelper.Set ("NominalCapacity", DoubleValue (20.0));

  /* install source */
  EnergySourceContainer sources = energySourceHelper.Install (devices);

  CapillaryEnergyModelHelper capillaryEnergyModelHelper = CapillaryEnergyModelHelper ();

  DeviceEnergyModelContainer energyModels = capillaryEnergyModelHelper.Install (capillaryDevices, sources);


  SixLowPanHelper sixlowpan;
  sixlowpan.SetDeviceAttribute ("ForceEtherType", BooleanValue (true) );
  NetDeviceContainer six1 = sixlowpan.Install (capillaryDevices);

  InternetStackHelper internetv6;
  internetv6.Install (devices);

  NS_LOG_INFO ("Create networks and assign IPv6 Addresses.");
  Ipv6AddressHelper ipv6;
  ipv6.SetBase (Ipv6Address ("2001:1::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer i1 = ipv6.Assign (six1);
  i1.SetForwarding (coordinatorPos, true);
  i1.SetDefaultRouteInAllNodes (coordinatorPos);

  SensorApplicationHelper sensor = SensorApplicationHelper ();
  sensor.SetAttribute ("RandomStream", StringValue ("ns3::ConstantRandomVariable[Constant=0.3]"));
  sensor.SetAttribute ("PacketSize", UintegerValue (140));
  for (uint32_t i = 0; i < devices.GetN (); i++)
    {
      if (i != 5)
        {
          ApplicationContainer sensors = sensor.Install (devices.Get (i));
          sensors.Start (Seconds (0));
          sensors.Stop (stopTime);
        }
    }

  AsciiTraceHelper ascii;
  logger.EnableAsciiAll (ascii.CreateFileStream (savePath + "fsaloha-capillary-packet"));
  logger.EnableEnergyAscii (ascii.CreateFileStream (savePath + "fsaloha-capillary-energy"), energyModels);
  logger.EnableDCRAscii (ascii.CreateFileStream (savePath + "fsaloha-capillary-DCR"), capillaryDevices);

  logger.EnablePcapAll (savePath + "fsaloha-capillary-test");

  Simulator::Stop (stopTime);

  config.ConfigureAttributes ();

  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_UNCOND ("Stop.");
}

// ==============================================================================
class FsalohaCapillaryTestSuite : public TestSuite
{
public:
  FsalohaCapillaryTestSuite ();
};

FsalohaCapillaryTestSuite::FsalohaCapillaryTestSuite () :
  TestSuite ("fsaloha-capillary-test", UNIT)
{
  AddTestCase (new FsalohaCapillaryTestCase, TestCase::QUICK);
}

static FsalohaCapillaryTestSuite fsalohaCapillaryTestSuite;
