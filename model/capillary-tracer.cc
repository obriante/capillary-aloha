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

#include "capillary-tracer.h"

#include <ns3/assert.h>
#include <ns3/nstime.h>
#include <ns3/simulator.h>
#include <ns3/trace-helper.h>
#include <iterator>
#include <sstream>


namespace ns3 {


CapillaryTracer::CapillaryTracer ()
{
}
CapillaryTracer::~CapillaryTracer ()
{
}

void
CapillaryTracer::EnableDCRAscii (std::string fileName, NetDeviceContainer n)
{
  for (NetDeviceContainer::Iterator i = n.Begin (); i != n.End (); ++i)
    {
      Ptr<CapillaryNetDevice> dev = DynamicCast<CapillaryNetDevice> (*i);
      NS_ASSERT (dev);

      std::ostringstream outputSuffix;
      outputSuffix << fileName << "." << dev->GetNode ()->GetId () << ".tr";
      AsciiTraceHelper ascii;
      Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (outputSuffix.str ());
      EnableAsciiInternal (stream, dev);
    }
}

void
CapillaryTracer::EnableEnergyAscii (std::string fileName, DeviceEnergyModelContainer n)
{
  for (DeviceEnergyModelContainer::Iterator i = n.Begin (); i != n.End (); ++i)
    {
      Ptr<CapillaryEnergyModel> dev = DynamicCast<CapillaryEnergyModel> (*i);
      NS_ASSERT (dev);
      std::ostringstream outputSuffix;
      outputSuffix << fileName << "." << dev->GetNode ()->GetId () << ".tr";
      AsciiTraceHelper ascii;
      Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (outputSuffix.str ());
      EnableAsciiInternal (stream, dev);
    }
}

void
CapillaryTracer::EnableSourceAscii (std::string fileName, EnergySourceContainer n)
{
  for (EnergySourceContainer::Iterator i = n.Begin (); i != n.End (); ++i)
    {
      Ptr<EnergySource> dev = DynamicCast<EnergySource> (*i);
      NS_ASSERT (dev);
      std::ostringstream outputSuffix;
      outputSuffix << fileName << "." << dev->GetNode ()->GetId () << ".tr";
      AsciiTraceHelper ascii;
      Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (outputSuffix.str ());
      EnableAsciiInternal (stream, dev);
    }
}

void
CapillaryTracer::EnableDCRAscii (Ptr<OutputStreamWrapper> stream, NetDeviceContainer n)
{
  for (NetDeviceContainer::Iterator i = n.Begin (); i != n.End (); ++i)
    {
      Ptr<CapillaryNetDevice> dev = DynamicCast<CapillaryNetDevice> (*i);
      NS_ASSERT (dev);
      EnableAsciiInternal (stream, dev);
    }
}

void
CapillaryTracer::EnableEnergyAscii (Ptr<OutputStreamWrapper> stream, DeviceEnergyModelContainer n)
{
  for (DeviceEnergyModelContainer::Iterator i = n.Begin (); i != n.End (); ++i)
    {
      Ptr<CapillaryEnergyModel> dev = DynamicCast<CapillaryEnergyModel> (*i);
      NS_ASSERT (dev);
      EnableAsciiInternal (stream, dev);
    }
}

void
CapillaryTracer::EnableSourceAscii (Ptr<OutputStreamWrapper> stream, EnergySourceContainer n)
{
  for (EnergySourceContainer::Iterator i = n.Begin (); i != n.End (); ++i)
    {
      Ptr<EnergySource> dev = DynamicCast<EnergySource> (*i);
      NS_ASSERT (dev);
      EnableAsciiInternal (stream, dev);
    }
}

void
CapillaryTracer::DefaultDataCollectionRoundSinkWithContext (Ptr<OutputStreamWrapper> stream, std::string context, CapillaryMac::DcrStatus previous, CapillaryMac::DcrStatus current)
{
  switch (current)
    {

    case CapillaryMac::ACTIVE_START:
      *stream->GetStream () << "+ " << Simulator::Now ().GetSeconds () << " " << context << " [ACTIVE_START]" << std::endl;
      break;

    case CapillaryMac::ACTIVE_STOP:
      *stream->GetStream () << "- " << Simulator::Now ().GetSeconds () << " " << context << " [ACTIVE_STOP]" << std::endl;
      break;

    case CapillaryMac::ACTIVE_ABORT:
      *stream->GetStream () << "- " << Simulator::Now ().GetSeconds () << " " << context << " [ACTIVE_ABORT]" << std::endl;
      break;

    case CapillaryMac::NON_ACTIVE_START:
      *stream->GetStream () << "+ " << Simulator::Now ().GetSeconds () << " " << context << " [NON_ACTIVE_START]" << std::endl;
      break;

    case CapillaryMac::NON_ACTIVE_STOP:
      *stream->GetStream () << "- " << Simulator::Now ().GetSeconds () << " " << context << " [NON_ACTIVE_STOP]" << std::endl;
      break;

    default:
      *stream->GetStream () << "* " << Simulator::Now ().GetSeconds () << " " << context << " [UNKNOWN]" << std::endl;
      break;

    }

}

void
CapillaryTracer::DefaultFramesSinkWithContext (Ptr<OutputStreamWrapper> stream, std::string context, int previous, int current)
{
  *stream->GetStream () << "- " << Simulator::Now ().GetSeconds () <<
    " " << context << " " << current << std::endl;
}

void
CapillaryTracer::DefaultEnergyConsumptionSinkWithContext (Ptr<OutputStreamWrapper> stream, std::string context, double previous, double current)
{
  *stream->GetStream () << "- " << Simulator::Now ().GetSeconds () << " " << context << " " << current << std::endl;
}

void CapillaryTracer::DefaultEnergySourceSinkWithContext (Ptr<OutputStreamWrapper> stream, std::string context, double previous, double current)
{
  *stream->GetStream () << "* " << Simulator::Now ().GetSeconds () << " " << context << " " << current << std::endl;
}


} /* namespace ns3 */
