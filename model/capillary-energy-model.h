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
#ifndef CAPILLARY_ENERGY_MODEL_H_
#define CAPILLARY_ENERGY_MODEL_H_

#include <ns3/callback.h>
#include <ns3/device-energy-model.h>
#include <ns3/energy-source.h>
#include <ns3/capillary-phy.h>
#include <ns3/node.h>
#include <ns3/nstime.h>
#include <ns3/ptr.h>
#include <ns3/traced-value.h>

namespace ns3 {

/*
 *
 */
class CapillaryEnergyModel : public DeviceEnergyModel
{
public:
  /**
   * Callback type for energy depletion handling.
   */
  typedef Callback<void> CapillaryEnergyDepletionCallback;

  /**
   * Callback type for energy recharged handling.
   */
  typedef Callback<void> CapillaryEnergyRechargedCallback;

  CapillaryEnergyModel ();
  virtual ~CapillaryEnergyModel ();

  static TypeId GetTypeId (void);

  /**
   * \param source Pointer to energy source installed on node.
   *
   * This function sets the pointer to energy source installed on node. Should
   * be called only by DeviceEnergyModel helper classes.
   */
  virtual void SetEnergySource (Ptr<EnergySource> source);

  /**
   * \returns Total energy consumption of the device.
   *
   * DeviceEnergyModel records its own energy consumption during simulation.
   */
  virtual double GetTotalEnergyConsumption (void) const;

  /**
   * \param newState New state the device is in.
   *
   * DeviceEnergyModel is a state based model. This function is implemented by
   * all child of DeviceEnergyModel to change the model's state. States are to
   * be defined by each child using an enum (int).
   */
  virtual void ChangeState (int newState);

  /**
   * This function is called by the EnergySource object when energy stored in
   * the energy source is depleted. Should be implemented by child classes.
   */
  virtual void HandleEnergyDepletion (void);

  /**
   * This function is called by the EnergySource object when energy stored in
   * the energy source is recharged. Should be implemented by child classes.
   */
  virtual void HandleEnergyRecharged (void);

  /** Getters and Setters*/
  double GetIdleCurrentA () const;
  void SetIdleCurrentA (double idleCurrentA);
  double GetCcaBusyCurrentA () const;
  void SetCcaBusyCurrentA (double ccaBusyCurrentA);
  double GetRxCurrentA () const;
  void SetRxCurrentA (double rxCurrentA);
  double GetSleepCurrentA () const;
  void SetSleepCurrentA (double sleepCurrentA);
  double GetSwitchingCurrentA () const;
  void SetSwitchingCurrentA (double switchingCurrentA);
  double GetTxCurrentA () const;
  void SetTxCurrentA (double txCurrentA);
  const Ptr<Node>& GetNode () const;
  void SetNode (Ptr<Node> node);
  CapillaryPhy::State GetCurrentState () const;

private:
  void DoDispose (void);

  /**
   * \returns 0.0 as the current value, in Ampere.
   *
   * Child class does not have to implement this method if current draw for its
   * states are not know. This default method will always return 0.0A. For the
   * devices who does know the current draw of its states, this method must be
   * overwritten.
   */
  virtual double DoGetCurrentA (void) const;

  Ptr<Node> m_node;            //!< The node.
  Ptr<EnergySource> m_source;  //!< The energy source.

  double m_txCurrentA;
  double m_rxCurrentA;
  double m_idleCurrentA;
  double m_ccaBusyCurrentA;
  double m_switchingCurrentA;
  double m_sleepCurrentA;

  /** The total energy consumed by this model. */
  TracedValue<double> m_totalEnergyConsumption;


  Time m_lastUpdateTime;       //!< Time stamp of previous energy update.
  // State variables.
  CapillaryPhy::State m_currentState;              // current state the radio is in

  // Energy depletion callback
  CapillaryEnergyDepletionCallback m_energyDepletionCallback;

  // Energy recharged callback
  CapillaryEnergyRechargedCallback m_energyRechargedCallback;

};

} /* namespace ns3 */

#endif /* CAPILLARY_ENERGY_MODEL_H_ */
