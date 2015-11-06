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

#include "capillary-net-device-helper.h"

#include <ns3/simulator.h>
#include <ns3/address.h>
#include <ns3/assert.h>
#include <ns3/callback.h>
#include <ns3/capillary-mac.h>
#include <ns3/capillary-net-device.h>
#include <ns3/capillary-phy-ideal.h>
#include <ns3/capillary-controller.h>
#include <ns3/net-device-container.h>
#include <ns3/log.h>
#include <ns3/log-macros-disabled.h>
#include <ns3/mac64-address.h>
#include <ns3/mobility-model.h>
#include <ns3/names.h>
#include <ns3/queue.h>
#include <ns3/spectrum-value.h>
#include <ns3/wifi-spectrum-value-helper.h>
#include <iterator>

#include <ns3/antenna-model.h>
#include <ns3/pointer.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CapillaryNetDeviceHelper");

CapillaryNetDeviceHelper::CapillaryNetDeviceHelper ()
{
  NS_LOG_FUNCTION (this);
  // set types
  m_phy.SetTypeId ("ns3::CapillaryPhyIdeal");
  m_mac.SetTypeId ("ns3::FsalohaMac");
  //m_queue.SetTypeId ("ns3::DropTailQueue");
  m_dev.SetTypeId ("ns3::CapillaryNetDevice");
  m_antenna.SetTypeId ("ns3::IsotropicAntennaModel");
  m_controller.SetTypeId ("ns3::BasicController");
  m_random = CreateObject<UniformRandomVariable> ();
}

CapillaryNetDeviceHelper::~CapillaryNetDeviceHelper ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<SpectrumChannel> CapillaryNetDeviceHelper::GetChannel (void)
{
  NS_LOG_FUNCTION (this);
  return m_channel;
}

void CapillaryNetDeviceHelper::SetChannel (Ptr<SpectrumChannel> channel)
{
  NS_LOG_FUNCTION (this);
  m_channel = channel;
}

void
CapillaryNetDeviceHelper::SetTxPowerSpectralDensity (Ptr<SpectrumValue> txPsd)
{
  NS_LOG_FUNCTION (this << txPsd);
  m_txPsd = txPsd;
}

void
CapillaryNetDeviceHelper::SetNoisePowerSpectralDensity (Ptr<SpectrumValue> noisePsd)
{
  NS_LOG_FUNCTION (this << noisePsd);
  m_noisePsd = noisePsd;
}

void CapillaryNetDeviceHelper::SetPhyTypeId (std::string name)
{
  NS_LOG_FUNCTION (this << name);
  m_phy.SetTypeId (name);

}

void CapillaryNetDeviceHelper::SetPhyAttribute (std::string name, const AttributeValue& v)
{
  NS_LOG_FUNCTION (this);
  m_phy.Set (name, v);
}

void CapillaryNetDeviceHelper::SetMacTypeId (std::string name)
{
  NS_LOG_FUNCTION (this << name);
  m_mac.SetTypeId (name);
}

void CapillaryNetDeviceHelper::SetMacAttribute (std::string name, const AttributeValue& v)
{
  NS_LOG_FUNCTION (this);
  m_mac.Set (name, v);
}

void CapillaryNetDeviceHelper::SetDeviceAttribute (std::string name, const AttributeValue& v1)
{
  NS_LOG_FUNCTION (this);
  m_dev.Set (name, v1);
}

void CapillaryNetDeviceHelper::SetControllerTypeId (std::string name)
{
  NS_LOG_FUNCTION (this << name);
  m_controller.SetTypeId (name);

}
void CapillaryNetDeviceHelper::SetControllerAttribute (std::string name, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_controller.Set (name,v);
}

