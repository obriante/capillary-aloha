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
#ifndef CAPILLARY_MAC_HEADER_H_
#define CAPILLARY_MAC_HEADER_H_

#include <ns3/buffer.h>
#include <ns3/header.h>
#include <ns3/mac64-address.h>
#include <iostream>

namespace ns3 {

/**
 * \ingroup m2m-capillary
 *
 * Implements the CAPILLARY MAC header
 */
class CapillaryMacHeader : public Header
{

public:
  typedef enum
  {
    CAPILLARY_MAC_RFD = 0x00,
    CAPILLARY_MAC_FBP = 0x01,
    CAPILLARY_MAC_DATA = 0x02
  } FrameType;

  CapillaryMacHeader (void);

  /**
   * Constructor
   * \param macType
   * \param seqNum the sequence number
   */
  CapillaryMacHeader (FrameType macType);

  virtual ~CapillaryMacHeader (void);


  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Get the frame Sequence number
   * \return the sequence number
   */
  uint8_t GetSeqNum (void) const;
  /**
   * Get the Destination address
   * \return the Destination address
   */
  Mac64Address GetDstAddr (void) const;
  /**
   * Get the Source Extended address
   * \return the Source Extended address
   */
  Mac64Address GetSrcAddr (void) const;
  /**
   * Get the header type
   * \return the header type
   */
  FrameType GetFrameType (void) const;
  /**
     * Get the payload size
     * \return the payload size
     */
  uint16_t GetPayloadSize (void) const;

  /**
   * Set the Sequence number
   * \param seqNum sequence number
   */
  void SetSeqNum (uint8_t seqNum);
  /**
   * Set the Frame Control field "Frame Type" bits
   * \param macType the frame type
   */
  void SetFrameType (FrameType frameType);
  /**
   * Set Source address
   * \param addr source address (64 bit)
   */
  void SetSrcAddr (Mac64Address addr);
  /**
   * Set Destination address
   * \param addr destination address (64 bit)
   */
  void SetDstAddr (Mac64Address addr);
  /**
   * Set the payload size
   * \param size the payload size
   */
  void SetPayloadSize (uint16_t size);

  uint8_t GetMoreFrag (void) const;
  void SetMoreFrag (bool is);
  uint8_t GetReserved1 (void) const;
  void SetReserved1 (bool is);
  uint8_t GetReserved2 (void) const;
  void SetReserved2 (bool is);
  uint8_t GetReserved3 (void) const;
  void SetReserved3 (bool is);
  uint8_t GetReserved4 (void) const;
  void SetReserved4 (bool is);
  uint8_t GetRetry (void) const;
  void SetRetry (bool is);

private:
  /**
   * Return the raw Frame Control field.
   *
   * \return the raw Frame Control field
   */
  uint8_t GetFrameControl (void) const;
  /**
   * Set the Frame Control field with the given raw value.
   *
   * \param control the raw Frame Control field value
   */
  void SetFrameControl (uint8_t control);

private:
  /** Addressing fields */
  Mac64Address m_srcAddr;                          //!< Src addr
  Mac64Address m_dstAddr;                          //!< Dst addr

  /** Frame control field*/

  uint8_t m_frameType;
  uint8_t m_moreFrag;
  uint8_t m_retry;
  uint8_t m_reserved1;
  uint8_t m_reserved2;
  uint8_t m_reserved3;
  uint8_t m_reserved4;

  /* Sequence Number */
  uint8_t m_SeqNum;                                    //!< Sequence Number

  /* Payload Size */
  uint16_t m_payloadSize;  //!< for future fragmentation
};
// CapillaryMacHeader

std::ostream& operator<< (std::ostream& os, CapillaryMacHeader header);

} // namespace ns3

#endif /* CAPILLARY_MAC_HEADER_H_ */
