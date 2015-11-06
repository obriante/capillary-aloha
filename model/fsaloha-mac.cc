/* -*- Mode:C++{} c-file-style:"gnu"{} indent-tabs-mode:nil{} -*- */
/*
 * Copyright (c) 2015 Universita' Mediterranea di Reggio Calabria (UNIRC)
 *
 * This program is free software{} you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation{}
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Orazio Briante <orazio.briante@unirc.it>
 */

#include "fsaloha-mac.h"

#include <ns3/assert.h>
#include <ns3/callback.h>
#include <ns3/data-rate.h>
#include <ns3/double.h>
#include <ns3/enum.h>
#include <ns3/event-id.h>
#include <ns3/llc-snap-header.h>
#include <ns3/log.h>
#include <ns3/log-macros-disabled.h>
#include <ns3/object-base.h>
#include <ns3/packet.h>
#include <ns3/pointer.h>
#include <ns3/simulator.h>
#include <ns3/string.h>
#include <ns3/traced-value.h>
#include <ns3/type-id.h>
#include <ns3/uinteger.h>
#include <stddef.h>
#include <algorithm>    // std::find
#include <cstring>
#include <iterator>

#include <ns3/capillary-mac-header.h>
#include <ns3/capillary-mac-trailer.h>
#include <ns3/capillary-phy.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FsalohaMac");

NS_OBJECT_ENSURE_REGISTERED (FsalohaMac);

#define MAC_DEBUG(x) NS_LOG_DEBUG ("" << Mac64Address::ConvertFrom (GetAddress ()) << " " << x)

FsalohaMac::FsalohaMac () :
  m_dev (0)
{
  NS_LOG_FUNCTION (this);
  m_activeDCR = CapillaryMac::ACTIVE_STOP;
  m_nFramesDCR = 0;
}

FsalohaMac::~FsalohaMac ()
{
  NS_LOG_FUNCTION (this);
}

void FsalohaMac::SetDevice (Ptr<NetDevice> d)
{
  NS_LOG_FUNCTION (this << d);
  m_dev = DynamicCast<CapillaryNetDevice> (d);
  NS_ASSERT (m_dev);
}

Ptr<NetDevice> FsalohaMac::GetDevice ()
{
  NS_LOG_FUNCTION (this);
  return m_dev;
}

TypeId FsalohaMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FsalohaMac")
    .SetParent<CapillaryMac> ()
    .SetGroupName ("m2m-capillary")
    .AddConstructor<FsalohaMac> ()
    .AddAttribute ("Mtu",
                   "Maximum Transmission Unit", UintegerValue (140),
                   MakeUintegerAccessor (&FsalohaMac::SetMtu,
                                         &FsalohaMac::GetMtu),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("ForwardUpCallback",
                   "A Callback to Forward up a received data packet.",
                   CallbackValue (),
                   MakeCallbackAccessor (&FsalohaMac::m_fwdUp),
                   MakeCallbackChecker ())
    .AddAttribute ("slots",
                   "The number of slots in a Frame", UintegerValue (1),
                   MakeUintegerAccessor (&FsalohaMac::SetNSlots, &FsalohaMac::GetNSlots),
                   MakeUintegerChecker<uint16_t> (1, 32768))
    .AddAttribute ("MaxDelay",
                   "The maximum accettable delay", TimeValue (MicroSeconds (10)),
                   MakeTimeAccessor (&FsalohaMac::m_maxDelay),
                   MakeTimeChecker ())
    .AddAttribute ("packets",
                   "The number of packets to transmit in a DCR", UintegerValue (1),
                   MakeUintegerAccessor (&FsalohaMac::m_NPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("RandomStream",
                   "A Random Variable Stream used to select transmission slots.",
                   PointerValue (),
                   MakePointerAccessor (&FsalohaMac::SetRandomStream, &FsalohaMac::GetRandomStream),
                   MakePointerChecker<UniformRandomVariable> ())
    .AddAttribute ("Queue",
                   "packets being transmitted get queued here",
                   StringValue ("ns3::DropTailQueue"),
                   MakePointerAccessor (&FsalohaMac::m_queue),
                   MakePointerChecker<Queue> ())
    .AddAttribute ("TxQueue",
                   "packets being transmitted get queued here",
                   StringValue ("ns3::DropTailQueue"),
                   MakePointerAccessor (&FsalohaMac::m_TxQueue),
                   MakePointerChecker<Queue> ())
    .AddAttribute ("Controller",
                   "The Capillary Controller .",
                   StringValue ("ns3::BasicController"),
                   MakePointerAccessor (&FsalohaMac::SetController, &FsalohaMac::GetController),
                   MakePointerChecker<CapillaryController> ())
    .AddTraceSource ("Frames",
                     "The number of frames in a DCR",
                     MakeTraceSourceAccessor (&FsalohaMac::m_nFrames))
  ;

  return tid;
}

/** Inherited Methods*/
void FsalohaMac::SetPhy (Ptr<CapillaryPhy> phy)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (phy);
  m_phy = phy;

  m_phy->SetAttribute ("TxEndCallback", CallbackValue (MakeCallback (&FsalohaMac::NotifyTransmissionEnd, this)));
  m_phy->SetAttribute ("RxStartCallback", CallbackValue (MakeCallback (&FsalohaMac::NotifyReceptionStart, this)));
  m_phy->SetAttribute ("RxEndErrorCallback", CallbackValue (MakeCallback (&FsalohaMac::NotifyReceptionEndError, this)));
  m_phy->SetAttribute ("RxEndOkCallback", CallbackValue (MakeCallback (&FsalohaMac::NotifyReceptionEndOk, this)));
}

Ptr<CapillaryPhy> FsalohaMac::GetPhy (void)
{
  NS_LOG_FUNCTION (this);

  return m_phy;
}

Address FsalohaMac::GetAddress (void)
{
  NS_LOG_FUNCTION (this);
  return m_addr;
}

void FsalohaMac::SetAddress (Address addr)
{
  NS_LOG_FUNCTION (this);
  m_addr = Mac64Address::ConvertFrom (addr);
}

Address FsalohaMac::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return Mac64Address ("ff:ff:ff:ff:ff:ff:ff:ff");
}

