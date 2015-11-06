/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */


#include "bounded-energy-source-helper.h"

#include "ns3/energy-source.h"

namespace ns3 {

BoundedEnergySourceHelper::BoundedEnergySourceHelper ()
{
  m_boundedEnergySource.SetTypeId ("ns3::BoundedEnergySource");
}

BoundedEnergySourceHelper::~BoundedEnergySourceHelper ()
{
}

void
BoundedEnergySourceHelper::Set (std::string name, const AttributeValue &v)
{
  m_boundedEnergySource.Set (name, v);
}

Ptr<EnergySource>
BoundedEnergySourceHelper::DoInstall (Ptr<Node> node) const
{
  NS_ASSERT (node != NULL);
  Ptr<EnergySource> source = m_boundedEnergySource.Create<EnergySource> ();
  NS_ASSERT (source != NULL);
  source->SetNode (node);
  return source;
}

} // namespace ns3
