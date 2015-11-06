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

#include "capillary-controller.h"

#include <ns3/capillary-mac.h>
#include <ns3/log.h>
#include <ns3/log-macros-disabled.h>
#include <ns3/node.h>
#include <ns3/object-base.h>
#include <ns3/pointer.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CapillaryController");

NS_OBJECT_ENSURE_REGISTERED (CapillaryController);

CapillaryController::CapillaryController ()
{
  NS_LOG_FUNCTION (this);
}

CapillaryController::~CapillaryController ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
CapillaryController::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CapillaryController")
    .SetParent<Object> ()
    .AddAttribute ("Node",
                   "The NS-3 Node.", PointerValue (),
                   MakePointerAccessor (&CapillaryController::SetNode, &CapillaryController::GetNode),
                   MakePointerChecker<Node> ())
    .AddAttribute ("Mac",
                   "The MAC layer attached to this device.", PointerValue (),
                   MakePointerAccessor (&CapillaryController::SetMac, &CapillaryController::GetMac),
                   MakePointerChecker<CapillaryMac> ())
  ;
  return tid;
}

} /* namespace ns3 */
