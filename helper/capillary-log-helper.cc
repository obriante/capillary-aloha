/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#include "capillary-log-helper.h"

#include <ns3/assert.h>
#include <ns3/callback.h>
#include <ns3/log.h>
#include <ns3/log-macros-disabled.h>
#include <ns3/nstime.h>
#include <ns3/packet.h>
#include <ns3/pcap-file-wrapper.h>
#include <ns3/simulator.h>
#include <ns3/fsaloha-mac.h>
#include <sstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CapillaryLogHelper");

/**
 * @brief Write a packet in a PCAP file
 * @param file the output file
 * @param packet the packet
 */
static void
PcapSniffCapillary (Ptr<PcapFileWrapper> file, Ptr<const Packet> packet)
{
  file->Write (Simulator::Now (), packet);
}

/**
 * @brief Output an ascii line representing the Transmit event (with context)
 * @param stream the output stream
 * @param context the context
 * @param p the packet
 */
static void
AsciiCapillaryMacTransmitSinkWithContext (
  Ptr<OutputStreamWrapper> stream,
  std::string context,
  Ptr<const Packet> p)
{
  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " " << context << " " << *p << std::endl;
}


CapillaryLogHelper::CapillaryLogHelper ()
{
  NS_LOG_FUNCTION (this);
}

CapillaryLogHelper::~CapillaryLogHelper ()
{
  NS_LOG_FUNCTION (this);
}

void CapillaryLogHelper::EnableLogComponents (enum LogLevel level)
{
  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnable ("CapillaryPhy", level);
  LogComponentEnable ("CapillaryMac", level);
  LogComponentEnable ("CapillaryController", level);
  LogComponentEnable ("CapillaryNetDevice", level);
  LogComponentEnable ("CapillaryEnergyModel", level);
  LogComponentEnable ("SensorApplication", level);
  LogComponentEnable ("BasicController", level);
  LogComponentEnable ("ResidualEnergyController", level);
  LogComponentEnable ("CapillaryPhyIdeal", level);
  LogComponentEnable ("FsalohaMac", level);
  Packet::EnablePrinting ();
  Packet::EnableChecking ();
}

void CapillaryLogHelper::EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename)
{
  NS_LOG_FUNCTION (this << prefix << nd << promiscuous << explicitFilename);


  //
  // All of the Pcap enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.
  //

  // In the future, if we create different NetDevice types, we will
  // have to switch on each type below and insert into the right
  // NetDevice type
  //
  Ptr<CapillaryNetDevice> device = nd->GetObject<CapillaryNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("CapillaryLogHelper::EnablePcapInternal(): Device " << device << " not of type ns3::CapillaryNetDevice");
      return;
    }

  PcapHelper pcapHelper;

  std::string filename;
  if (explicitFilename)
    {
      filename = prefix;
    }
  else
    {
      filename = pcapHelper.GetFilenameFromDevice (prefix, device);
    }

  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile (filename, std::ios::out,
                                                     PcapHelper::DLT_NULL);

  if (promiscuous == true)
    {
      device->GetMac ()->TraceConnectWithoutContext ("PromiscSniffer", MakeBoundCallback (&PcapSniffCapillary, file));

    }
  else
    {
      device->GetMac ()->TraceConnectWithoutContext ("Sniffer", MakeBoundCallback (&PcapSniffCapillary, file));
    }
}

