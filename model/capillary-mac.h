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
#ifndef MODEL_CAPILLARY_MAC_H_
#define MODEL_CAPILLARY_MAC_H_

#include <ns3/callback.h>
#include <ns3/packet.h>
#include <ns3/object.h>
#include <ns3/net-device.h>
#include <ns3/ptr.h>
#include <ns3/traced-callback.h>
#include <ns3/traced-value.h>

#include "capillary-controller.h"


namespace ns3 {

class Address;
class CapillaryPhy;
class LlcSnapHeader;

typedef Callback<void, Ptr<Packet>, LlcSnapHeader, Address, Address > ForwardUpCallback;

typedef Callback<void, const Address& > StartDCRCallback;
typedef Callback<void, const Address& > StopDCRCallback;
typedef Callback<void, const Address& > AbortDCRCallback;

/*
 *
 */
class CapillaryMac : public Object
{
public:
  typedef enum
  {
    ACTIVE_START,
    ACTIVE_STOP,
    ACTIVE_ABORT,
    NON_ACTIVE_STOP,
    NON_ACTIVE_START
  } DcrStatus;

  CapillaryMac (void);
  virtual ~CapillaryMac (void);

  static TypeId GetTypeId (void);

  virtual void SetDevice (Ptr<NetDevice> d) = 0;
  virtual Ptr<NetDevice> GetDevice (void) = 0;

  /**
   * Set the PHY Layer for the MAC.
   *
   * \param phy the PHY
   */
  virtual void SetPhy (Ptr<CapillaryPhy> phy) = 0;

  /**
   * Get the PHY Layer of the MAC.
   *
   * \return the PHY
   */
  virtual Ptr<CapillaryPhy> GetPhy (void) = 0;

  /**
   * Get the MAC Mac64Address.
   *
   * \return MAC Mac64Address.
   */
  virtual Address GetAddress (void) = 0;

  /**
   * Set the address.
   *
   * \param addr Mac64Address for this MAC.
   */
  virtual void SetAddress (Address addr) = 0;

  /**
   * Get the broadcast address.
   *
   * \return The broadcast address.
   */
  virtual Address GetBroadcast (void) const = 0;

  /**
   * Get the coordinator address.
   *
   * \return The coordinator address.
   */
  virtual Address GetCoordinator (void) const = 0;

  /**
   * Enqueue Data packets into the Data Queue
   *
   * \param p The packet to enqueue.
   */
  virtual bool DataEnqueue (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber) = 0;

  /**
   * Set the max mtu
   *
   * \param mtu the mtu
   */
  virtual bool SetMtu (const uint16_t mtu) = 0;

  /**
   * Get the max mtu
   *
   * \return the mtu
   */
  virtual uint16_t GetMtu (void) const = 0;

  /**
   * Wake Up the device
   */
  virtual void WakeUp (void) = 0;

  /**
   * Impose the SLEEP state
   */
  virtual void ForceSleep (void) = 0;
  virtual Ptr<CapillaryController> GetController () const = 0;
  virtual void SetController (Ptr<CapillaryController> controller) = 0;

protected:
  virtual void DoInitialize (void);
  virtual void DoDispose (void);


  /**
   * A trace source that emulates a non-promiscuous protocol sniffer connected
   * to the device.  Unlike your average everyday sniffer, this trace source
   * will not fire on PACKET_OTHERHOST events.
   *
   * On the transmit size, this trace hook will fire after a packet is dequeued
   * from the device queue for transmission.  In Linux, for example, this would
   * correspond to the point just before a device hard_start_xmit where
   * dev_queue_xmit_nit is called to dispatch the packet to the PF_PACKET
   * ETH_P_ALL handlers.
   *
   * On the receive side, this trace hook will fire when a packet is received,
   * just before the receive callback is executed.  In Linux, for example,
   * this would correspond to the point at which the packet is dispatched to
   * packet sniffers in netif_receive_skb.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_snifferTrace;

  /**
   * A trace source that emulates a promiscuous mode protocol sniffer connected
   * to the device.  This trace source fire on packets destined for any host
   * just like your average everyday packet sniffer.
   *
   * On the transmit size, this trace hook will fire after a packet is dequeued
   * from the device queue for transmission.  In Linux, for example, this would
   * correspond to the point just before a device hard_start_xmit where
   * dev_queue_xmit_nit is called to dispatch the packet to the PF_PACKET
   * ETH_P_ALL handlers.
   *
   * On the receive side, this trace hook will fire when a packet is received,
   * just before the receive callback is executed.  In Linux, for example,
   * this would correspond to the point at which the packet is dispatched to
   * packet sniffers in netif_receive_skb.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_promiscSnifferTrace;

  /**
    * The trace source fired when packets are being sent down.
    *
    * \see class CallBackTraceSource
    */
  TracedCallback<Ptr<const Packet> > m_macTxTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * immediately before being forwarded up to higher layers.
   * This is a non-promiscuous trace.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macRxTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * but dropped before being forwarded up to higher layers.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macRxDropTrace;

  /**
   * The trace source fired when packets come into the "top" of the device
   * at the L3/L2 transition, when being queued for transmission.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxEnqueueTrace;

  /**
   * The trace source fired when packets are dequeued from the
   * L3/l2 transmission queue.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxDequeueTrace;


  StartDCRCallback startDCRCallback;
  StopDCRCallback stopDCRCallback;
  AbortDCRCallback abortDCRCallback;

  TracedValue<DcrStatus> m_activeDCR;

};

} /* namespace ns3 */

#endif /* MODEL_CAPILLARY_MAC_H_ */
