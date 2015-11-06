/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef BOUNDED_ENERGY_SOURCE_HELPER_H
#define BOUNDED_ENERGY_SOURCE_HELPER_H

#include "ns3/energy-model-helper.h"
#include "ns3/node.h"

namespace ns3 {

/**
 * \ingroup energy
 * \brief Creates a BoundedEnergySource object.
 *
 */
class BoundedEnergySourceHelper : public EnergySourceHelper
{
public:
  BoundedEnergySourceHelper ();
  ~BoundedEnergySourceHelper ();

  void Set (std::string name, const AttributeValue &v);

private:
  virtual Ptr<EnergySource> DoInstall (Ptr<Node> node) const;

private:
  ObjectFactory m_boundedEnergySource;

};

} // namespace ns3

#endif  /* BOUNDED_ENERGY_SOURCE_HELPER_H */