void CapillaryNetDeviceHelper::SetAntenna (std::string type,
                                           std::string n0, const AttributeValue& v0,
                                           std::string n1, const AttributeValue& v1,
                                           std::string n2, const AttributeValue& v2,
                                           std::string n3, const AttributeValue& v3,
                                           std::string n4, const AttributeValue& v4,
                                           std::string n5, const AttributeValue& v5,
                                           std::string n6, const AttributeValue& v6,
                                           std::string n7, const AttributeValue& v7)
{
  NS_LOG_FUNCTION (this);

  m_antenna.SetTypeId (type);
  m_antenna.Set (n0, v0);
  m_antenna.Set (n1, v1);
  m_antenna.Set (n2, v2);
  m_antenna.Set (n3, v3);
  m_antenna.Set (n4, v4);
  m_antenna.Set (n5, v5);
  m_antenna.Set (n6, v6);
  m_antenna.Set (n7, v7);
}

NetDeviceContainer CapillaryNetDeviceHelper::Install (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);

  NetDeviceContainer devices;

  Ptr<CapillaryNetDevice> dev = (m_dev.Create ())->GetObject<CapillaryNetDevice> ();
  NS_ASSERT (dev);
  dev->SetNode (node);
  dev->SetAddress (Mac64Address::Allocate ());


  Ptr<CapillaryPhy> phy = (m_phy.Create ())->GetObject<CapillaryPhy> ();
  NS_ASSERT (phy);
  phy->SetDevice (dev);
  NS_ASSERT_MSG (m_txPsd, "you forgot to call CapillaryNetDeviceHelper::SetTxPowerSpectralDensity ()");
  phy->SetTxPowerSpectralDensity (m_txPsd);
  NS_ASSERT_MSG (m_noisePsd, "you forgot to call CapillaryNetDeviceHelper::SetNoisePowerSpectralDensity ()");
  phy->SetNoisePowerSpectralDensity (m_noisePsd);

  dev->SetType (CapillaryNetDevice::END_DEVICE);
  dev->SetPhy (phy);


  Ptr<CapillaryMac> mac = (m_mac.Create ())->GetObject<CapillaryMac> ();
  NS_ASSERT (mac);
  mac->SetAttribute ("RandomStream", PointerValue (m_random));
  mac->SetDevice (dev);
  mac->SetPhy (phy);

  //Ptr<Queue> q = (m_queue.Create ())->GetObject<Queue> ();
  //mac->SetAttribute ("Queue", PointerValue (q));

  dev->SetMac (mac);


  Ptr<CapillaryController> controller = (m_controller.Create ())->GetObject<CapillaryController> ();
  NS_ASSERT (controller);
  controller->SetNode (node);
  controller->SetMac (mac);

  mac->SetController (controller);


  NS_ASSERT (node);
  phy->SetMobility (node->GetObject<MobilityModel> ());

  phy->SetChannel (m_channel);
  dev->SetChannel (m_channel);

  Ptr<AntennaModel> antenna = (m_antenna.Create ())->GetObject<AntennaModel> ();
  NS_ASSERT_MSG (antenna, "error in creating the AntennaModel object");
  phy->SetAntenna (antenna);

  node->AddDevice (dev);

  devices.Add (dev);

  Simulator::Schedule (Seconds (0), &CapillaryNetDevice::LinkUp,dev);

  return devices;
}

NetDeviceContainer CapillaryNetDeviceHelper::Install (std::string nodeName) const
{
  NS_LOG_FUNCTION (this);

  Ptr<Node> node = Names::Find<Node> (nodeName);
  return Install (node);
}

NetDeviceContainer CapillaryNetDeviceHelper::Install (NodeContainer c) const
{
  NS_LOG_FUNCTION (this);

  NetDeviceContainer devices;

  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      devices.Add (Install (*i));
    }
  return devices;
}

Ptr<CapillaryNetDevice> CapillaryNetDeviceHelper::InstallCoordinator (Ptr<Node> device)
{
  NS_LOG_FUNCTION (this);

  NetDeviceContainer dev = Install (device);
  return SetCoordinator (dev.Get (0));
}

Ptr<CapillaryNetDevice> CapillaryNetDeviceHelper::SetCoordinator (Ptr<NetDevice> device)
{
  NS_LOG_FUNCTION (this);

  Ptr<CapillaryNetDevice> coordinator = DynamicCast<CapillaryNetDevice> (device);
  NS_ASSERT (coordinator);
  coordinator->SetType (CapillaryNetDevice::COORDINATOR);
  return coordinator;
}

} /* namespace ns3 */
