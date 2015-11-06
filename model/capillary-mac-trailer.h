/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
#ifndef CAPILLARY_MAC_TRAILER_H_
#define CAPILLARY_MAC_TRAILER_H_

#include <ns3/trailer.h>

namespace ns3 {

class Packet;

class CapillaryMacTrailer : public Trailer
{

public:
  CapillaryMacTrailer (void);
  CapillaryMacTrailer (Ptr<const Packet> p);
  virtual ~CapillaryMacTrailer (void);

  /**
   * Get the type ID.
   *
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  // Inherited from the Trailer class.
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  /**
   * Get this trailers CRC value.
   *
   * \return the CRC value.
   */
  uint16_t GetCRC (void) const;

  /**
   * Calculate and set the CRC value based on the given packet.
   *
   * \param p the packet for which the CRC should be calculated
   */
  void SetCRC (Ptr<const Packet> p);

  /**
   *
   * \param p the packet to be checked
   * \return false, if the CRC values do not match, true otherwise
   */
  bool CheckCRC (Ptr<const Packet> p);

private:
  /**
   * Calculate the 16-bit FCS value.
   * CRC16-CCITT with a generator polynomial = ^16 + ^12 + ^5 + 1, LSB first and
   * initial value = 0x0000.
   *
   * \param data the checksum will be calculated over this data
   * \param length the length of the data
   * \return the checksum
   */
  uint16_t GenerateCrc16 (uint8_t *data, int length);

  uint16_t m_crc;

};
// CapillaryMacTrailer

}// namespace ns3

#endif /* CAPILLARY_MAC_TRAILER_H_ */
