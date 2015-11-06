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
#ifndef FSALOHA_CAPILLARY_DEVICE_H_
#define FSALOHA_CAPILLARY_DEVICE_H_

#include <ns3/callback.h>
#include <ns3/llc-snap-header.h>
#include <ns3/capillary-mac.h>
#include <ns3/capillary-mac-header.h>
#include <ns3/capillary-phy.h>
#include <ns3/ipv4-address.h>
#include <ns3/ipv6-address.h>
#include <ns3/mac64-address.h>
#include <ns3/net-device.h>
#include <ns3/node.h>
#include <ns3/packet.h>
#include <ns3/ptr.h>
#include <ns3/spectrum-channel.h>
#include <ns3/traced-callback.h>

namespace ns3 {

/*
 *
 */
class CapillaryNetDevice : public NetDevice
{

public:
  typedef enum
  {
    COORDINATOR,
    END_DEVICE
  } Type;

  CapillaryNetDevice ();
  virtual ~CapillaryNetDevice ();

  static TypeId GetTypeId (void);
  void SetMac (Ptr<CapillaryMac> mac);
  Ptr<CapillaryMac> GetMac (void) const;
  void SetPhy (Ptr<CapillaryPhy> phy);
  Ptr<CapillaryPhy> GetPhy (void) const;
  void SetChannel (Ptr<SpectrumChannel> channel);

  /**
   * Get the Device Type
   *
   *\return COORDINATOR, END_DEVICE,
   */
  CapillaryNetDevice::Type GetType (void) const;
  /**
   * Set the Device Type
   *
   *\param type: COORDINATOR, END_DEVICE,
   */
  void SetType (const CapillaryNetDevice::Type type);

  /**
   * Mark NetDevice link as up.
   */
  void LinkUp (void);

  /**
   * Mark NetDevice link as down.
   */
  void LinkDown (void);

  void ForwardUp (Ptr<Packet> packet, LlcSnapHeader llc, Address src, Address dest);


  // From class NetDevice
  virtual void SetIfIndex (const uint32_t index);
  virtual uint32_t GetIfIndex (void) const;
  virtual Ptr<Channel> GetChannel (void) const;

  /**
   * This method indirects to LrWpanMac::SetShortAddress ()
   */
  virtual void SetAddress (Address address);
  /**
   * This method indirects to LrWpanMac::SetShortAddress ()
   */
  virtual Address GetAddress (void) const;
  virtual bool SetMtu (const uint16_t mtu);
  virtual uint16_t GetMtu (void) const;
  virtual bool IsLinkUp (void) const;
  virtual void AddLinkChangeCallback (Callback<void> callback);
  virtual bool IsBroadcast (void) const;
  virtual Address GetBroadcast (void) const;
  virtual bool IsMulticast (void) const;
  virtual Address GetMulticast (Ipv4Address multicastGroup) const;
  virtual Address GetMulticast (Ipv6Address addr) const;
  virtual bool IsBridge (void) const;
  virtual bool IsPointToPoint (void) const;
  virtual bool Send (Ptr<Packet> packet, const Address& dest,
                     uint16_t protocolNumber);
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source,
                         const Address& dest, uint16_t protocolNumber);
  virtual Ptr<Node> GetNode (void) const;
  virtual void SetNode (Ptr<Node> node);
  virtual bool NeedsArp (void) const;

  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);
  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom (void) const;

protected:
  // Inherited from NetDevice/Object
  virtual void DoDispose (void);
  virtual void DoInitialize (void);

private:
  Ptr<SpectrumChannel> m_channel;
  Ptr<CapillaryMac> m_mac;
  Ptr<CapillaryPhy> m_phy;
  Ptr<Node> m_node;
  Type m_type;

  Mac64Address m_addr;
  bool m_linkUp;
  uint32_t m_ifIndex;

  /**
   * Trace source for link up/down changes.
   */
  TracedCallback<> m_linkChanges;

  /**
   * Upper layer callback used for notification of new data packet arrivals.
   */
  ReceiveCallback m_receiveCallback;
  PromiscReceiveCallback m_promiscRxCallback;
};

} /* namespace ns3 */

#endif /* MODEL_FSALOHA_CAPILLARY_DEVICE_H_ */
