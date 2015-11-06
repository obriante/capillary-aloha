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
#ifndef MODEL_CAPILLARY_TRACER_H_
#define MODEL_CAPILLARY_TRACER_H_

#include <ns3/capillary-energy-model.h>
#include <ns3/capillary-mac.h>
#include <ns3/device-energy-model-container.h>
#include <ns3/energy-source.h>
#include <ns3/energy-source-container.h>
#include <ns3/net-device-container.h>
#include <ns3/output-stream-wrapper.h>
#include <ns3/ptr.h>
#include <string>

#include <ns3/capillary-net-device.h>

namespace ns3 {

/*
 *
 */
class CapillaryTracer
{
public:
  CapillaryTracer ();
  virtual ~CapillaryTracer ();

  void EnableDCRAscii (std::string fileName, NetDeviceContainer n);
  void EnableEnergyAscii (std::string fileName, DeviceEnergyModelContainer n);
  void EnableSourceAscii (std::string fileName, EnergySourceContainer n);

  void EnableDCRAscii (Ptr<OutputStreamWrapper> stream, NetDeviceContainer n);
  void EnableEnergyAscii (Ptr<OutputStreamWrapper> stream, DeviceEnergyModelContainer n);
  void EnableSourceAscii (Ptr<OutputStreamWrapper> stream, EnergySourceContainer n);

  virtual void EnableAsciiInternal (Ptr<OutputStreamWrapper> stream, Ptr<CapillaryNetDevice> nd) = 0;
  virtual void EnableAsciiInternal (Ptr<OutputStreamWrapper> stream, Ptr<CapillaryEnergyModel> nd) = 0;
  virtual void EnableAsciiInternal (Ptr<OutputStreamWrapper> stream, Ptr<EnergySource> nd) = 0;

protected:

  static void DefaultDataCollectionRoundSinkWithContext (Ptr<OutputStreamWrapper> stream, std::string context, CapillaryMac::DcrStatus previous, CapillaryMac::DcrStatus current);
  static void DefaultFramesSinkWithContext (Ptr<OutputStreamWrapper> stream, std::string context, int previous, int current);
  static void DefaultEnergyConsumptionSinkWithContext (Ptr<OutputStreamWrapper> stream, std::string context, double previous, double current);

  static void DefaultEnergySourceSinkWithContext (Ptr<OutputStreamWrapper> stream, std::string context, double previous, double current);
};

} /* namespace ns3 */

#endif /* MODEL_CAPILLARY_TRACER_H_ */
