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

#include <ns3/assert.h>
#include <ns3/callback.h>
#include <ns3/double.h>
#include <ns3/half-duplex-ideal-phy-signal-parameters.h>
#include <ns3/log.h>
#include <ns3/log-macros-disabled.h>
#include <ns3/mac64-address.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/object-base.h>
#include <ns3/simulator.h>
#include <ns3/spectrum-error-model.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/type-id.h>
#include <cmath>
#include <iostream>
#include "capillary-phy-ideal.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CapillaryPhyIdeal");

NS_OBJECT_ENSURE_REGISTERED (CapillaryPhyIdeal);

CapillaryPhyIdeal::CapillaryPhyIdeal (void)
  : m_mobility (0),
  m_netDevice (0),
  m_channel (0),
  m_txPsd (0),
  m_state (IDLE)
{
  NS_LOG_FUNCTION (this);
  m_interference.SetErrorModel (CreateObject<ShannonSpectrumErrorModel> ());
}

CapillaryPhyIdeal::~CapillaryPhyIdeal (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId CapillaryPhyIdeal::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CapillaryPhyIdeal")
    .SetParent<CapillaryPhy> ()
    .AddConstructor<CapillaryPhyIdeal> ()
    .SetGroupName ("m2m-capillary")
    .AddAttribute ("Rate", "The PHY rate used by this device", DataRateValue (DataRate ("250Kbps")),
                   MakeDataRateAccessor (&CapillaryPhyIdeal::SetRate,
                                         &CapillaryPhyIdeal::GetRate),
                   MakeDataRateChecker ())
    .AddAttribute ("TransitionTime",
                   "The Transition time between IDLE -> SLEEP and vice versa", TimeValue (MicroSeconds (192)),
                   MakeTimeAccessor (&CapillaryPhyIdeal::m_switch),
                   MakeTimeChecker ())
  ;
  return tid;
}

void CapillaryPhyIdeal::SetDevice (Ptr<NetDevice> d)
{
  NS_LOG_FUNCTION (this << d);
  m_netDevice = d;
}

Ptr<NetDevice> CapillaryPhyIdeal::GetDevice (void) const
{
  NS_LOG_FUNCTION (this);
  return m_netDevice;
}

void CapillaryPhyIdeal::SetMobility (Ptr<MobilityModel> m)
{
  NS_LOG_FUNCTION (this << m);
  m_mobility = m;
}

Ptr<MobilityModel> CapillaryPhyIdeal::GetMobility (void)
{
  NS_LOG_FUNCTION (this);
  return m_mobility;
}

void CapillaryPhyIdeal::SetChannel (Ptr<SpectrumChannel> c)
{
  NS_LOG_FUNCTION (this << c);
  m_channel = c;
}

Ptr<const SpectrumModel> CapillaryPhyIdeal::GetRxSpectrumModel (void) const
{
  NS_LOG_FUNCTION (this);

  if (m_txPsd)
    {
      return m_txPsd->GetSpectrumModel ();
    }
  else
    {
      return 0;
    }
}

Ptr<AntennaModel> CapillaryPhyIdeal::GetRxAntenna (void)
{
  NS_LOG_FUNCTION (this);
  return m_antenna;
}