Address FsalohaMac::GetCoordinator (void) const
{
  NS_LOG_FUNCTION (this);
  return Mac64Address ("00:00:00:00:00:00:00:00");
}

bool FsalohaMac::SetMtu (const uint16_t mtu)
{
  NS_LOG_FUNCTION (this);
  m_mtu = mtu;

  return true;
}

uint16_t FsalohaMac::GetMtu (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mtu;
}

Time FsalohaMac::GetSlotDuration (void) const
{
  NS_LOG_FUNCTION (this);

  CapillaryMacHeader header;
  LlcSnapHeader llc;
  CapillaryMacTrailer trailer;

  return (2 * m_maxDelay) + Time (Seconds ((m_mtu + header.GetSerializedSize () +  trailer.GetSerializedSize () + llc.GetSerializedSize ()) * 8.0 / m_phy->GetRate ().GetBitRate ()));
}

void FsalohaMac::SetNSlots (const uint16_t nSlots)
{
  NS_LOG_FUNCTION (this << nSlots);
  NS_ASSERT (nSlots > 0);
  m_nSlots = nSlots;

  m_slotStatus.resize (m_nSlots);

  if (m_random)
    {
      m_random->SetAttribute ("Max", DoubleValue (m_nSlots - 1));
    }
}

uint16_t FsalohaMac::GetNSlots (void) const
{
  NS_LOG_FUNCTION (this);
  return m_nSlots;
}


void FsalohaMac::SetRandomStream (Ptr<UniformRandomVariable> random)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (random);

  m_random = random;

  m_random->SetAttribute ("Min", DoubleValue (0));
  m_random->SetAttribute ("Max", DoubleValue (m_nSlots - 1));
}

Ptr<UniformRandomVariable> FsalohaMac::GetRandomStream (void) const
{
  NS_LOG_FUNCTION (this);
  return m_random;
}

void FsalohaMac::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_rndSlot = 0;
  m_currSlot = 0;
  m_slotStatus = std::vector<SlotState> ();
}

void FsalohaMac::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_phy = 0;
  m_random = 0;
  m_dev = 0;
  m_controller = 0;
  m_fwdUp.Nullify ();
}

