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
#ifndef MODEL_CAPILLARY_PHY_H_
#define MODEL_CAPILLARY_PHY_H_

#include <ns3/antenna-model.h>
#include <ns3/spectrum-phy.h>
#include <ns3/device-energy-model.h>
#include <ns3/traced-callback.h>

// rxSetupTime1792

namespace ns3 {
class DataRate;
class Packet;
class Time;
class SpectrumChannel;
class MobilityModel;
class AntennaModel;
class SpectrumValue;
class SpectrumModel;
class NetDevice;
struct SpectrumSignalParameters;


/**
 * This method allows the MAC to instruct the PHY to start a
 * transmission of a given packet
 *
 * @param packet the Packet to be transmitted
 * @return this method returns false if the PHY will start TX,
 * true if the PHY refuses to start the TX. If false, the MAC layer
 * will expect that PhyTxEndCallback is invoked at some point later.
 */
typedef Callback< bool, Ptr<Packet> > PhyTxStartCallback;

/**
 * this method is invoked by the PHY to notify the MAC that the
 * transmission of a given packet has been completed.
 *
 * @param packet the Packet whose TX has been completed.
 */
typedef Callback< void, Ptr<const Packet> > PhyTxEndCallback;

/**
 * This method is used by the PHY to notify the MAC that a RX
 * attempt is being started, i.e., a valid signal has been
 * recognized by the PHY.
 *
 */
typedef Callback< void > PhyRxStartCallback;

/**
 * This method is used by the PHY to notify the MAC that a
 * previously started RX attempt has terminated without success.
 */
typedef Callback< void > PhyRxEndErrorCallback;

/**
 * This method is used by the PHY to notify the MAC that a
 * previously started RX attempt has been successfully completed.
 *
 * @param packet the received Packet
 */
typedef Callback< void, Ptr<Packet> > PhyRxEndOkCallback;

/*
 *
 */
class CapillaryPhy : public SpectrumPhy
{
public:
  CapillaryPhy (void);
  virtual ~CapillaryPhy (void);

  /**
   * The state of the PHY layer.
   */
  typedef enum
  {
    /**
     * The PHY layer is IDLE.
     */
    IDLE = 0,
    /**
     * The PHY layer has sense the medium busy through the CCA mechanism
     */
    CCA_BUSY,
    /**
     * The PHY layer is sending a packet.
     */
    TX,
    /**
     * The PHY layer is receiving a packet.
     */
    RX,
    /**
     * The PHY layer is sleeping.
     */
    SLEEP,
    /**
     * The PHY layer is switching.
     */
    SWITCHING
  } State;

  static TypeId GetTypeId (void);

  /**
   * set the associated NetDevice instance
   *
   * @param d the NetDevice instance
   */
  virtual void SetDevice (Ptr<NetDevice> d) = 0;

  /**
   * get the associated NetDevice instance
   *
   * @return a Ptr to the associated NetDevice instance
   */
  virtual Ptr<NetDevice> GetDevice (void) const = 0;

  /**
   * Set the mobility model associated with this device.
   *
   * @param m the mobility model
   */
  virtual void SetMobility (Ptr<MobilityModel> m) = 0;

  /**
   * get the associated MobilityModel instance
   *
   * @return a Ptr to the associated MobilityModel instance
   */
  virtual Ptr<MobilityModel> GetMobility (void) = 0;

  /**
   * Set the channel attached to this device.
   *
   * @param c the channel
   */
  virtual void SetChannel (Ptr<SpectrumChannel> c) = 0;

  /**
   *
   * @return returns the SpectrumModel that this SpectrumPhy expects to be used
   * for all SpectrumValues that are passed to StartRx. If 0 is
   * returned, it means that any model will be accepted.
   */
  virtual Ptr<const SpectrumModel> GetRxSpectrumModel (void) const = 0;

  /**
   * get the AntennaModel used by the NetDevice for reception
   *
   * @return a Ptr to the AntennaModel used by the NetDevice for reception
   */
  virtual Ptr<AntennaModel> GetRxAntenna (void) = 0;

  /**
   * Notify the SpectrumPhy instance of an incoming signal
   *
   * @param params the parameters of the signals being received
   */
  virtual void StartRx (Ptr<SpectrumSignalParameters> params) = 0;

  /**
   * Start a transmission
   *
   *
   * @param p the packet to be transmitted
   *
   * @return true if an error occurred and the transmission was not
   * started, false otherwise.
   */
  virtual bool StartTx (Ptr<Packet> p) = 0;

  /**
   * set the AntennaModel to be used
   *
   * \param a the Antenna Model
   */
  virtual void SetAntenna (Ptr<AntennaModel> a) = 0;

  virtual void ForceSleep (void) = 0;
  virtual void WakeUp (void) = 0;

  virtual CapillaryPhy::State GetStatus (void) const = 0;

  /**
   *
   * @return the PHY rate used by this PHY.
   */
  virtual DataRate GetRate (void) const = 0;

  virtual Time GetSwitchingTime (void) const = 0;

  virtual void SetTxPowerSpectralDensity (Ptr<SpectrumValue> txPsd) = 0;
  virtual void SetNoisePowerSpectralDensity (Ptr<const SpectrumValue> noisePsd) = 0;

  void SetEnergyModelCallback (DeviceEnergyModel::ChangeStateCallback cb);

protected:
  virtual void DoInitialize (void);
  virtual void DoDispose (void);

  virtual Ptr<SpectrumSignalParameters> TransmissionSignalParameters () = 0;

  /** Callback*/
  PhyTxEndCallback        m_phyTxEndCallback;
  PhyRxStartCallback      m_phyRxStartCallback;
  PhyRxEndErrorCallback   m_phyRxEndErrorCallback;
  PhyRxEndOkCallback      m_phyRxEndOkCallback;

  /** Energy model callback. */
  DeviceEnergyModel::ChangeStateCallback m_energyCallback;

  TracedCallback<Ptr<const Packet> > m_phyTxStartTrace;
  TracedCallback<Ptr<const Packet> > m_phyTxEndTrace;
  TracedCallback<Ptr<const Packet> > m_phyRxStartTrace;
  TracedCallback<Ptr<const Packet> > m_phyRxAbortTrace;
  TracedCallback<Ptr<const Packet> > m_phyRxEndOkTrace;
  TracedCallback<Ptr<const Packet> > m_phyRxEndErrorTrace;

private:
  CapillaryPhy (CapillaryPhy const &);
  CapillaryPhy& operator= (CapillaryPhy const &);

};

/**
 * \param os          output stream
 * \param state       fsaloha state to stringify
 * \return output stream
 */
std::ostream& operator<< (std::ostream& os, CapillaryPhy::State state);

} /* namespace ns3 */

#endif /* MODEL_CAPILLARY_PHY_H_ */
