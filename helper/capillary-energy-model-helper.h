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
#ifndef HELPER_CAPILLARY_ENERGY_MODEL_HELPER_H_
#define HELPER_CAPILLARY_ENERGY_MODEL_HELPER_H_

#include <ns3/device-energy-model.h>
#include <ns3/energy-model-helper.h>
#include <ns3/energy-source.h>
#include <ns3/net-device.h>
#include <ns3/object-factory.h>
#include <ns3/ptr.h>
#include <string>

namespace ns3 {

/*
 *
 */
class CapillaryEnergyModelHelper : public DeviceEnergyModelHelper
{
public:
  CapillaryEnergyModelHelper ();
  virtual ~CapillaryEnergyModelHelper ();

  virtual void Set (std::string name, const AttributeValue &v);


private:
  virtual Ptr<DeviceEnergyModel> DoInstall (Ptr<NetDevice> device,
                                            Ptr<EnergySource> source) const;

  ObjectFactory m_energyModel;
};

} /* namespace ns3 */

#endif /* HELPER_CAPILLARY_ENERGY_MODEL_HELPER_H_ */