void FsalohaMac::WakeUp (void)
{
  NS_LOG_FUNCTION (this);

  if (m_phy->GetStatus () == CapillaryPhy::SLEEP)
    {
      m_phy->WakeUp ();
    }

  switch (m_dev->GetType ())
    {
    case CapillaryNetDevice::COORDINATOR:
      Simulator::Schedule (m_phy->GetSwitchingTime (), &FsalohaMac::StartActivePeriod, this);
      break;

    case CapillaryNetDevice::END_DEVICE:
      break;
    }
}

void FsalohaMac::ForceSleep (void)
{
  NS_LOG_FUNCTION (this);

  MAC_DEBUG ("Forced State Sleep.");

  m_phy->ForceSleep ();
}

bool FsalohaMac::DataEnqueue (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (packet << source << dest << protocolNumber);

  switch (m_dev->GetType ())
    {
    case CapillaryNetDevice::COORDINATOR:
      NS_LOG_ERROR ("DATA packets transmission not supported by COORDINATOR devices.");
      break;
    case CapillaryNetDevice::END_DEVICE:
      {

        if ( packet->GetSize () <= m_mtu)
          {
            LlcSnapHeader llc;
            llc.SetType (protocolNumber);
            packet->AddHeader (llc);

            CapillaryMacHeader macHdr (CapillaryMacHeader::CAPILLARY_MAC_DATA);
            macHdr.SetSeqNum (m_DataSeqNum);
            m_DataSeqNum++;
            macHdr.SetSrcAddr (Mac64Address::ConvertFrom (source));
            macHdr.SetDstAddr (Mac64Address::ConvertFrom (dest));

            packet->AddHeader (macHdr);

            m_queue->Enqueue (packet);

            MAC_DEBUG ("Data Queue: " << m_queue->GetNPackets ());

            return true;
          }
        else
          {
            NS_LOG_ERROR ("Fragmentation not implemented yet. The Packet was dropped");
          }
      }
      break;
    }

  return false;
}

bool FsalohaMac::TrasmissionEnqueue (void)
{
  NS_LOG_FUNCTION (this);

  for (uint32_t i = 0; i < m_NPackets; i++)
    {

      if (!m_queue->IsEmpty ())
        {
          if (m_TxQueue->GetNPackets () >= m_NPackets)
            {
              break;
            }

          Ptr<Packet> p = m_queue->Dequeue ();
          if (p)
            {
              m_TxQueue->Enqueue (p);
              m_macTxEnqueueTrace (p);
            }
        }
      else
        {
          break;
        }

    }

  if (!m_TxQueue->IsEmpty ())
    {
      MAC_DEBUG ("Transmission Queue: " << m_TxQueue->GetNPackets ());

      m_currentPkt = m_TxQueue->Dequeue ();
      if (m_currentPkt)
        {
          return true;
        }
    }

  return false;
}


void FsalohaMac::NotifyTransmissionStart (Ptr<const Packet> p)
{
  NS_LOG_FUNCTION (this << *p);

  m_macTxTrace (p);
}


void FsalohaMac::NotifyTransmissionEnd (Ptr<const Packet> p)
{
  NS_LOG_FUNCTION (this << *p);

  m_promiscSnifferTrace (p);
  m_snifferTrace (p);

  CapillaryMacHeader macHdr;
  p->PeekHeader (macHdr);

  switch (m_dev->GetType ())
    {
    case CapillaryNetDevice::COORDINATOR:
      m_macTxDequeueTrace (p);
      switch (macHdr.GetFrameType ())
        {
        case CapillaryMacHeader::CAPILLARY_MAC_RFD:
          MAC_DEBUG ("RFD Successfully Sent");
          Simulator::Schedule (m_maxDelay, &FsalohaMac::StartFrame, this);
          break;
        case CapillaryMacHeader::CAPILLARY_MAC_DATA:
          MAC_DEBUG ("DATA Successfully Sent");
          break;
        case CapillaryMacHeader::CAPILLARY_MAC_FBP:
          {
            MAC_DEBUG ("FBP Successfully Sent");
            MAC_DEBUG ("Slots Status" << m_slotStatus);

            m_nFramesDCR++;

            if (m_NPackets == 1)
              {
                if (std::find (m_slotStatus.begin (), m_slotStatus.end (), ERROR) == m_slotStatus.end ())
                  {
                    NotifyActivePeriodStopped ();
                  }
                else
                  {
                    Simulator::Schedule (m_maxDelay, &FsalohaMac::StartFrame, this);
                  }

              }
            else
              {

                if ((std::find (m_slotStatus.begin (), m_slotStatus.end (), ERROR) == m_slotStatus.end ())
                    && (std::find (m_slotStatus.begin (), m_slotStatus.end (), OK) == m_slotStatus.end ())
                    )
                  {
                    NotifyActivePeriodStopped ();
                  }
                else
                  {
                    Simulator::Schedule (m_maxDelay, &FsalohaMac::StartFrame, this);
                  }
              }

          }
          break;
        }
      break;

    case CapillaryNetDevice::END_DEVICE:
      MAC_DEBUG ("Packet successfully sent.");
      break;
    }
}

