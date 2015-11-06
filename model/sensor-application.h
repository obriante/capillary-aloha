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
#ifndef MODEL_SENSOR_APPLICATION_H_
#define MODEL_SENSOR_APPLICATION_H_

#include <ns3/application.h>
#include <ns3/callback.h>
#include <ns3/event-id.h>
#include <ns3/nstime.h>
#include <ns3/packet.h>
#include <ns3/ptr.h>
#include <ns3/random-variable-stream.h>
#include <ns3/traced-callback.h>
#include <sys/types.h>

#include <ns3/capillary-net-device.h>

namespace ns3 {

/*
 *
 */
class SensorApplication : public Application
{
public:
  SensorApplication (void);
  virtual ~SensorApplication (void);
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief Assign a fixed random variable stream number to the random variables
   * used by this model.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);
  Ptr<CapillaryNetDevice> GetDev () const;
  void SetDev (Ptr<CapillaryNetDevice> dev);

private:
  Ptr<RandomVariableStream> m_random;
  uint32_t m_pktSize; //!< Size of packets
  Time m_lastStartTime; //!< Time last packet sent

  EventId m_enqueEvent; //!< Event id for next start or stop event

  TracedCallback<Ptr<const Packet> > m_dataTrace; //!< Traced Callback: enqueued packet.

  Ptr<CapillaryNetDevice> m_dev;

  /** */
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  /**
   * \brief Cancel all pending events.
   */
  void CancelEvents (void);

  void Enqueue (void);
  void ScheduleEvent (void);

protected:
  virtual void DoDispose (void);
  virtual void DoInitialize (void);
};

} /* namespace ns3 */

#endif /* MODEL_SENSOR_APPLICATION_H_ */
