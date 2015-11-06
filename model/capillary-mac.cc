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

#include "capillary-mac.h"

#include <ns3/log.h>
#include <ns3/log-macros-disabled.h>
#include <ns3/object-base.h>
#include <ns3/ptr.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/type-id.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CapillaryMac");

NS_OBJECT_ENSURE_REGISTERED (CapillaryMac);

CapillaryMac::CapillaryMac (void)
{
  NS_LOG_FUNCTION (this);
}

CapillaryMac::~CapillaryMac (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId CapillaryMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CapillaryMac")
    .SetParent<Object> ()
    .SetGroupName ("m2m-capillary")
    .AddTraceSource ("Sniffer",
                     "Trace source simulating a non-promiscuous "
                     "packet sniffer attached to the device",
                     MakeTraceSourceAccessor (&CapillaryMac::m_snifferTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PromiscSniffer",
                     "Trace source simulating a promiscuous "
                     "packet sniffer attached to the device",
                     MakeTraceSourceAccessor (&CapillaryMac::m_promiscSnifferTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacTx",
                     "Trace source indicating a packet has "
                     "arrived for transmission by this device",
                     MakeTraceSourceAccessor (&CapillaryMac::m_macTxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacRx",
                     "A packet has been received by this device, "
                     "has been passed up from the physical layer "
                     "This is a non-promiscuous trace,",
                     MakeTraceSourceAccessor (&CapillaryMac::m_macRxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacRxDrop",
                     "Trace source indicating a packet was received, "
                     "but dropped before being forwarded up the stack",
                     MakeTraceSourceAccessor (&CapillaryMac::m_macRxDropTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacTxEnqueue",
                     "Trace source indicating a packet has been "
                     "enqueued in the transaction queue",
                     MakeTraceSourceAccessor (&CapillaryMac::m_macTxEnqueueTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacTxDequeue",
                     "Trace source indicating a packet has was "
                     "dequeued from the transaction queue",
                     MakeTraceSourceAccessor (&CapillaryMac::m_macTxDequeueTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("DcrStatus",
                     "Data collection round status: [START, STOP, ABORT]",
                     MakeTraceSourceAccessor (&CapillaryMac::m_activeDCR),
                     "ns3::TracedValue::EnumCallback")
    .AddAttribute ("StartDCRCallback",
                   "A Callback to call when a DCR is started.",
                   CallbackValue (),
                   MakeCallbackAccessor (&CapillaryMac::startDCRCallback),
                   MakeCallbackChecker ())
    .AddAttribute ("StopDCRCallback",
                   "A Callback to call when a DCR is stopped.",
                   CallbackValue (),
                   MakeCallbackAccessor (&CapillaryMac::stopDCRCallback),
                   MakeCallbackChecker ())
    .AddAttribute ("AbortDCRCallback",
                   "A Callback to call when a DCR is aborted.",
                   CallbackValue (),
                   MakeCallbackAccessor (&CapillaryMac::abortDCRCallback),
                   MakeCallbackChecker ())
  ;
  return tid;
}

void CapillaryMac::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  Object::Initialize ();
}

void CapillaryMac::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Object::Dispose ();
}

} /* namespace ns3 */
