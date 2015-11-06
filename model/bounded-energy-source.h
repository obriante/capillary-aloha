/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef MODEL_BOUNDED_ENERGY_SOURCE_H_
#define MODEL_BOUNDED_ENERGY_SOURCE_H_

#include "ns3/traced-value.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/energy-source.h"

namespace ns3 {

/**
 * \ingroup energy
 * BoundedEnergySource decreases/increases remaining energy stored in itself in
 * linearly.
 *
 */
class BoundedEnergySource : public EnergySource
{
public:
  static TypeId GetTypeId (void);
  BoundedEnergySource ();
  virtual ~BoundedEnergySource ();

  /**
   * \return Initial energy stored in energy source, in Joules.
   *
   * Implements GetInitialEnergy.
   */
  virtual double GetInitialEnergy (void) const;

  /**
   * \returns Supply voltage at the energy source.
   *
   * Implements GetSupplyVoltage.
   */
  virtual double GetSupplyVoltage (void) const;

  /**
   * \return Remaining energy in energy source, in Joules
   *
   * Implements GetRemainingEnergy.
   */
  virtual double GetRemainingEnergy (void);

  /**
   * \returns Energy fraction.
   *
   * Implements GetEnergyFraction.
   */
  virtual double GetEnergyFraction (void);

  /**
   * Implements UpdateEnergySource.
   */
  virtual void UpdateEnergySource (void);

  /**
   * \param initialEnergyJ Initial energy, in Joules
   *
   * Sets initial energy stored in the energy source. Note that initial energy
   * is assumed to be set before simulation starts and is set only once per
   * simulation.
   */
  void SetInitialEnergy (double initialEnergyJ);

  /**
   * \param supplyVoltageV Supply voltage at the energy source, in Volts.
   *
   * Sets supply voltage of the energy source.
   */
  void SetSupplyVoltage (double supplyVoltageV);

  /**
   * \param interval Energy update interval.
   *
   * This function sets the interval between each energy update.
   */
  void SetEnergyUpdateInterval (Time interval);

  /**
   * \returns The interval between each energy update.
   */
  Time GetEnergyUpdateInterval (void) const;
  double GetStartingPercentage () const;
  void SetStartingPercentage (double startingPercentage);

private:
  /// Defined in ns3::Object
  void DoInitialize (void);

  /// Defined in ns3::Object
  void DoDispose (void);

  /**
   * Handles the remaining energy going to zero event. This function notifies
   * all the energy models aggregated to the node about the energy being
   * depleted. Each energy model is then responsible for its own handler.
   */
  void HandleEnergyDrainedEvent (void);

  /**
   * Handles the remaining energy exceeding the high threshold after it went
   * below the low threshold. This function notifies all the energy models
   * aggregated to the node about the energy being recharged. Each energy model
   * is then responsible for its own handler.
   */
  void HandleEnergyRechargedEvent (void);

  /**
   * Calculates remaining energy. This function uses the total current from all
   * device models to calculate the amount of energy to decrease. The energy to
   * decrease is given by:
   *    energy to decrease = total current * supply voltage * time duration
   * This function subtracts the calculated energy to decrease from remaining
   * energy.
   */
  void CalculateRemainingEnergy (void);


  void InternalConfig (void);

private:
  double m_startingPercentage;                    // The starting energy percentage value
  double m_initialEnergyJ;                // initial energy, in Joules
  double m_supplyVoltageV;                // supply voltage, in Volts
  double m_lowBatteryTh;                  // low battery threshold, as a fraction of the initial energy
  double m_highBatteryTh;                 // high battery threshold, as a fraction of the initial energy
  bool m_depleted;                        // set to true when the remaining energy goes below the low threshold,
                                          // set to false again when the remaining energy exceeds the high threshold
  TracedValue<double> m_remainingEnergyJ; // remaining energy, in Joules
  EventId m_energyUpdateEvent;            // energy update event
  Time m_lastUpdateTime;                  // last update time
  Time m_energyUpdateInterval;            // energy update interval

};

} // namespace ns3

#endif /* MODEL_BOUNDED_ENERGY_SOURCE_H_ */