void CapillaryPhyIdeal::StartRx (Ptr<SpectrumSignalParameters> spectrumParams)
{
  NS_LOG_FUNCTION (this << spectrumParams);
  NS_LOG_LOGIC (this << " state: " << m_state);
  NS_LOG_LOGIC (this << " rx power: " << 10 * std::log10 (Integral (*(spectrumParams->psd))) + 30 << " dBm");

  // interference will happen regardless of the state of the receiver
  m_interference.AddSignal (spectrumParams->psd, spectrumParams->duration);

  // the device might start RX only if the signal is of a type understood by this device
  // this corresponds in real devices to preamble detection
  Ptr<HalfDuplexIdealPhySignalParameters> rxParams = DynamicCast<HalfDuplexIdealPhySignalParameters> (spectrumParams);
  if (rxParams != 0)
    {
      // signal is of known type
      switch (m_state)
        {

        case CapillaryPhy::IDLE:
          {
            // preamble detection and synchronization is supposed to be always successful.
            Ptr<Packet> p = rxParams->data;
            m_phyRxStartTrace (p);
            m_rxPacket = p;
            m_rxPsd = rxParams->psd;
            ChangeState (CapillaryPhy::RX);
            if (!m_phyRxStartCallback.IsNull ())
              {
                NS_LOG_LOGIC (this << " calling m_phyMacRxStartCallback");
                m_phyRxStartCallback ();
              }
            else
              {
                NS_LOG_LOGIC (this << " m_phyMacRxStartCallback is NULL");
              }
            m_interference.StartRx (p, rxParams->psd);
            NS_LOG_LOGIC (this << " scheduling EndRx with delay " << rxParams->duration);
            m_endRxEventId = Simulator::Schedule (rxParams->duration, &CapillaryPhyIdeal::EndRx, this);
          }
          break;
        case CapillaryPhy::RX:
          m_endRxEventId.Cancel ();
          m_endRxEventId = Simulator::Schedule (rxParams->duration, &CapillaryPhyIdeal::AbortRx, this);

/*
         we should check if we should re-sync on a new incoming signal and discard the old one
         (somebody calls this the "capture" effect)
         criteria considered to do might include the following:
          1) signal strength (e.g., as returned by rxPsd.Norm ())
          2) how much time has passed since previous RX attempt started
         if re-sync (capture) is done, then we should call AbortRx ()
*/
          break;

        case CapillaryPhy::CCA_BUSY:
        case CapillaryPhy::TX:
        // the PHY will not notice this incoming signal
        case CapillaryPhy::SLEEP:
        case CapillaryPhy::SWITCHING:
          break;
        }
    }
  else           // rxParams == 0
    {
      NS_LOG_LOGIC (this << " signal of unknown type");
    }

  NS_LOG_LOGIC (this << " state: " << m_state);
}

void CapillaryPhyIdeal::ForceSleep (void)
{
  NS_LOG_FUNCTION (this);
  m_endRxEventId.Cancel ();
  ChangeState (CapillaryPhy::SLEEP);
}

void CapillaryPhyIdeal::WakeUp ()
{
  NS_LOG_FUNCTION (this);
  m_endRxEventId.Cancel ();
  ChangeState (CapillaryPhy::IDLE);
}

void CapillaryPhyIdeal::SetTxPowerSpectralDensity (Ptr<SpectrumValue> txPsd)
{
  NS_LOG_FUNCTION (this << txPsd);
  NS_ASSERT (txPsd);
  m_txPsd = txPsd;
  NS_LOG_INFO ( *txPsd << *m_txPsd);
}

void CapillaryPhyIdeal::SetNoisePowerSpectralDensity (
  Ptr<const SpectrumValue> noisePsd)
{
  NS_LOG_FUNCTION (this << noisePsd);
  NS_ASSERT (noisePsd);
  m_interference.SetNoisePowerSpectralDensity (noisePsd);
}

Ptr<SpectrumSignalParameters> CapillaryPhyIdeal::TransmissionSignalParameters ()
{
  NS_LOG_FUNCTION (this);
  Ptr<HalfDuplexIdealPhySignalParameters> txParams = Create<HalfDuplexIdealPhySignalParameters> ();
  Time txTimeSeconds = m_rate.CalculateBytesTxTime (m_txPacket->GetSize ());
  txParams->duration = txTimeSeconds;
  txParams->txPhy = GetObject<SpectrumPhy> ();
  txParams->txAntenna = m_antenna;
  txParams->psd = m_txPsd;
  txParams->data = m_txPacket;

  return txParams;
}

bool CapillaryPhyIdeal::StartTx (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  NS_LOG_LOGIC (this << "state: " << m_state);

  m_phyTxStartTrace (p);

  switch (m_state)
    {
    case CapillaryPhy::RX:
      AbortRx ();
    // fall through

    case CapillaryPhy::IDLE:
      {
        m_txPacket = p;
        ChangeState (CapillaryPhy::TX);

        Ptr<SpectrumSignalParameters> txParams = TransmissionSignalParameters ();
        NS_LOG_LOGIC (this << " tx power: " << 10 * std::log10 (Integral (*(txParams->psd))) + 30 << " dBm");
        m_channel->StartTx (txParams);
        Simulator::Schedule (txParams->duration, &CapillaryPhyIdeal::EndTx, this);
        break;
      }
    case CapillaryPhy::TX:
    case CapillaryPhy::CCA_BUSY:
    case CapillaryPhy::SLEEP:
    case CapillaryPhy::SWITCHING:
      return true;
      break;
    }

  return false;
}

