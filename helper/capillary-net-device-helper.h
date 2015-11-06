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
#ifndef FSALOHA_END_DEVICE_HELPER_H_
#define FSALOHA_END_DEVICE_HELPER_H_

#include <ns3/node.h>
#include <ns3/node-container.h>
#include <ns3/object-factory.h>
#include <ns3/ptr.h>
#include <ns3/spectrum-channel.h>
#include <ns3/spectrum-value.h>
#include <ns3/random-variable-stream.h>
#include <string>


namespace ns3 {
class NetDeviceContainer;
class CapillaryNetDevice;
} /* namespace ns3 */

namespace ns3 {

/*
 *
 */
class CapillaryNetDeviceHelper
{
public:
  CapillaryNetDeviceHelper ();
  virtual ~CapillaryNetDeviceHelper ();

  /**
   * \brief Get the channel associated to this helper
   * \returns the channel
   */
  Ptr<SpectrumChannel> GetChannel (void);

  /**
   * \brief Set the channel associated to this helper
   * \param channel the channel
   */
  void SetChannel (Ptr<SpectrumChannel> channel);

  /**
   *
   * @param txPsd the Power Spectral Density to be used for transmission by all created PHY instances
   */
  void SetTxPowerSpectralDensity (Ptr<SpectrumValue> txPsd);

  /**
   *
   * @param noisePsd the Power Spectral Density to be used for transmission by all created PHY instances
   */
  void SetNoisePowerSpectralDensity (Ptr<SpectrumValue> noisePsd);

  void SetPhyTypeId (std::string name);

  /**
   * @param name the name of the attribute to set
   * @param v the value of the attribute
   *
   * Set these attributes on each CapillaryPhy instance to be created
   */
  void SetPhyAttribute (std::string name, const AttributeValue &v);

  void SetMacTypeId (std::string name);

  /**
   * @param name the name of the attribute to set
   * @param v the value of the attribute
   *
   * Set these attributes on each CapillaryMac instance to be created
   */
  void SetMacAttribute (std::string name, const AttributeValue &v);

  void SetNSlots (uint16_t nSlots);

  void SetControllerTypeId (std::string name);
  void SetControllerAttribute (std::string name, const AttributeValue &v);

  /**
   * @param n1 the name of the attribute to set
   * @param v1 the value of the attribute to set
   *
   * Set these attributes on each FsalohaCapillaryDevice created
   */
  void SetDeviceAttribute (std::string name, const AttributeValue &v1);

  /**
   * \param type the type of the model to set
   * \param n0 the name of the attribute to set
   * \param v0 the value of the attribute to set
   * \param n1 the name of the attribute to set
   * \param v1 the value of the attribute to set
   * \param n2 the name of the attribute to set
   * \param v2 the value of the attribute to set
   * \param n3 the name of the attribute to set
   * \param v3 the value of the attribute to set
   * \param n4 the name of the attribute to set
   * \param v4 the value of the attribute to set
   * \param n5 the name of the attribute to set
   * \param v5 the value of the attribute to set
   * \param n6 the name of the attribute to set
   * \param v6 the value of the attribute to set
   * \param n7 the name of the attribute to set
   * \param v7 the value of the attribute to set
   *
   * Configure the AntennaModel instance for each new device to be created
   */
  void SetAntenna (std::string type,
                   std::string n0, const AttributeValue& v0,
                   std::string n1, const AttributeValue& v1,
                   std::string n2, const AttributeValue& v2,
                   std::string n3, const AttributeValue& v3,
                   std::string n4, const AttributeValue& v4,
                   std::string n5, const AttributeValue& v5,
                   std::string n6, const AttributeValue& v6,
                   std::string n7, const AttributeValue& v7);
  /**
   * @param node the node on which a device must be created
   * \returns a device container which contains all the devices created by this method.
   */
  NetDeviceContainer Install (Ptr<Node> node) const;
  /**
   * @param nodeName the name of node on which a device must be created
   * @return a device container which contains all the devices created by this method.
   */
  NetDeviceContainer Install (std::string nodeName) const;

  NetDeviceContainer Install (NodeContainer c) const;

  Ptr<CapillaryNetDevice> InstallCoordinator (Ptr<Node> device);
  Ptr<CapillaryNetDevice> SetCoordinator (Ptr<NetDevice> device);


protected:
  Ptr<SpectrumChannel> m_channel;
  Ptr<SpectrumValue> m_txPsd;
  Ptr<SpectrumValue> m_noisePsd;

  Ptr<UniformRandomVariable> m_random;

  ObjectFactory m_antenna;
  ObjectFactory m_phy;
  ObjectFactory m_mac;
  // ObjectFactory m_queue;
  ObjectFactory m_dev;
  ObjectFactory m_controller;
};

} /* namespace ns3 */

#endif /* FSALOHA_END_DEVICE_HELPER_H_ */