void FsalohaMac::NotifyReceptionStart (void)
{
  NS_LOG_FUNCTION (this);
}


void FsalohaMac::NotifyReceptionEndError (void)
{
  NS_LOG_FUNCTION (this);

  MAC_DEBUG ("Reception Error.");

  switch (m_dev->GetType ())
    {
    case CapillaryNetDevice::COORDINATOR:
      {
        m_slotStatus[m_currSlot] = ERROR;
      }
      break;

    case CapillaryNetDevice::END_DEVICE:
      break;
    }
}


void FsalohaMac::NotifyReceptionEndOk (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  if (p)
    {
      CapillaryMacTrailer macTrailer;
      p->RemoveTrailer (macTrailer);

      if (macTrailer.CheckCRC (p))
        {
          m_macRxTrace (p);

          MAC_DEBUG ("Received packet: " << *p);

          CapillaryMacHeader header;
          p->RemoveHeader (header);
          NS_LOG_LOGIC ("packet " << header.GetSrcAddr () << " --> " << header.GetDstAddr () << " (here: " << m_addr << ")");

          LlcSnapHeader llc;
          p->RemoveHeader (llc);

          switch (m_dev->GetType ())
            {
            case CapillaryNetDevice::COORDINATOR:

              switch (header.GetFrameType ())
                {
                case CapillaryMacHeader::CAPILLARY_MAC_DATA:
                  {
                    m_slotStatus[m_currSlot] = OK;

                    if (!m_fwdUp.IsNull ())
                      {
                        m_fwdUp (p, llc, header.GetSrcAddr (), header.GetDstAddr ());
                      }
                  }
                  break;

                case CapillaryMacHeader::CAPILLARY_MAC_RFD:
                case CapillaryMacHeader::CAPILLARY_MAC_FBP:
                  break;

                }

              break;

            case CapillaryNetDevice::END_DEVICE:

              if (header.GetDstAddr () == Mac64Address::ConvertFrom (GetAddress ())
                  || header.GetDstAddr () == Mac64Address::ConvertFrom (GetBroadcast ()))
                {

                  switch (header.GetFrameType ())
                    {
                    case CapillaryMacHeader::CAPILLARY_MAC_DATA:
                      if (!m_fwdUp.IsNull ())
                        {
                          m_fwdUp (p, llc, header.GetSrcAddr (), header.GetDstAddr ());
                        }
                      break;

                    case CapillaryMacHeader::CAPILLARY_MAC_RFD:
                      if (m_activeDCR == CapillaryMac::ACTIVE_START)
                        {
                          MAC_DEBUG ("Aborting Previous DCR.");
                          NotifyActivePeriodAborted ();
                        }
                      else
                        {
                          StartActivePeriod ();
                        }
                      break;

                    case CapillaryMacHeader::CAPILLARY_MAC_FBP:
                      {
                        if (m_activeDCR == CapillaryMac::ACTIVE_START)
                          {
                            m_nFramesDCR++;

                            uint8_t payload[p->GetSize ()];
                            p->CopyData (payload, p->GetSize ());

                            DeserializeFBP (payload, p->GetSize ());

                            MAC_DEBUG ("Slots Status" << m_slotStatus);
                            MAC_DEBUG ("Current Slot: " << m_rndSlot);
                            MAC_DEBUG ("Current Slot Status: " << m_slotStatus[m_rndSlot]);

                            if (m_currentPkt)
                              {
                                switch (m_slotStatus[m_rndSlot])
                                  {
                                  case OK:
                                    MAC_DEBUG ("Transmission: [SUCCESS]");
                                    m_currentPkt = 0;
                                    if (!m_TxQueue->IsEmpty ())
                                      {
                                        m_currentPkt = m_TxQueue->Dequeue ();
                                        StartFrame ();
                                      }
                                    else
                                      {
                                        NotifyActivePeriodStopped ();
                                      }
                                    break;

                                  case EMPTY:
                                    MAC_DEBUG ("Transmission: [EMPTY]");
                                    NotifyActivePeriodAborted ();
                                    break;

                                  case ERROR:
                                    MAC_DEBUG ("Transmission: [ERROR]");

                                    CapillaryMacTrailer trailer;
                                    m_currentPkt->RemoveTrailer (trailer);

                                    CapillaryMacHeader header;
                                    m_currentPkt->RemoveHeader (header);

                                    header.SetRetry (true);
                                    m_currentPkt->AddHeader (header);
                                    StartFrame ();
                                    break;
                                  }
                              }
                          }
                      }
                      break;
                    }
                }
              break;
            }
        }
      else
        {
          m_macRxDropTrace (p);
        }
    }
}


