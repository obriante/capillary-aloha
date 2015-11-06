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
#ifndef MODEL_CAPILLARY_PHY_IDEAL_H_
#define MODEL_CAPILLARY_PHY_IDEAL_H_

#include <ns3/antenna-model.h>
#include <ns3/capillary-phy.h>
#include <ns3/data-rate.h>
#include <ns3/event-id.h>
#include <ns3/mobility-model.h>
#include <ns3/net-device.h>
#include <ns3/packet.h>
#include <ns3/ptr.h>
#include <ns3/spectrum-channel.h>
#include <ns3/spectrum-interference.h>
#include <ns3/spectrum-model.h>
#include <ns3/spectrum-signal-parameters.h>
#include <ns3/spectrum-value.h>

namespace ns3 {

/*
 *
 */
class CapillaryPhyIdeal : public CapillaryPhy
{
public:
  CapillaryPhyIdeal (void);
  virtual ~CapillaryPhyIdeal (void);

  static TypeId GetTypeId (void);

  virtual void SetDevice (Ptr<NetDevice> d);
  virtual Ptr<NetDevice> GetDevice (void) const;
  virtual void SetMobility (Ptr<MobilityModel> m);
  virtual Ptr<MobilityModel> GetMobility (void);
  virtual void SetChannel (Ptr<SpectrumChannel> c);
  virtual Ptr<const SpectrumModel> GetRxSpectrumModel (void) const;
  virtual Ptr<AntennaModel> GetRxAntenna (void);
  virtual void StartRx (Ptr<SpectrumSignalParameters> params);

  virtual void ForceSleep (void);
  virtual void WakeUp (void);

  virtual CapillaryPhy::State GetStatus (void) const;
  virtual void SetTxPowerSpectralDensity (Ptr<SpectrumValue> txPsd);
  virtual void SetNoisePowerSpectralDensity (Ptr<const SpectrumValue> noisePsd);
  virtual void SetAntenna (Ptr<AntennaModel> a);


  /**
   * Start a transmission
   *
   *
   * @param p the packet to be transmitted
   *
   * @return true if an error occurred and the transmission was not
   * started, false otherwise.
   */
  bool StartTx (Ptr<Packet> p);

  /**
   * set the PHY rate to be used by this PHY.
   *
   * @param rate
   */
  void SetRate (DataRate rate);

  /**
   *
   * @return the PHY rate used by this PHY.
   */
  virtual DataRate GetRate (void) const;

  /**
   *
   */
  virtual Time GetSwitchingTime (void) const;


private:
  virtual void DoDispose (void);

  Ptr<SpectrumSignalParameters> TransmissionSignalParameters ();

  void ChangeState (CapillaryPhy::State newState);
  void EndTx (void);
  void AbortRx (void);
  void EndRx (void);

  EventId m_endRxEventId;

  Ptr<MobilityModel> m_mobility;
  Ptr<AntennaModel> m_antenna;
  Ptr<NetDevice> m_netDevice;
  Ptr<SpectrumChannel> m_channel;

  Ptr<SpectrumValue> m_txPsd;
  Ptr<const SpectrumValue> m_rxPsd;
  Ptr<Packet> m_txPacket;
  Ptr<Packet> m_rxPacket;

  DataRate m_rate;

  Time m_switch;

  CapillaryPhy::State m_state;

  SpectrumInterference m_interference;
};



} /* namespace ns3 */

#endif /* MODEL_CAPILLARY_PHY_IDEAL_H_ */
