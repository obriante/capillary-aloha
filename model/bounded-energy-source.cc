/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "bounded-energy-source.h"

#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/double.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BoundedEnergySource");

NS_OBJECT_ENSURE_REGISTERED (BoundedEnergySource);

TypeId
BoundedEnergySource::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BoundedEnergySource")
    .SetParent<EnergySource> ()
    .SetGroupName ("Energy")
    .AddConstructor<BoundedEnergySource> ()
    .AddAttribute ("BoundedEnergySourceInitialEnergyJ",
                   "Initial energy stored in basic energy source.",
                   DoubleValue (10),  // in Joules
                   MakeDoubleAccessor (&BoundedEnergySource::SetInitialEnergy,
                                       &BoundedEnergySource::GetInitialEnergy),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("BoundedEnergyStartingPercentage",
                   "The Initial energy percentage.",
                   DoubleValue (1),
                   MakeDoubleAccessor (&BoundedEnergySource::SetStartingPercentage,
                                       &BoundedEnergySource::GetStartingPercentage),
                   MakeDoubleChecker<double> (0,1))

    .AddAttribute ("BoundedEnergySupplyVoltageV",
                   "Initial supply voltage for basic energy source.",
                   DoubleValue (3.0), // in Volts
                   MakeDoubleAccessor (&BoundedEnergySource::SetSupplyVoltage,
                                       &BoundedEnergySource::GetSupplyVoltage),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("BoundedEnergyLowBatteryThreshold",
                   "Low battery threshold for basic energy source.",
                   DoubleValue (0.10), // as a fraction of the initial energy
                   MakeDoubleAccessor (&BoundedEnergySource::m_lowBatteryTh),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("BoundedEnergyHighBatteryThreshold",
                   "High battery threshold for basic energy source.",
                   DoubleValue (0.15), // as a fraction of the initial energy
                   MakeDoubleAccessor (&BoundedEnergySource::m_highBatteryTh),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("PeriodicEnergyUpdateInterval",
                   "Time between two consecutive periodic energy updates.",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&BoundedEnergySource::SetEnergyUpdateInterval,
                                     &BoundedEnergySource::GetEnergyUpdateInterval),
                   MakeTimeChecker ())
    .AddTraceSource ("RemainingEnergy",
                     "Remaining energy at BoundedEnergySource.",
                     MakeTraceSourceAccessor (&BoundedEnergySource::m_remainingEnergyJ),
                     "ns3::TracedValueCallback::Double")
  ;
  return tid;
}

BoundedEnergySource::BoundedEnergySource ()
{
  NS_LOG_FUNCTION (this);
  m_lastUpdateTime = Seconds (0.0);
  m_depleted = false;
}

BoundedEnergySource::~BoundedEnergySource ()
{
  NS_LOG_FUNCTION (this);
}

void
BoundedEnergySource::SetInitialEnergy (double initialEnergyJ)
{
  NS_LOG_FUNCTION (this << initialEnergyJ);
  NS_ASSERT (initialEnergyJ >= 0);
  m_initialEnergyJ = initialEnergyJ;
}

void
BoundedEnergySource::SetSupplyVoltage (double supplyVoltageV)
{
  NS_LOG_FUNCTION (this << supplyVoltageV);
  m_supplyVoltageV = supplyVoltageV;
}

void
BoundedEnergySource::SetEnergyUpdateInterval (Time interval)
{
  NS_LOG_FUNCTION (this << interval);
  m_energyUpdateInterval = interval;
}

Time
BoundedEnergySource::GetEnergyUpdateInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return m_energyUpdateInterval;
}

double
BoundedEnergySource::GetSupplyVoltage (void) const
{
  NS_LOG_FUNCTION (this);
  return m_supplyVoltageV;
}

double
BoundedEnergySource::GetInitialEnergy (void) const
{
  NS_LOG_FUNCTION (this);
  return m_initialEnergyJ;
}

double
BoundedEnergySource::GetRemainingEnergy (void)
{
  NS_LOG_FUNCTION (this);
  // update energy source to get the latest remaining energy.
  UpdateEnergySource ();
  return m_remainingEnergyJ;
}

double BoundedEnergySource::GetStartingPercentage () const
{
  NS_LOG_FUNCTION (this);
  return m_startingPercentage;
}

void BoundedEnergySource::SetStartingPercentage (double startingPercentage)
{
  NS_LOG_FUNCTION (this << startingPercentage);
  m_startingPercentage = startingPercentage;
  m_remainingEnergyJ = m_startingPercentage * m_initialEnergyJ;
}

double
BoundedEnergySource::GetEnergyFraction (void)
{
  NS_LOG_FUNCTION (this);
  // update energy source to get the latest remaining energy.
  UpdateEnergySource ();
  return m_remainingEnergyJ / m_initialEnergyJ;
}

void
BoundedEnergySource::UpdateEnergySource (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("BoundedEnergySource:Updating remaining energy.");

  // do not update if simulation has finished
  if (Simulator::IsFinished ())
    {
      return;
    }

  m_energyUpdateEvent.Cancel ();

  CalculateRemainingEnergy ();

  m_lastUpdateTime = Simulator::Now ();

  if (!m_depleted && m_remainingEnergyJ <= m_lowBatteryTh * m_initialEnergyJ)
    {
      m_depleted = true;
      HandleEnergyDrainedEvent ();
    }

  if (m_depleted && m_remainingEnergyJ > m_highBatteryTh * m_initialEnergyJ)
    {
      m_depleted = false;
      HandleEnergyRechargedEvent ();
    }

  m_energyUpdateEvent = Simulator::Schedule (m_energyUpdateInterval,
                                             &BoundedEnergySource::UpdateEnergySource,
                                             this);
}

/*
 * Private functions start here.
 */

void
BoundedEnergySource::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  UpdateEnergySource ();  // start periodic update
}

void
BoundedEnergySource::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  BreakDeviceEnergyModelRefCycle ();  // break reference cycle
}

void
BoundedEnergySource::HandleEnergyDrainedEvent (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("BoundedEnergySource:Energy depleted!");
  NotifyEnergyDrained (); // notify DeviceEnergyModel objects
  if (m_remainingEnergyJ <= 0)
    {
      m_remainingEnergyJ = 0; // energy never goes below 0
    }
}

void
BoundedEnergySource::HandleEnergyRechargedEvent (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("BoundedEnergySource:Energy recharged!");
  NotifyEnergyRecharged (); // notify DeviceEnergyModel objects
}

void
BoundedEnergySource::CalculateRemainingEnergy (void)
{
  NS_LOG_FUNCTION (this);
  double totalCurrentA = CalculateTotalCurrent ();
  Time duration = Simulator::Now () - m_lastUpdateTime;
  NS_ASSERT (duration.GetSeconds () >= 0);
  // energy = current * voltage * time
  double energyToDecreaseJ = totalCurrentA * m_supplyVoltageV * duration.GetSeconds ();

  double remainingEnergy = m_remainingEnergyJ.Get ();
  remainingEnergy -= energyToDecreaseJ;

  if (remainingEnergy <= 0)
    {
      m_remainingEnergyJ = 0;
    }
  else if (remainingEnergy > m_initialEnergyJ)
    {
      m_remainingEnergyJ = m_initialEnergyJ;
    }
  else
    {
      m_remainingEnergyJ -= energyToDecreaseJ;
    }

  NS_LOG_DEBUG ("BoundedEnergySource:Remaining energy = " << m_remainingEnergyJ);
}

} // namespace ns3
