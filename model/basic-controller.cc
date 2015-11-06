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

#include "basic-controller.h"

#include <ns3/log.h>
#include <ns3/log-macros-disabled.h>
#include <ns3/object-base.h>
#include <ns3/type-id.h>

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("BasicController");

NS_OBJECT_ENSURE_REGISTERED (BasicController);

BasicController::BasicController ()
{
  NS_LOG_FUNCTION (this);
}

BasicController::~BasicController ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
BasicController::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BasicController")
    .SetParent<CapillaryController> ()
    .SetGroupName ("m2m-capillary")
    .AddConstructor<BasicController> ()
  ;
  return tid;
}

void BasicController::SetMac (Ptr<CapillaryMac> mac)
{
  NS_LOG_FUNCTION (this << mac);
  m_mac = mac;
}

Ptr<CapillaryMac> BasicController::GetMac (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mac;
}

void BasicController::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}

Ptr<Node> BasicController::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}

Time
BasicController::GetOffTime (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG (this);
  return Time (Seconds (0));
}

void BasicController::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);


}

void BasicController::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_node = 0;
  m_mac = 0;
}

} /* namespace ns3 */
