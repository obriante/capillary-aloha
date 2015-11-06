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

#include "sensor-application.h"

#include <ns3/address.h>
#include <ns3/assert.h>
#include <ns3/int64x64-128.h>
#include <ns3/log.h>
#include <ns3/log-macros-disabled.h>
#include <ns3/object-base.h>
#include <ns3/pointer.h>
#include <ns3/simulator.h>
#include <ns3/string.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/type-id.h>
#include <ns3/uinteger.h>
#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SensorApplication");

NS_OBJECT_ENSURE_REGISTERED (SensorApplication);

SensorApplication::SensorApplication ()
{
  NS_LOG_FUNCTION (this);
}

SensorApplication::~SensorApplication ()
{
  NS_LOG_FUNCTION (this);
}

TypeId SensorApplication::GetTypeId (void)
{
  static TypeId tid =
    TypeId ("ns3::SensorApplication").SetParent<Application> ()
    .SetGroupName ("m2m-capillary")
    .AddConstructor<SensorApplication> ()
    .AddAttribute ("PacketSize",
                   "The size of packets sent in on state",
                   UintegerValue (140),
                   MakeUintegerAccessor (&SensorApplication::m_pktSize),
                   MakeUintegerChecker<uint32_t> (0))
    .AddAttribute ("RandomStream",
                   "A RandomVariableStream used to wake-up and transmit.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&SensorApplication::m_random),
                   MakePointerChecker<RandomVariableStream> ())
    .AddTraceSource ("EnqueuedData",
                     "A new packet is created and is enqueued",
                     MakeTraceSourceAccessor (&SensorApplication::m_dataTrace),
                     "Packet::TracedCallback")
  ;

  return tid;
}

int64_t SensorApplication::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_random->SetStream (stream);
  return 2;
}

void SensorApplication::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_random = 0;
  m_dev = 0;
  m_enqueEvent.Cancel ();

  // chain up
  Application::DoDispose ();
}

void SensorApplication::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoInitialize ();
}

void SensorApplication::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  // Insure no pending event
  CancelEvents ();
  Enqueue ();
}

void SensorApplication::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
  CancelEvents ();
}

void SensorApplication::CancelEvents (void)
{
  NS_LOG_FUNCTION (this);
  Simulator::Cancel (m_enqueEvent);
}

void SensorApplication::Enqueue (void)
{

  NS_LOG_FUNCTION (this);
  m_lastStartTime = Simulator::Now ();

  uint8_t buffer[m_pktSize];

  Ptr<Packet> packet = Create<Packet> (buffer, m_pktSize);
  m_dataTrace (packet);

  if (m_dev)
    {
      if (m_dev->Send (packet, m_dev->GetMac ()->GetCoordinator (),NetDevice::PACKET_HOST))
        {
          NS_LOG_DEBUG ("Node (" << this->GetNode ()->GetId () << "): Packet Forwarded Down");
        }
      else
        {
          NS_LOG_DEBUG ("Node (" << this->GetNode ()->GetId () << "): Packet Dropped");
        }
    }
  else
    {
      NS_LOG_DEBUG ("CapillaryNetDevice is NULL");
    }

  ScheduleEvent ();
}

void SensorApplication::ScheduleEvent (void)
{
  NS_LOG_FUNCTION (this);

  Time offInterval = Seconds (m_random->GetValue ());
  NS_LOG_DEBUG ("start at " << Seconds (Time (Simulator::Now () + offInterval)));
  m_enqueEvent = Simulator::Schedule (offInterval, &SensorApplication::Enqueue, this);
}

Ptr<CapillaryNetDevice> SensorApplication::GetDev () const
{
  NS_LOG_FUNCTION (this);
  return m_dev;
}

void SensorApplication::SetDev (Ptr<CapillaryNetDevice> dev)
{
  NS_LOG_FUNCTION (this << dev);
  NS_ASSERT (dev);
  m_dev = dev;
}

} /* namespace ns3 */
