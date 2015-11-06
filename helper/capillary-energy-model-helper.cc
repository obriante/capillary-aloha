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
#include <ns3/capillary-energy-model.h>
#include <ns3/capillary-energy-model-helper.h>
#include <ns3/capillary-mac.h>
#include <ns3/capillary-net-device.h>
#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <ns3/log-macros-disabled.h>
#include <ns3/node.h>
#include <ns3/type-id.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CapillaryEnergyModelHelper");


CapillaryEnergyModelHelper::CapillaryEnergyModelHelper ()
{
  NS_LOG_FUNCTION (this);
  m_energyModel.SetTypeId ("ns3::CapillaryEnergyModel");
}

CapillaryEnergyModelHelper::~CapillaryEnergyModelHelper ()
{
  NS_LOG_FUNCTION (this);
}


void CapillaryEnergyModelHelper::Set (std::string name, const AttributeValue& v)
{
  NS_LOG_FUNCTION (this);
  m_energyModel.Set (name, v);
}

Ptr<DeviceEnergyModel> CapillaryEnergyModelHelper::DoInstall (Ptr<NetDevice> device,
                                                              Ptr<EnergySource> source) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (device);
  NS_ASSERT (source);

  // check if device is WifiNetDevice
  std::string deviceName = device->GetInstanceTypeId ().GetName ();
  if (deviceName.compare ("ns3::CapillaryNetDevice") != 0)
    {
      NS_FATAL_ERROR ("NetDevice type is not CapillaryNetDevice!");
    }

  Ptr<CapillaryEnergyModel> model = m_energyModel.Create ()->GetObject<CapillaryEnergyModel> ();
  NS_ASSERT (model);

  // set energy source pointer
  model->SetEnergySource (source);
  // add model to device model list in energy source
  source->AppendDeviceEnergyModel (model);

  Ptr<Node> node = device->GetNode ();
  NS_ASSERT (node);

  model->SetNode (node);

  Ptr<CapillaryNetDevice> dev = DynamicCast<CapillaryNetDevice> (device);
  NS_ASSERT (dev);

  dev->GetPhy ()->SetEnergyModelCallback (MakeCallback (&CapillaryEnergyModel::ChangeState, model));

  model->SetAttribute ("DepletionCallback", CallbackValue (MakeCallback (&CapillaryNetDevice::LinkDown, dev)));
  model->SetAttribute ("RechargedCallback", CallbackValue (MakeCallback (&CapillaryNetDevice::LinkUp, dev)));

  return model;
}

} /* namespace ns3 */