void FsalohaMac::SerializeFBP (uint8_t *payload, uint32_t length)
{
  NS_LOG_FUNCTION (this);

  memset ( payload, 0x00, length );
  for (size_t i = 0; i < m_slotStatus.size (); i++)
    {
      uint16_t positionVector = (i / 4);
      uint8_t position = 0;

      switch (i - (positionVector * 4))
        {
        case 0x00:
          position = 6;
          break;
        case 0x01:
          position = 4;
          break;
        case 0x02:
          position = 2;
          break;
        case 0x03:
          position = 0;
          break;
        }

      payload[positionVector] |= (m_slotStatus[i] << position) & (0x03 << position);
    }
}
void FsalohaMac::DeserializeFBP (uint8_t *payload, uint32_t length)
{
  NS_LOG_FUNCTION (this);

  for (size_t i = 0; i < m_slotStatus.size (); i++)
    {
      uint16_t positionVector = (i / 4);
      uint8_t position = 0;

      switch (i - (positionVector * 4))
        {
        case 0x00:
          position = 6;
          break;
        case 0x01:
          position = 4;
          break;
        case 0x02:
          position = 2;
          break;
        case 0x03:
          position = 0;
          break;
        }

      m_slotStatus[i] = static_cast<SlotState> (( payload[positionVector] >> position) & 0x03);
    }
}

void FsalohaMac::StartActivePeriod (void)
{
  NS_LOG_FUNCTION (this);

  switch (m_dev->GetType ())
    {
    case CapillaryNetDevice::COORDINATOR:

      MAC_DEBUG ("Is starting a new DCR.");
      m_activeDCR = CapillaryMac::ACTIVE_START;
      m_controller->NotifyActivePeriodStart();

      if (!startDCRCallback.IsNull ())
        {
          startDCRCallback (m_addr);
        }
      m_nFramesDCR = 0;
      m_nFrames = m_nFramesDCR;
      FsalohaMac::SendRequestForData ();

      break;

    case CapillaryNetDevice::END_DEVICE:

      if (m_activeDCR == CapillaryMac::ACTIVE_START)
        {
          NotifyActivePeriodAborted ();
        }

      if (TrasmissionEnqueue ())
        {
          MAC_DEBUG ("Is starting a new DCR.");
          m_activeDCR = CapillaryMac::ACTIVE_START;

          if (!startDCRCallback.IsNull ())
            {
              startDCRCallback (m_addr);
            }
          m_nFramesDCR = 0;
          m_nFrames = m_nFramesDCR;
          StartFrame ();
        }
      else
        {
          ForceSleep ();
          Simulator::Schedule (m_nSlots * GetSlotDuration () - m_phy->GetSwitchingTime (), &FsalohaMac::WakeUp, this);
        }

      break;
    }
}

void FsalohaMac::NotifyActivePeriodStopped (void)
{
  NS_LOG_FUNCTION (this);
  MAC_DEBUG ("Active Period [STOP]");


  m_nFrames = m_nFramesDCR;

  if (m_activeDCR == CapillaryMac::ACTIVE_START)
    {
      m_activeDCR = CapillaryMac::ACTIVE_STOP;

      if (!stopDCRCallback.IsNull ())
        {
          stopDCRCallback (m_addr);
        }
    }

  StartNonActivePeriod ();
}

