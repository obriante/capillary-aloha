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
#ifndef MODEL_CAPILLARY_CONTROLLER_H_
#define MODEL_CAPILLARY_CONTROLLER_H_

#include <ns3/object.h>
#include <ns3/nstime.h>

namespace ns3 {
class Node;
class CapillaryMac;
}

namespace ns3 {

/*
 *
 */
class CapillaryController : public Object
{
public:
  CapillaryController ();
  virtual ~CapillaryController ();

  static TypeId GetTypeId (void);

  virtual void SetNode (Ptr<Node> node) = 0;
  virtual void SetMac (Ptr<CapillaryMac> mac) = 0;
  virtual Ptr<Node> GetNode (void) const = 0;
  virtual Ptr<CapillaryMac> GetMac (void) const = 0;
  virtual Time GetOffTime (void) = 0;
};


} /* namespace ns3 */

#endif /* MODEL_CAPILLARY_CONTROLLER_H_ */
