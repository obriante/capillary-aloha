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

#include "capillary-phy.h"

#include <ns3/callback.h>
#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <ns3/packet.h>
#include <ns3/log-macros-disabled.h>
#include <ns3/type-id.h>

namespace ns3 {


NS_LOG_COMPONENT_DEFINE ("CapillaryPhy");

NS_OBJECT_ENSURE_REGISTERED (CapillaryPhy);

CapillaryPhy::CapillaryPhy (void)
{
  NS_LOG_FUNCTION (this);
}

CapillaryPhy::~CapillaryPhy (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId
CapillaryPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CapillaryPhy")
    .SetParent<SpectrumPhy> ()
    .SetGroupName ("m2m-capillary")
    .AddAttribute ("TxEndCallback",
                   "A Callbackto call when TX starts.",
                   CallbackValue (),
                   MakeCallbackAccessor (&CapillaryPhy::m_phyTxEndCallback),
                   MakeCallbackChecker ())
    .AddAttribute ("RxStartCallback",
                   "A Callback to call when TX ends.",
                   CallbackValue (),
                   MakeCallbackAccessor (&CapillaryPhy::m_phyRxStartCallback),
                   MakeCallbackChecker ())
    .AddAttribute ("RxEndErrorCallback",
                   "A Callback to call when RX ends with error.",
                   CallbackValue (),
                   MakeCallbackAccessor (&CapillaryPhy::m_phyRxEndErrorCallback),
                   MakeCallbackChecker ())
    .AddAttribute ("RxEndOkCallback",
                   "A Callback to call when RX ends successfully.",
                   CallbackValue (),
                   MakeCallbackAccessor (&CapillaryPhy::m_phyRxEndOkCallback),
                   MakeCallbackChecker ())
    .AddAttribute ("EnergyStateCallback",
                   "A Callback to update energy consumption",
                   CallbackValue (),
                   MakeCallbackAccessor (&CapillaryPhy::m_energyCallback),
                   MakeCallbackChecker ())
    .AddTraceSource ("TxStart",
                     "Trace fired when a transmission starts",
                     MakeTraceSourceAccessor (&CapillaryPhy::m_phyTxStartTrace),
                     "Packet::TraceCallback")
    .AddTraceSource ("TxEnd",
                     "Trace fired when a previosuly started transmission is finished",
                     MakeTraceSourceAccessor (&CapillaryPhy::m_phyTxEndTrace),
                     "Packet::TraceCallback")
    .AddTraceSource ("RxStart",
                     "Trace fired when the start of a signal is detected",
                     MakeTraceSourceAccessor (&CapillaryPhy::m_phyRxStartTrace),
                     "Packet::TraceCallback")
    .AddTraceSource ("RxAbort",
                     "Trace fired when a previously started RX is aborted before time",
                     MakeTraceSourceAccessor (&CapillaryPhy::m_phyRxAbortTrace),
                     "Packet::TraceCallback")
    .AddTraceSource ("RxEndOk",
                     "Trace fired when a previosuly started RX terminates successfully",
                     MakeTraceSourceAccessor (&CapillaryPhy::m_phyRxEndOkTrace),
                     "Packet::TraceCallback")
    .AddTraceSource ("RxEndError",
                     "Trace fired when a previosuly started RX terminates with an error (packet is corrupted)",
                     MakeTraceSourceAccessor (&CapillaryPhy::m_phyRxEndErrorTrace),
                     "Packet::TraceCallback")
  ;
  return tid;
}

void
CapillaryPhy::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_phyTxEndCallback.Nullify ();
  m_phyRxStartCallback.Nullify ();
  m_phyRxEndErrorCallback.Nullify ();
  m_phyRxEndOkCallback.Nullify ();

  SpectrumPhy::Dispose ();
}

void
CapillaryPhy::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  SpectrumPhy::Initialize ();
}

void
CapillaryPhy::SetEnergyModelCallback (DeviceEnergyModel::ChangeStateCallback cb)
{
  NS_LOG_FUNCTION (this);
  m_energyCallback = cb;
}

std::ostream& operator<< (std::ostream& os, CapillaryPhy::State state)
{
  switch (state)
    {
    case CapillaryPhy::IDLE:
      return (os << "IDLE");
    case CapillaryPhy::CCA_BUSY:
      return (os << "CCA_BUSY");
    case CapillaryPhy::TX:
      return (os << "TX");
    case CapillaryPhy::RX:
      return (os << "RX");
    case CapillaryPhy::SLEEP:
      return (os << "SLEEP");
    case CapillaryPhy::SWITCHING:
      return (os << "SWITCHING");
    default:
      NS_FATAL_ERROR ("PHY: Invalid FSALOHA PHY state");
      return (os << "INVALID");
    }
}

} /* namespace ns3 */