void FsalohaMac::NotifyActivePeriodAborted (void)
{
  NS_LOG_FUNCTION (this);
  MAC_DEBUG ("Active Period [ABORT]");

  m_nFrames = m_nFramesDCR;

  if (m_activeDCR == CapillaryMac::ACTIVE_START)
    {
      m_activeDCR = CapillaryMac::ACTIVE_ABORT;

      if (!abortDCRCallback.IsNull ())
        {
          abortDCRCallback (m_addr);
        }
    }

  StartNonActivePeriod ();
}

void FsalohaMac::StartNonActivePeriod (void)
{
  NS_LOG_FUNCTION (this);
  MAC_DEBUG ("Non Active Period [START]");

  m_activeDCR = CapillaryMac::NON_ACTIVE_START;

  Time off = m_controller->GetOffTime ();

  if (off > m_phy->GetSwitchingTime ())
    {
      ForceSleep ();
      Simulator::Schedule (off, &FsalohaMac::NotifyNonActivePeriodStopped, this);
    }
  else
    {
      NotifyNonActivePeriodStopped ();
    }
}

void FsalohaMac::NotifyNonActivePeriodStopped (void)
{
  NS_LOG_FUNCTION (this);
  MAC_DEBUG ("Non Active Period [STOP]");

  m_activeDCR = CapillaryMac::NON_ACTIVE_STOP;

  WakeUp ();
}

void
FsalohaMac::ResetFrame (void)
{
  NS_LOG_FUNCTION (this);

  m_currSlot = 0;

  if (m_dev->GetType () == CapillaryNetDevice::END_DEVICE)
    {
      m_rndSlot = m_random->GetInteger ();
      MAC_DEBUG ("Random Slot: " << m_rndSlot);
    }

  for (uint32_t i = 0; i < m_slotStatus.size (); i++)
    {
      m_slotStatus[i] = EMPTY;
    }
}

void FsalohaMac::StartFrame (void)
{
  NS_LOG_FUNCTION (this);

  ResetFrame ();

  if (m_activeDCR == CapillaryMac::ACTIVE_START)
    {
      m_startFrame = Simulator::Now ();

      switch (m_dev->GetType ())
        {
        case CapillaryNetDevice::COORDINATOR:
          break;
        case CapillaryNetDevice::END_DEVICE:
          if ((m_rndSlot * GetSlotDuration ()) > 2 * m_phy->GetSwitchingTime ())
            {
              m_phy->ForceSleep ();
              Simulator::Schedule ((m_rndSlot * GetSlotDuration ()) - m_phy->GetSwitchingTime (), &CapillaryPhy::WakeUp, m_phy);
            }
          break;
        }

      Simulator::Schedule (m_maxDelay, &FsalohaMac::StartSlot, this);

      //StartSlot();
    }
}

void
FsalohaMac::StartSlot (void)
{
  NS_LOG_FUNCTION (this);

  MAC_DEBUG ("Start Slot: " << m_currSlot << ", length: " << GetSlotDuration ().GetSeconds ());

  if (m_activeDCR == CapillaryMac::ACTIVE_START)
    {
      switch (m_dev->GetType ())
        {
        case CapillaryNetDevice::COORDINATOR:
          break;
        case CapillaryNetDevice::END_DEVICE:

          if (m_currSlot == m_rndSlot)
            {
              MAC_DEBUG ("TX on Slot: " << m_currSlot);
              ForwardDown (m_currentPkt);
            }
          break;
        }

      Simulator::Schedule (GetSlotDuration (), &FsalohaMac::StopSlot, this);
    }
}

void
FsalohaMac::StopSlot (void)
{
  NS_LOG_FUNCTION (this);

  MAC_DEBUG ("Stop Slot: " << m_currSlot);

  if (m_activeDCR == CapillaryMac::ACTIVE_START)
    {
      switch (m_dev->GetType ())
        {
        case CapillaryNetDevice::COORDINATOR:
          break;
        case CapillaryNetDevice::END_DEVICE:
          if ((m_currSlot == m_rndSlot) && (m_nSlots - 1 - m_rndSlot) * GetSlotDuration () > 2 * m_phy->GetSwitchingTime ())
            {
              m_phy->ForceSleep ();

              Simulator::Schedule ((m_nSlots - 1 - m_rndSlot) * GetSlotDuration () - m_phy->GetSwitchingTime (), &CapillaryPhy::WakeUp, m_phy);
            }
          break;
        }

      m_currSlot++;


      if (m_currSlot < m_nSlots)
        {
          FsalohaMac::StartSlot ();
        }
      else
        {
          FsalohaMac::StopFrame ();
        }
    }
}