void CapillaryPhyIdeal::SetRate (DataRate rate)
{
  NS_LOG_FUNCTION (this << rate);
  m_rate = rate;
}

DataRate CapillaryPhyIdeal::GetRate (void) const
{
  NS_LOG_FUNCTION (this);
  return m_rate;
}

Time CapillaryPhyIdeal::GetSwitchingTime (void) const
{
  NS_LOG_FUNCTION (this);
  return m_switch;
}

void CapillaryPhyIdeal::SetAntenna (Ptr<AntennaModel> a)
{
  NS_LOG_FUNCTION (this << a);
  m_antenna = a;
}

void CapillaryPhyIdeal::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_mobility = 0;
  m_netDevice = 0;
  m_channel = 0;
  m_txPsd = 0;
  m_rxPsd = 0;
  m_txPacket = 0;
  m_rxPacket = 0;
}

void CapillaryPhyIdeal::ChangeState (CapillaryPhy::State newState)
{
  NS_LOG_FUNCTION (this);


  if ((m_state != SWITCHING) && (m_state == SLEEP  || newState == SLEEP))
    {

      NS_LOG_DEBUG ("" << Mac64Address::ConvertFrom (GetDevice ()->GetAddress ()) << " state: " << m_state << " -> " << SWITCHING);

      m_state = SWITCHING;
      Simulator::Schedule (m_switch, &CapillaryPhyIdeal::ChangeState, this, newState);
    }
  else
    {
      NS_LOG_DEBUG ("" << Mac64Address::ConvertFrom (GetDevice ()->GetAddress ()) << " state: " << m_state << " -> " << newState);
      m_state = newState;
    }

  if (!m_energyCallback.IsNull ())
    {
      m_energyCallback (m_state);
    }


}

void CapillaryPhyIdeal::EndTx (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " state: " << m_state);

  if (m_state == CapillaryPhy::TX)
    {

      m_phyTxEndTrace (m_txPacket);

      if (!m_phyTxEndCallback.IsNull ())
        {
          m_phyTxEndCallback (m_txPacket);
        }

      m_txPacket = 0;
      ChangeState (CapillaryPhy::IDLE);
    }
}

void CapillaryPhyIdeal::AbortRx (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << "state: " << m_state);

  if (m_state == CapillaryPhy::RX)
    {
      m_interference.AbortRx ();
      m_phyRxAbortTrace (m_rxPacket);
      m_endRxEventId.Cancel ();
      m_rxPacket = 0;
      ChangeState (CapillaryPhy::IDLE);

      m_phyRxEndErrorTrace (m_rxPacket);
      if (!m_phyRxEndErrorCallback.IsNull ())
        {
          NS_LOG_LOGIC (this << " calling m_phyMacRxEndErrorCallback");
          m_phyRxEndErrorCallback ();
        }
      else
        {
          NS_LOG_LOGIC (this << " m_phyMacRxEndErrorCallback is NULL");
        }
    }
}

void CapillaryPhyIdeal::EndRx (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " state: " << m_state);

  if (m_state == CapillaryPhy::RX)
    {
      bool rxOk = m_interference.EndRx ();

      if (rxOk)
        {
          m_phyRxEndOkTrace (m_rxPacket);
          if (!m_phyRxEndOkCallback.IsNull ())
            {
              NS_LOG_LOGIC (this << " calling m_phyMacRxEndOkCallback");
              m_phyRxEndOkCallback (m_rxPacket);
            }
          else
            {
              NS_LOG_LOGIC (this << " m_phyMacRxEndOkCallback is NULL");
            }
        }
      else
        {
          m_phyRxEndErrorTrace (m_rxPacket);
          if (!m_phyRxEndErrorCallback.IsNull ())
            {
              NS_LOG_LOGIC (this << " calling m_phyMacRxEndErrorCallback");
              m_phyRxEndErrorCallback ();
            }
          else
            {
              NS_LOG_LOGIC (this << " m_phyMacRxEndErrorCallback is NULL");
            }
        }

      ChangeState (CapillaryPhy::IDLE);
      m_rxPacket = 0;
      m_rxPsd = 0;
    }
}

CapillaryPhy::State CapillaryPhyIdeal::GetStatus (void) const
{
  NS_LOG_FUNCTION (this);
  return m_state;
}

} /* namespace ns3 */
