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

#include "capillary-energy-model.h"

#include <ns3/energy-source.h>

#include <ns3/assert.h>
#include <ns3/callback.h>
#include <ns3/double.h>
#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <ns3/log-macros-disabled.h>
#include <ns3/node.h>
#include <ns3/nstime.h>
#include <ns3/object-base.h>
#include <ns3/ptr.h>
#include <ns3/simulator.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/type-id.h>
#include <stddef.h>
#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CapillaryEnergyModel");

NS_OBJECT_ENSURE_REGISTERED (CapillaryEnergyModel);

CapillaryEnergyModel::CapillaryEnergyModel ()
{
  NS_LOG_FUNCTION (this);
  m_currentState = CapillaryPhy::IDLE;              // initially IDLE
  m_lastUpdateTime = Seconds (0.0);
  m_energyDepletionCallback.Nullify ();
  m_energyRechargedCallback.Nullify ();
  m_source = 0;
  m_node = 0;
}

CapillaryEnergyModel::~CapillaryEnergyModel ()
{
  NS_LOG_FUNCTION (this);
}

TypeId CapillaryEnergyModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CapillaryEnergyModel")
    .SetParent<DeviceEnergyModel> ()
    .SetGroupName ("Capillary")
    .AddConstructor<CapillaryEnergyModel> ()
    .AddAttribute ("IdleCurrentA",
                   "The default radio Idle current in Ampere.", DoubleValue (273e-3), // idle mode = 273mA
                   MakeDoubleAccessor (&CapillaryEnergyModel::SetIdleCurrentA,
                                       &CapillaryEnergyModel::GetIdleCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("CcaBusyCurrentA",
                   "The default radio CCA Busy State current in Ampere.", DoubleValue (0.273), // default to be the same as idle mode
                   MakeDoubleAccessor (&CapillaryEnergyModel::SetCcaBusyCurrentA,
                                       &CapillaryEnergyModel::GetCcaBusyCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxCurrentA",
                   "The radio Tx current in Ampere.", DoubleValue (380e-3), // transmit at 0dBm = 380mA
                   MakeDoubleAccessor (&CapillaryEnergyModel::SetTxCurrentA,
                                       &CapillaryEnergyModel::GetTxCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxCurrentA",
                   "The radio Rx current in Ampere.", DoubleValue (313e-3), // receive mode = 313mA
                   MakeDoubleAccessor (&CapillaryEnergyModel::SetRxCurrentA,
                                       &CapillaryEnergyModel::GetRxCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SwitchingCurrentA",
                   "The default radio Channel Switch current in Ampere.", DoubleValue (273e-3), // default to be the same as idle mode
                   MakeDoubleAccessor (&CapillaryEnergyModel::SetSwitchingCurrentA,
                                       &CapillaryEnergyModel::GetSwitchingCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SleepCurrentA",
                   "The radio Sleep current in Ampere.", DoubleValue (33e-3), // sleep mode = 33mA
                   MakeDoubleAccessor (&CapillaryEnergyModel::SetSleepCurrentA,
                                       &CapillaryEnergyModel::GetSleepCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DepletionCallback",
                   "A Callback to call on source depletion.",
                   CallbackValue (),
                   MakeCallbackAccessor (&CapillaryEnergyModel::m_energyDepletionCallback),
                   MakeCallbackChecker ())
    .AddAttribute ("RechargedCallback",
                   "A Callback to call when source is recharged.",
                   CallbackValue (),
                   MakeCallbackAccessor (&CapillaryEnergyModel::m_energyRechargedCallback),
                   MakeCallbackChecker ())
    .AddTraceSource ("TotalEnergyConsumption",
                     "Total energy consumption of the radio device.",
                     MakeTraceSourceAccessor (
                       &CapillaryEnergyModel::m_totalEnergyConsumption),
                     "ns3::TracedValue::DoubleCallback");
  return tid;
}

void CapillaryEnergyModel::SetEnergySource (Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION (this << source);
  NS_ASSERT (source);
  m_source = source;
}

double CapillaryEnergyModel::GetTotalEnergyConsumption (void) const
{
  NS_LOG_FUNCTION (this);
  return m_totalEnergyConsumption;
}

void CapillaryEnergyModel::ChangeState (int newState)
{
  NS_LOG_FUNCTION (this << newState);

  NS_LOG_DEBUG ("CapillaryEnergyModel (" << m_node->GetId () << "): " << m_currentState << "->" << (CapillaryPhy::State) newState);

  Time duration = Simulator::Now () - m_lastUpdateTime;
  NS_ASSERT (duration.GetNanoSeconds () >= 0);         // check if duration is valid

  // energy to decrease = current * voltage * time
  double energyToDecrease = 0.0;
  double supplyVoltage = m_source->GetSupplyVoltage ();
  switch (m_currentState)
    {
    case CapillaryPhy::IDLE:
      energyToDecrease = duration.GetSeconds () * m_idleCurrentA * supplyVoltage;
      break;
    case CapillaryPhy::CCA_BUSY:
      energyToDecrease = duration.GetSeconds () * m_ccaBusyCurrentA * supplyVoltage;
      break;
    case CapillaryPhy::TX:
      energyToDecrease = duration.GetSeconds () * m_txCurrentA * supplyVoltage;
      break;
    case CapillaryPhy::RX:
      energyToDecrease = duration.GetSeconds () * m_rxCurrentA * supplyVoltage;
      break;
    case CapillaryPhy::SWITCHING:
      energyToDecrease = duration.GetSeconds () * m_switchingCurrentA * supplyVoltage;
      break;
    case CapillaryPhy::SLEEP:
      energyToDecrease = duration.GetSeconds () * m_sleepCurrentA * supplyVoltage;
      break;
    default:
      NS_FATAL_ERROR ("CapillaryEnergyModel:Undefined radio state: " << m_currentState);
    }

  // update total energy consumption
  m_totalEnergyConsumption += energyToDecrease;

  // update last update time stamp
  m_lastUpdateTime = Simulator::Now ();

  // notify energy source
  m_source->UpdateEnergySource ();

  m_currentState = (CapillaryPhy::State)newState;

  NS_LOG_DEBUG ("CapillaryEnergyModel (" << m_node->GetId () << "): Total energy consumption: " << m_totalEnergyConsumption << " [J]");
}

void CapillaryEnergyModel::HandleEnergyDepletion (void)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("CapillaryEnergyModel (" << m_node->GetId () << "): Energy is depleted.");
  // invoke energy depletion callback, if set.
  if (!m_energyDepletionCallback.IsNull ())
    {
      m_energyDepletionCallback ();
    }
}

void CapillaryEnergyModel::HandleEnergyRecharged (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("CapillaryEnergyModel (" << m_node->GetId () << "): Source recharged.");  // invoke energy recharged callback, if set.
  if (!m_energyRechargedCallback.IsNull ())
    {
      m_energyRechargedCallback ();
    }
}

double CapillaryEnergyModel::DoGetCurrentA (void) const
{
  NS_LOG_FUNCTION (this);

  double stateCurrent = 0.0;

  NS_LOG_DEBUG ("CapillaryEnergyModel (" << m_node->GetId () << "): Current State: " << m_currentState);

  switch (m_currentState)
    {
    case CapillaryPhy::IDLE:
      stateCurrent = m_idleCurrentA;
      break;

    case CapillaryPhy::CCA_BUSY:
      stateCurrent = m_ccaBusyCurrentA;
      break;

    case CapillaryPhy::TX:
      stateCurrent = m_txCurrentA;
      break;

    case CapillaryPhy::RX:
      stateCurrent = m_rxCurrentA;
      break;

    case CapillaryPhy::SLEEP:
      stateCurrent = m_sleepCurrentA;
      break;

    case CapillaryPhy::SWITCHING:
      stateCurrent = m_switchingCurrentA;
      break;

    default:
      NS_FATAL_ERROR ("CapillaryEnergyModel (" << m_node->GetId () << "): Undefined radio state!");
    }

  return stateCurrent;
}

double CapillaryEnergyModel::GetIdleCurrentA () const
{
  NS_LOG_FUNCTION (this);
  return m_idleCurrentA;
}

void CapillaryEnergyModel::SetIdleCurrentA (double idleCurrentA)
{
  NS_LOG_FUNCTION (this << idleCurrentA);
  m_idleCurrentA = idleCurrentA;
}

double CapillaryEnergyModel::GetCcaBusyCurrentA () const
{
  NS_LOG_FUNCTION (this);
  return m_ccaBusyCurrentA;
}

void CapillaryEnergyModel::SetCcaBusyCurrentA (double ccaBusyCurrentA)
{
  NS_LOG_FUNCTION (this << ccaBusyCurrentA);
  m_ccaBusyCurrentA = ccaBusyCurrentA;
}

double CapillaryEnergyModel::GetRxCurrentA () const
{
  NS_LOG_FUNCTION (this);
  return m_rxCurrentA;
}

void CapillaryEnergyModel::SetRxCurrentA (double rxCurrentA)
{
  NS_LOG_FUNCTION (this << rxCurrentA);
  m_rxCurrentA = rxCurrentA;
}

double CapillaryEnergyModel::GetSleepCurrentA () const
{
  NS_LOG_FUNCTION (this);
  return m_sleepCurrentA;
}

void CapillaryEnergyModel::SetSleepCurrentA (double sleepCurrentA)
{
  NS_LOG_FUNCTION (this << sleepCurrentA);
  m_sleepCurrentA = sleepCurrentA;
}

double CapillaryEnergyModel::GetSwitchingCurrentA () const
{
  NS_LOG_FUNCTION (this);
  return m_switchingCurrentA;
}

void CapillaryEnergyModel::SetSwitchingCurrentA (double switchingCurrentA)
{
  NS_LOG_FUNCTION (this << switchingCurrentA);
  m_switchingCurrentA = switchingCurrentA;
}

double CapillaryEnergyModel::GetTxCurrentA () const
{
  NS_LOG_FUNCTION (this);
  return m_txCurrentA;
}

void CapillaryEnergyModel::SetTxCurrentA (double txCurrentA)
{
  NS_LOG_FUNCTION (this << txCurrentA);
  m_txCurrentA = txCurrentA;
}

const Ptr<Node>&
CapillaryEnergyModel::GetNode () const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}

void CapillaryEnergyModel::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}

CapillaryPhy::State CapillaryEnergyModel::GetCurrentState () const
{
  NS_LOG_FUNCTION (this);
  return m_currentState;
}

void CapillaryEnergyModel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_source = 0;
  m_node = 0;
  m_energyDepletionCallback.Nullify ();
  m_energyRechargedCallback.Nullify ();
}

} /* namespace ns3 */
