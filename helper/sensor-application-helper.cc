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

#include "sensor-application-helper.h"

#include <ns3/assert.h>
#include <ns3/log.h>
#include <ns3/log-macros-disabled.h>
#include <ns3/names.h>
#include <ns3/sensor-application.h>
#include <ns3/application-container.h>
#include <iterator>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SensorApplicationHelper");

SensorApplicationHelper::SensorApplicationHelper ()
{
  NS_LOG_FUNCTION (this);

  // set types
  m_factory.SetTypeId ("ns3::SensorApplication");

}

SensorApplicationHelper::~SensorApplicationHelper ()
{
  NS_LOG_FUNCTION (this);
}

void SensorApplicationHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  NS_LOG_FUNCTION (this);
  m_factory.Set (name, value);
}

ApplicationContainer
SensorApplicationHelper::Install (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this);
  return InstallPriv (node);
}

ApplicationContainer
SensorApplicationHelper::Install (std::string nodeName) const
{
  NS_LOG_FUNCTION (this);
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return InstallPriv (node);
}

ApplicationContainer
SensorApplicationHelper::Install (NodeContainer c) const
{
  NS_LOG_FUNCTION (this);
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

ApplicationContainer
SensorApplicationHelper::InstallPriv (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this);
  ApplicationContainer apps;
  for (uint32_t i = 0; i < node->GetNDevices (); i++)
    {
      Ptr<CapillaryNetDevice> dev = DynamicCast<CapillaryNetDevice> (node->GetDevice (i));

      if (dev)
        {
          Ptr<Application> app = InstallPriv (dev);
          node->AddApplication (app);
          apps.Add (app);
        }
    }

  return apps;
}

Ptr<Application>
SensorApplicationHelper::InstallPriv (Ptr<CapillaryNetDevice> dev) const
{
  NS_LOG_FUNCTION (this);
  Ptr<SensorApplication> app = m_factory.Create<SensorApplication> ();
  NS_ASSERT (app);
  app->SetDev (dev);

  return app;
}

} /* namespace ns3 */
