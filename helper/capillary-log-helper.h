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
#ifndef HELPER_CAPILLARY_LOG_HELPER_H_
#define HELPER_CAPILLARY_LOG_HELPER_H_

#include <ns3/capillary-net-device.h>
#include <ns3/capillary-energy-model.h>
#include <ns3/capillary-tracer.h>
#include <ns3/output-stream-wrapper.h>
#include <ns3/ptr.h>
#include <ns3/trace-helper.h>
#include <string>
#include <ns3/log.h>

namespace ns3 {

class SpectrumChannel;
class MobilityModel;

/*
 *
 */
class CapillaryLogHelper : public PcapHelperForDevice,
                           public AsciiTraceHelperForDevice,
                           public CapillaryTracer
{
public:
  CapillaryLogHelper ();
  virtual ~CapillaryLogHelper ();

  /**
   * Helper to enable all Capillary log components with one statement
   */
  void EnableLogComponents (enum LogLevel level);

  virtual void EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename);
  virtual void EnableAsciiInternal (Ptr<OutputStreamWrapper> stream, std::string prefix, Ptr<NetDevice> nd, bool explicitFilename);
  virtual void EnableAsciiInternal (Ptr<OutputStreamWrapper> stream, Ptr<CapillaryEnergyModel> nd);
  virtual void EnableAsciiInternal (Ptr<OutputStreamWrapper> stream, Ptr<CapillaryNetDevice> nd);
  virtual void EnableAsciiInternal (Ptr<OutputStreamWrapper> stream, Ptr<EnergySource> nd);


};

} /* namespace ns3 */

#endif /* HELPER_CAPILLARY_LOG_HELPER_H_ */
