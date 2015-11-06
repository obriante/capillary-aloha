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

#include "energy-controller.h"

#include <ns3/assert.h>
#include <ns3/device-energy-model-container.h>
#include <ns3/double.h>
#include <ns3/energy-harvester-container.h>
#include <ns3/energy-source-container.h>
#include <ns3/log.h>
#include <ns3/log-macros-disabled.h>
#include <ns3/nstime.h>
#include <ns3/object-base.h>
#include <ns3/simulator.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/traced-value.h>
#include <ns3/type-id.h>
#include <iostream>
#include <iterator>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EnergyController");

NS_OBJECT_ENSURE_REGISTERED (EnergyController);

EnergyController::EnergyController ()
{
  NS_LOG_FUNCTION (this);
  m_lastConsumption=0;
  m_lastHarvested=0;
  m_lastUpdate = Simulator::Now ();
}

EnergyController::~EnergyController ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
EnergyController::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EnergyController")
    .SetParent<CapillaryController> ()
    .SetGroupName ("capillary-network")
    .AddConstructor<EnergyController> ()
    .AddAttribute ("MinTh",
                   "The Energy Store Device Threshold", DoubleValue (0.3),
                   MakeDoubleAccessor (&EnergyController::SetMinThreshold, &EnergyController::GetMinThreshold),
                   MakeDoubleChecker<double> (0,1))
    .AddAttribute ("MaxTh",
                   "The Energy Store Device Threshold", DoubleValue (0.5),
                   MakeDoubleAccessor (&EnergyController::SetMaxThreshold, &EnergyController::GetMaxThreshold),
                   MakeDoubleChecker<double> (0,1))
    .AddAttribute ("MaxOffTime",
                   "The maximum off time", TimeValue (Seconds (60)),
                   MakeTimeAccessor (&EnergyController::m_maxToff),
                   MakeTimeChecker ())
    .AddTraceSource ("Toff",
                     "The OFF Time",
                     MakeTraceSourceAccessor (&EnergyController::m_Toff))
  ;
  return tid;
}

double EnergyController::GetMaxThreshold () const
{
  NS_LOG_FUNCTION (this);
  return m_maxThreshold;
}

void EnergyController::SetMaxThreshold (double maxThreshold)
{
  NS_LOG_FUNCTION (this << maxThreshold);
  m_maxThreshold = maxThreshold;
}

double EnergyController::GetMinThreshold () const
{
  NS_LOG_FUNCTION (this);
  return m_minThreshold;
}

void EnergyController::SetMinThreshold (double minThreshold)
{
  NS_LOG_FUNCTION (this << minThreshold);
  m_minThreshold = minThreshold;
}

Ptr<Node> EnergyController::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}

void EnergyController::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (node);
  m_node = node;
}

void EnergyController::SetMac (Ptr<CapillaryMac> mac)
{
  NS_LOG_FUNCTION (this << mac);
  NS_ASSERT (mac);
  m_mac = mac;
}

Ptr<CapillaryMac> EnergyController::GetMac (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mac;
}

Time
EnergyController::GetOffTime (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<EnergySourceContainer> EnergySourceContainerOnNode = m_node->GetObject<EnergySourceContainer> ();

  double energyFraction = 0;

  DeviceEnergyModelContainer capillaryEnergyModelContainer;

  if (EnergySourceContainerOnNode)
    {
      for (EnergySourceContainer::Iterator i = EnergySourceContainerOnNode->Begin (); i != EnergySourceContainerOnNode->End (); ++i)
        {
          if ((*i)->GetEnergyFraction ()  > ( energyFraction))
            {
              energyFraction = (*i)->GetEnergyFraction ();
              capillaryEnergyModelContainer.Add ((*i)->FindDeviceEnergyModels ("ns3::CapillaryEnergyModel"));
            }
        }

      NS_LOG_DEBUG ("Node (" << m_node->GetId () << "): Energy Fraction: " << energyFraction);
    }

  m_lastConsumption=0; // I need to reset this value!!

  for (DeviceEnergyModelContainer::Iterator i = capillaryEnergyModelContainer.Begin (); i != capillaryEnergyModelContainer.End (); ++i)
    {
	  m_lastConsumption += (*i)->GetTotalEnergyConsumption ();
    }

  NS_LOG_DEBUG ("Node (" << m_node->GetId () << "): Energy Consumed: " << m_lastConsumption << " [J]");

  /*  double powerConsumption = 0;

  powerConsumption = (totalEnergyConsumption-m_lastConsumption) / (Simulator::Now () - m_lastUpdate).GetSeconds ();

  NS_LOG_DEBUG ("Node (" << m_node->GetId () << "): Power Consumption: " << powerConsumption << " [W]");*/

  Ptr<EnergyHarvesterContainer> EnergyHarvesterContainerOnNode = m_node->GetObject<EnergyHarvesterContainer> ();


  double harvestingPower=0;

  if (EnergyHarvesterContainerOnNode)
    {
      for (EnergyHarvesterContainer::Iterator i = EnergyHarvesterContainerOnNode->Begin (); i != EnergyHarvesterContainerOnNode->End (); ++i)
        {
    	  harvestingPower+=(*i)->GetPower ();

        }
      NS_LOG_DEBUG ("Node (" << m_node->GetId () << "): Energy Harvested: " << m_lastHarvested << " [J]");
    }

  m_lastHarvested += harvestingPower * (Simulator::Now().GetSeconds() - m_lastUpdate.GetSeconds());

  if (energyFraction >= m_maxThreshold)
    {
      m_Toff = Seconds (0);
    }
  else if (energyFraction <= m_minThreshold)
    {
      m_Toff = m_maxToff;
    }
  else
    {

    double value=((m_lastConsumption/Simulator::Now().GetSeconds())/harvestingPower);

    Time Toff = Time ( value * m_maxToff.GetDouble ());;

    if(Toff>m_maxToff)
    {
    	m_Toff=m_maxToff;
    }
    else if(Toff<0)
    {
    	m_Toff=0;
    }
    else
    {
    	m_Toff=Toff;
    }


    }

  NS_LOG_DEBUG ("Node (" << m_node->GetId () << "): TimeOff: " << m_Toff.Get ().GetSeconds () << "[sec]");

  m_lastUpdate = Simulator::Now ();

  return m_Toff.Get();
}

void EnergyController::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_lastUpdate = Seconds (0);

}

void EnergyController::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_node = 0;
  m_mac = 0;
}

} /* namespace ns3 */
