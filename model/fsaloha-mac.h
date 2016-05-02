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
#ifndef MODEL_FSALOHA_MAC_H_
#define MODEL_FSALOHA_MAC_H_

#include <ns3/capillary-mac.h>
#include <ns3/mac64-address.h>
#include <ns3/capillary-controller.h>
#include <ns3/capillary-net-device.h>
#include <ns3/nstime.h>
#include <ns3/ptr.h>
#include <ns3/queue.h>
#include <ns3/random-variable-stream.h>
#include <iostream>
#include <vector>


namespace ns3 {

class CapillaryPhy;
class Packet;

/*
 *
 */
class FsalohaMac : public CapillaryMac
{
public:
  typedef enum
  {
    EMPTY = 0x00,
    OK = 0x01,
    ERROR = 0x02
  } SlotState;


  FsalohaMac ();
  virtual ~FsalohaMac ();

  static TypeId GetTypeId (void);

  Time GetSlotDuration (void) const;
  uint16_t GetNSlots (void) const;

  /** Inherited Methods*/
  virtual void SetDevice (Ptr<NetDevice> d);
  virtual Ptr<NetDevice> GetDevice (void);
  virtual void SetPhy (Ptr<CapillaryPhy> phy);
  virtual Ptr<CapillaryPhy> GetPhy (void);
  virtual Address GetAddress (void);
  virtual void SetAddress (Address addr);
  virtual Address GetBroadcast (void) const;
  virtual Address GetCoordinator (void) const;
  virtual bool DataEnqueue (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
  virtual bool SetMtu (const uint16_t mtu);
  virtual uint16_t GetMtu (void) const;
  virtual void WakeUp (void);
  virtual void ForceSleep (void);
  virtual Ptr<CapillaryController> GetController () const;
  virtual void SetController (Ptr<CapillaryController> controller);

protected:
  virtual void DoInitialize (void);
  virtual void DoDispose (void);

  void SetNSlots (const uint16_t nSlots);
  void SetRandomStream (Ptr<UniformRandomVariable> random);
  Ptr<UniformRandomVariable> GetRandomStream (void) const;

  bool TrasmissionEnqueue (void);

  /**
   * Notify the MAC that the PHY is starting a packet transmission.
   *
   */
  void NotifyTransmissionStart (Ptr<const Packet> p);

  /**
   * Notify the MAC that the PHY has finished a previously started transmission
   *
   */
  void NotifyTransmissionEnd (Ptr<const Packet> p);

  /**
   * Notify the MAC that the PHY has started a reception
   *
   */
  void NotifyReceptionStart (void);

  /**
   * Notify the MAC that the PHY finished a reception with an error
   *
   */
  void NotifyReceptionEndError (void);

  /**
   * Notify the MAC that the PHY finished a reception successfully
   *
   * @param p the received packet
   */
  void NotifyReceptionEndOk (Ptr<Packet> p);

  void SerializeFBP (uint8_t *payload, uint32_t length);
  void DeserializeFBP (uint8_t *payload, uint32_t length);

  void StartActivePeriod (void);
  void NotifyActivePeriodStopped (void);
  void NotifyActivePeriodAborted (void);

  void StartNonActivePeriod (void);
  void NotifyNonActivePeriodStopped (void);

  void ResetFrame (void);
  void StartFrame (void);
  void StopFrame (void);

  void StartSlot (void);
  void StopSlot (void);

  bool SendRequestForData (void);
  bool SendFeedback (void);

  bool ForwardDown (Ptr<Packet> p);

private:
  Ptr<CapillaryNetDevice> m_dev;

  /**
   * The Packets Queues.
   */
  Ptr<Queue> m_queue;
  Ptr<Queue> m_TxQueue;

  uint32_t m_NPackets;
  Ptr<Packet> m_currentPkt;

  Mac64Address m_addr;
  Ptr<CapillaryPhy> m_phy;
  Ptr<UniformRandomVariable> m_random;

  uint16_t m_rndSlot;
  uint16_t m_currSlot;
  uint16_t m_nSlots;

  uint16_t m_mtu;
  uint8_t m_SigSeqNum;
  uint8_t m_DataSeqNum;

  Time m_maxDelay;

  Time m_startFrame;

  Time m_nextDCR;

  std::vector<SlotState> m_slotStatus;

  /** Controller */
  Ptr<CapillaryController> m_controller;

  /** Forwarding up callback. */
  ForwardUpCallback m_fwdUp;


  int m_nFramesDCR;

  /** Trace*/
  TracedValue<int> m_nFrames;
};

std::ostream& operator<< (std::ostream& os, std::vector<FsalohaMac::SlotState> states);
std::ostream& operator<< (std::ostream& os, FsalohaMac::SlotState state);

} /* namespace ns3 */

#endif /* MODEL_FSALOHA_MAC_H_ */
