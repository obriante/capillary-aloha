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

#include "capillary-net-device.h"
#include "capillary-mac-header.h"

#include <ns3/abort.h>
#include <ns3/address.h>
#include <ns3/assert.h>
#include <ns3/llc-snap-header.h>
#include <ns3/log.h>
#include <ns3/log-macros-disabled.h>
#include <ns3/mobility-model.h>
#include <ns3/object-base.h>
#include <ns3/pointer.h>
#include <ns3/type-id.h>
#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CapillaryNetDevice");

NS_OBJECT_ENSURE_REGISTERED (CapillaryNetDevice);

CapillaryNetDevice::CapillaryNetDevice ()
{
  NS_LOG_FUNCTION (this);

  m_linkUp = false;
}

CapillaryNetDevice::~CapillaryNetDevice ()
{
  NS_LOG_FUNCTION (this);
}

TypeId CapillaryNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CapillaryNetDevice")
    .SetParent<NetDevice> ()
    .SetGroupName ("m2m-capillary")
    .AddConstructor<CapillaryNetDevice> ()
    .AddAttribute ("Channel",
                   "The channel attached to this device", PointerValue (),
                   MakePointerAccessor (&CapillaryNetDevice::GetChannel,
                                        &CapillaryNetDevice::SetChannel),
                   MakePointerChecker<SpectrumChannel> ())
    .AddAttribute ("Phy",
                   "The PHY layer attached to this device.", PointerValue (),
                   MakePointerAccessor (&CapillaryNetDevice::GetPhy,
                                        &CapillaryNetDevice::SetPhy),
                   MakePointerChecker<CapillaryPhy> ())
    .AddAttribute ("Mac",
                   "The MAC layer attached to this device.", PointerValue (),
                   MakePointerAccessor (&CapillaryNetDevice::GetMac,
                                        &CapillaryNetDevice::SetMac),
                   MakePointerChecker<CapillaryMac> ())
    .AddAttribute ("Type",
                   "(i.e., coordinator, end-device)", EnumValue (END_DEVICE),
                   MakeEnumAccessor (&CapillaryNetDevice::SetType, &CapillaryNetDevice::GetType),
                   MakeEnumChecker (COORDINATOR, "coordinator", END_DEVICE,"end-device"))
  ;
  return tid;
}

void CapillaryNetDevice::SetMac (Ptr<CapillaryMac> mac)
{
  NS_LOG_FUNCTION (this << mac);
  NS_ASSERT (mac);
  m_mac = mac;
  m_mac->SetAddress (m_addr);
  if (m_phy)
    {
      m_mac->SetPhy (m_phy);
    }

  m_mac->SetAttribute ("ForwardUpCallback",CallbackValue (MakeCallback (&CapillaryNetDevice::ForwardUp,this)));
}

Ptr<CapillaryMac> CapillaryNetDevice::GetMac (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mac;
}

void CapillaryNetDevice::SetPhy (Ptr<CapillaryPhy> phy)
{
  NS_LOG_FUNCTION (this << phy);
  NS_ASSERT (phy);
  m_phy = phy;
  m_phy->SetMobility (m_node->GetObject<MobilityModel> ());
  m_phy->SetDevice (this);

  if (m_mac)
    {
      m_mac->SetPhy (m_phy);
    }

  if (m_channel)
    {
      m_phy->SetChannel (m_channel);
      m_channel->AddRx (m_phy);
    }

}

Ptr<CapillaryPhy> CapillaryNetDevice::GetPhy (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy;
}

void CapillaryNetDevice::SetChannel (Ptr<SpectrumChannel> channel)
{
  NS_LOG_FUNCTION (this << channel);
  NS_ASSERT (channel);
  m_channel = channel;

  if (m_phy)
    {
      m_phy->SetChannel (m_channel);
      m_channel->AddRx (m_phy);
    }
}

Ptr<Channel> CapillaryNetDevice::GetChannel () const
{
  NS_LOG_FUNCTION (this);
  return m_channel;
}

void CapillaryNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_channel = 0;
  //m_mac = 0;
  m_phy = 0;
  m_node = 0;

  //m_receiveCallback.Nullify ();
  // chain up.
  NetDevice::DoDispose ();
}

void CapillaryNetDevice::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  NetDevice::DoInitialize ();
}

void CapillaryNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  m_ifIndex = index;
}

uint32_t CapillaryNetDevice::GetIfIndex (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ifIndex;
}

void CapillaryNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this << address);
  m_addr = Mac64Address::ConvertFrom (address);

  if (m_mac)
    {
      m_mac->SetAddress (m_addr);
    }
}

Address CapillaryNetDevice::GetAddress (void) const
{
  NS_LOG_FUNCTION (this);
  return m_addr;
}

bool CapillaryNetDevice::SetMtu (const uint16_t mtu)
{
  NS_LOG_FUNCTION (this << mtu);
  return m_mac->SetMtu (mtu);
}

uint16_t CapillaryNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mac->GetMtu ();
}

bool CapillaryNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy != 0 && m_linkUp;
}

void CapillaryNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION (this);
  m_linkChanges.ConnectWithoutContext (callback);
}

bool CapillaryNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

Address CapillaryNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mac->GetBroadcast ();
}

bool CapillaryNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

Address CapillaryNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_ABORT_MSG ("Unsupported");
  return Address ();
}

Address CapillaryNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this << addr);
  return m_mac->GetBroadcast ();
}

bool CapillaryNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool CapillaryNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool CapillaryNetDevice::Send (Ptr<Packet> packet, const Address& dest,
                               uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (packet << dest << protocolNumber);

  return SendFrom (packet,m_addr, dest, protocolNumber);
}

bool CapillaryNetDevice::SendFrom (Ptr<Packet> packet, const Address& source,
                                   const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (packet << source << dest << protocolNumber);

  if (m_linkUp)
    {
      return m_mac->DataEnqueue (packet, source, dest, protocolNumber);
    }

  return false;
}

Ptr<Node> CapillaryNetDevice::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}

void CapillaryNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;

}

CapillaryNetDevice::Type CapillaryNetDevice::GetType (void) const
{
  NS_LOG_FUNCTION (this);
  return m_type;
}

void CapillaryNetDevice::SetType (const CapillaryNetDevice::Type type)
{
  NS_LOG_FUNCTION (this);
  m_type = type;
}

bool CapillaryNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

void CapillaryNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this);
  m_receiveCallback = cb;
}

void CapillaryNetDevice::SetPromiscReceiveCallback (
  PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION (this);
  m_promiscRxCallback = cb;
}

bool CapillaryNetDevice::SupportsSendFrom (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

void CapillaryNetDevice::LinkUp (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_mac);
  m_mac->WakeUp ();
  m_linkUp = true;
  m_linkChanges ();
}

void CapillaryNetDevice::LinkDown (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_mac);
  m_mac->ForceSleep ();
  m_linkUp = false;
  m_linkChanges ();
}

void CapillaryNetDevice::ForwardUp (Ptr<Packet> packet, LlcSnapHeader llc, Address src, Address dest)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (packet);

  PacketType packetType;
  if (src == m_mac->GetBroadcast ())
    {
      packetType = PACKET_BROADCAST;
    }
//   else if (header.GetDestination ().IsGroup ())
//     {
//       packetType = PACKET_MULTICAST;
//     }
  else if (src == m_addr)
    {
      packetType = PACKET_HOST;
    }
  else
    {
      packetType = PACKET_OTHERHOST;
    }

  NS_LOG_LOGIC ("packet type = " << packetType);

  if (!m_promiscRxCallback.IsNull ())
    {
      m_promiscRxCallback (this, packet->Copy (), llc.GetType (), src, dest, packetType);
    }

  if (packetType != PACKET_OTHERHOST)
    {
      m_receiveCallback (this, packet, llc.GetType (), src );
    }

}

} /* namespace ns3 */
