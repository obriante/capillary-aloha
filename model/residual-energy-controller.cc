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

#include "residual-energy-controller.h"

#include <ns3/assert.h>
#include <ns3/energy-controller.h>
#include <ns3/energy-source-container.h>
#include <ns3/capillary-net-device.h>
#include <ns3/log.h>
#include <ns3/log-macros-disabled.h>
#include <ns3/object-base.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/type-id.h>
#include <ns3/uinteger.h>
#include <iostream>
#include <iterator>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ResidualEnergyController");

NS_OBJECT_ENSURE_REGISTERED (ResidualEnergyController);

ResidualEnergyController::ResidualEnergyController ()
{
  NS_LOG_FUNCTION (this);

}

ResidualEnergyController::~ResidualEnergyController ()
{
  NS_LOG_FUNCTION (this);
}

TypeId ResidualEnergyController::GetTypeId (void)
{

  static TypeId tid = TypeId ("ns3::ResidualEnergyController")
    .SetParent<CapillaryController> ()
    .SetGroupName ("capillary-network")
    .AddConstructor<ResidualEnergyController> ()
    .AddAttribute ("MinTh",
                   "The Energy Store Device Threshold", DoubleValue (0.3),
                   MakeDoubleAccessor (&ResidualEnergyController::SetThreshold, &ResidualEnergyController::GetThreshold),
                   MakeDoubleChecker<double> (0,1))
    .AddAttribute ("MaxOffTime",
                   "The maximum off time", TimeValue (Seconds (60)),
                   MakeTimeAccessor (&ResidualEnergyController::m_maxToff),
                   MakeTimeChecker ())
    .AddTraceSource ("Toff",
                     "The OFF Time",
                     MakeTraceSourceAccessor (&ResidualEnergyController::m_Toff))
  ;
  return tid;

}


double ResidualEnergyController::GetThreshold () const
{
  NS_LOG_FUNCTION (this);
  return m_threshold;
}

void ResidualEnergyController::SetThreshold (double threshold)
{
  NS_LOG_FUNCTION (this << threshold);
  m_threshold = threshold;
}

void ResidualEnergyController::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  NS_ASSERT (node);
  m_node = node;
}

void ResidualEnergyController::SetMac (Ptr<CapillaryMac> mac)
{
  NS_LOG_FUNCTION (this << mac);
  NS_ASSERT (mac);
  m_mac = mac;
}

Ptr<Node> ResidualEnergyController::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}

Ptr<CapillaryMac> ResidualEnergyController::GetMac (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mac;
}

Time ResidualEnergyController::GetOffTime (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<CapillaryNetDevice> dev = DynamicCast<CapillaryNetDevice> (m_mac->GetDevice ());

  if (dev->GetType () == CapillaryNetDevice::END_DEVICE)
    {
      Ptr<EnergySourceContainer> EnergySourceContainerOnNode = m_node->GetObject<EnergySourceContainer> ();

      double energyFraction = 0;

      if (EnergySourceContainerOnNode)
        {
          for (EnergySourceContainer::Iterator i = EnergySourceContainerOnNode->Begin (); i != EnergySourceContainerOnNode->End (); ++i)
            {
              if ((*i)->GetEnergyFraction ()  > ( energyFraction))
                {
                  energyFraction = (*i)->GetEnergyFraction ();
                }
            }
        }


      if (energyFraction <= m_threshold)
        {
          m_Toff = m_maxToff;
        }
      else
        {
          double value = ((energyFraction) - m_threshold) / (1 - m_threshold);
          NS_LOG_DEBUG ("Value= " << value);
          m_Toff = Time (m_maxToff.GetDouble () - value * m_maxToff.GetDouble ());
        }

      NS_LOG_DEBUG ("Node (" << m_node->GetId () << "): Energy Fraction: " << energyFraction << " %, TimeOff: " << m_Toff.Get ().GetSeconds () << "[sec]");

      return m_Toff;
    }

  return Time (Seconds (0));
}

void ResidualEnergyController::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);


}

void ResidualEnergyController::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_node = 0;
  m_mac = 0;
}


} /* namespace ns3 */