void CapillaryLogHelper::EnableAsciiInternal (Ptr<OutputStreamWrapper> stream, std::string prefix, Ptr<NetDevice> nd, bool explicitFilename)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (stream);
  NS_ASSERT (nd);

  uint32_t nodeid = nd->GetNode ()->GetId ();
  uint32_t deviceid = nd->GetIfIndex ();
  std::ostringstream oss;

  Ptr<CapillaryNetDevice> device = nd->GetObject<CapillaryNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("CapillaryLogHelper::EnableAsciiInternal(): Device " << device << " not of type ns3::CapillaryNetDevice");
      return;
    }

  //
  // Our default trace sinks are going to use packet printing, so we have to
  // make sure that is turned on.
  //
  Packet::EnablePrinting ();

  //
  // If we are not provided an OutputStreamWrapper, we are expected to create
  // one using the usual trace filename conventions and do a Hook*WithoutContext
  // since there will be one file per context and therefore the context would
  // be redundant.
  //
  if (stream == 0)
    {
      //
      // Set up an output stream object to deal with private ofstream copy
      // constructor and lifetime issues.  Let the helper decide the actual
      // name of the file given the prefix.
      //
      AsciiTraceHelper asciiTraceHelper;

      std::string filename;
      if (explicitFilename)
        {
          filename = prefix;
        }
      else
        {
          filename = asciiTraceHelper.GetFilenameFromDevice (prefix, device);
        }

      Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream (filename);

      // Ascii traces typically have "+", '-", "d", "r", and sometimes "t"
      // The Mac and Phy objects have the trace sources for these
      //

      asciiTraceHelper.HookDefaultReceiveSinkWithoutContext<CapillaryNetDevice> (device, "MacRx", theStream);

      device->GetMac ()->TraceConnectWithoutContext ("MacTx", MakeBoundCallback (&AsciiCapillaryMacTransmitSinkWithContext, theStream));

      asciiTraceHelper.HookDefaultEnqueueSinkWithoutContext<CapillaryNetDevice> (device, "MacTxEnqueue", theStream);
      asciiTraceHelper.HookDefaultDequeueSinkWithoutContext<CapillaryNetDevice> (device, "MacTxDequeue", theStream);
      asciiTraceHelper.HookDefaultDropSinkWithoutContext<CapillaryNetDevice> (device, "MacTxDrop", theStream);

      return;
    }

  //
  // If we are provided an OutputStreamWrapper, we are expected to use it, and
  // to provide a context.  We are free to come up with our own context if we
  // want, and use the AsciiTraceHelper Hook*WithContext functions, but for
  // compatibility and simplicity, we just use Config::Connect and let it deal
  // with the context.
  //
  // Note that we are going to use the default trace sinks provided by the
  // ascii trace helper.  There is actually no AsciiTraceHelper in sight here,
  // but the default trace sinks are actually publicly available static
  // functions that are always there waiting for just such a case.
  //


  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::CapillaryNetDevice/Mac/MacRx";
  device->GetMac ()->TraceConnect ("MacRx", oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultReceiveSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::CapillaryNetDevice/Mac/MacTx";
  device->GetMac ()->TraceConnect ("MacTx", oss.str (), MakeBoundCallback (&AsciiCapillaryMacTransmitSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::CapillaryNetDevice/Mac/MacTxEnqueue";
  device->GetMac ()->TraceConnect ("MacTxEnqueue", oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultEnqueueSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::CapillaryNetDevice/Mac/MacTxDequeue";
  device->GetMac ()->TraceConnect ("MacTxDequeue", oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDequeueSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::CapillaryNetDevice/Mac/MacTxDrop";
  device->GetMac ()->TraceConnect ("MacTxDrop", oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDropSinkWithContext, stream));

}

void CapillaryLogHelper::EnableAsciiInternal (Ptr<OutputStreamWrapper> stream, Ptr<CapillaryEnergyModel> device)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (stream);
  NS_ASSERT_MSG (device, "CapillaryLogHelper::EnableAsciiInternal(): Device " << device << " not of type ns3::CapillaryEnergyModel");

  uint32_t nodeid = device->GetNode ()->GetId ();
  std::ostringstream oss;

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/$ns3::CapillaryEnergyModel/TotalEnergyConsumption";
  device->TraceConnect ("TotalEnergyConsumption", oss.str (), MakeBoundCallback (&CapillaryTracer::DefaultEnergyConsumptionSinkWithContext, stream));

}

void CapillaryLogHelper::EnableAsciiInternal (Ptr<OutputStreamWrapper> stream, Ptr<CapillaryNetDevice> device)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (stream);
  NS_ASSERT_MSG (device, "CapillaryLogHelper::EnableAsciiInternal(): Device " << device << " not of type ns3::CapillaryNetDevice");

  uint32_t nodeid = device->GetNode ()->GetId ();
  uint32_t deviceid = device->GetIfIndex ();
  std::ostringstream oss;

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::CapillaryNetDevice/Mac/DcrStatus";
  device->GetMac ()->TraceConnect ("DcrStatus", oss.str (), MakeBoundCallback (&CapillaryTracer::DefaultDataCollectionRoundSinkWithContext, stream));


  Ptr<FsalohaMac> mac = DynamicCast<FsalohaMac> (device->GetMac ());
  if (mac)
    {
      oss.str ("");
      oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::CapillaryNetDevice/Mac/Frames";
      mac->TraceConnect ("Frames", oss.str (), MakeBoundCallback (&CapillaryTracer::DefaultFramesSinkWithContext, stream));
    }

}

void CapillaryLogHelper::EnableAsciiInternal (Ptr<OutputStreamWrapper> stream, Ptr<EnergySource> nd)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (stream);
  NS_ASSERT (nd);

  uint32_t nodeid = nd->GetNode ()->GetId ();
  std::ostringstream oss;

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/$ns3::EnergySource/RemainingEnergy";
  nd->TraceConnect ("RemainingEnergy", oss.str (), MakeBoundCallback (&CapillaryTracer::DefaultEnergySourceSinkWithContext, stream));
}


} /* namespace ns3 */