void FsalohaMac::StopFrame (void)
{
  NS_LOG_FUNCTION (this);

  if (m_activeDCR == CapillaryMac::ACTIVE_START)
    {

      switch (m_dev->GetType ())
        {
        case CapillaryNetDevice::COORDINATOR:
          Simulator::Schedule (m_maxDelay, &FsalohaMac::SendFeedback, this);
          break;
        case CapillaryNetDevice::END_DEVICE:
          break;
        }
    }
}

bool FsalohaMac::SendRequestForData (void)
{
  NS_LOG_FUNCTION (this);

  MAC_DEBUG ("Try to send RFD");

  CapillaryMacHeader macHdr (CapillaryMacHeader::CAPILLARY_MAC_RFD);
  macHdr.SetSrcAddr (Mac64Address::ConvertFrom (GetAddress ()));
  macHdr.SetDstAddr (Mac64Address::ConvertFrom (GetBroadcast ()));

  Ptr<Packet> p = Create<Packet> (m_SigSeqNum);
  m_SigSeqNum++;
  LlcSnapHeader llc;
  llc.SetType (NetDevice::PACKET_BROADCAST);
  p->AddHeader (llc);

  p->AddHeader (macHdr);

  if (ForwardDown (p))
    {
      return true;
    }

  Simulator::Schedule (m_phy->GetSwitchingTime (), &FsalohaMac::WakeUp, this);
  return false;
}

bool FsalohaMac::SendFeedback (void)
{
  NS_LOG_FUNCTION (this);

  MAC_DEBUG ("Try to send FBP");

  CapillaryMacHeader macHdr (CapillaryMacHeader::CAPILLARY_MAC_FBP);
  macHdr.SetSeqNum (m_SigSeqNum);
  m_SigSeqNum++;
  macHdr.SetSrcAddr (Mac64Address::ConvertFrom (GetAddress ()));
  macHdr.SetDstAddr (Mac64Address::ConvertFrom (GetBroadcast ()));

  int length = 0;

  if ((m_nSlots * 2) % 8)
    {
      length = (m_nSlots * 2 / 8) + 1;
    }
  else
    {
      length = (m_nSlots * 2 / 8);
    }

  uint8_t payload[length];
  SerializeFBP (payload, length);

  Ptr<Packet> p = Create<Packet> (payload, length);

  LlcSnapHeader llc;
  llc.SetType (NetDevice::PACKET_BROADCAST);
  p->AddHeader (llc);

  p->AddHeader (macHdr);

  return ForwardDown (p);
}

bool FsalohaMac::ForwardDown (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);

  p->AddTrailer (CapillaryMacTrailer (p));

  if (!m_phy->StartTx (p))
    {
      MAC_DEBUG ("Packet forwarded down. (size=" << p->GetSerializedSize () << ").");
      return true;
    }

  MAC_DEBUG ("PHY: " << m_phy->GetStatus ());
  return false;
}

Ptr<CapillaryController> FsalohaMac::GetController () const
{
  NS_LOG_FUNCTION (this);
  return m_controller;
}

void FsalohaMac::SetController (Ptr<CapillaryController> controller)
{
  NS_LOG_FUNCTION (this);
  m_controller = controller;
}

std::ostream& operator<< (std::ostream& os, std::vector<FsalohaMac::SlotState> states)
{
  os << "[ ";

  for (uint32_t i = 0; i < states.size (); i++)
    {
      os << states[i];
      os << ", ";
    }

  os << " ]";

  return os;
}

std::ostream& operator<< (std::ostream& os, FsalohaMac::SlotState state)
{
  switch (state)
    {
    case  FsalohaMac::EMPTY:
      os << "EMPTY";
      break;
    case  FsalohaMac::OK:
      os << "OK";
      break;
    case  FsalohaMac::ERROR:
      os << "ERROR";
      break;
    }

  return os;
}

} /* namespace ns3 */
