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
#include <ns3/address-utils.h>

#include "capillary-mac-header.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CapillaryMacHeader);

CapillaryMacHeader::CapillaryMacHeader (void)
{
  SetSeqNum (0);
  SetFrameControl (0);
  SetPayloadSize (0);
  SetSrcAddr (Mac64Address ());
  SetDstAddr (Mac64Address ());
}

CapillaryMacHeader::CapillaryMacHeader (FrameType frameType)
{
  SetSeqNum (0);
  SetFrameControl (0);
  SetPayloadSize (0);
  SetFrameType (frameType);
  SetSrcAddr (Mac64Address ());
  SetDstAddr (Mac64Address ());
}

CapillaryMacHeader::~CapillaryMacHeader (void)
{
}

uint8_t CapillaryMacHeader::GetSeqNum (void) const
{
  return m_SeqNum;
}

Mac64Address CapillaryMacHeader::GetDstAddr (void) const
{
  return m_dstAddr;
}

Mac64Address CapillaryMacHeader::GetSrcAddr (void) const
{
  return m_srcAddr;
}

void CapillaryMacHeader::SetSeqNum (uint8_t seqNum)
{
  m_SeqNum = seqNum;
}

void CapillaryMacHeader::SetFrameType (FrameType frameType)
{
  m_frameType = frameType;
}

void CapillaryMacHeader::SetSrcAddr (Mac64Address addr)
{
  m_srcAddr = addr;
}

void CapillaryMacHeader::SetDstAddr (Mac64Address addr)
{
  m_dstAddr = addr;
}

uint8_t CapillaryMacHeader::GetMoreFrag () const
{
  return m_moreFrag;
}

void CapillaryMacHeader::SetMoreFrag (bool is)
{
  if (is)
    {
      m_moreFrag = 0x01;
    }
  else
    {
      m_moreFrag = 0x00;
    }
}

uint8_t CapillaryMacHeader::GetReserved1 () const
{
  return m_reserved1;
}

void CapillaryMacHeader::SetReserved1 (bool is)
{
  if (is)
    {
      m_reserved1 = 0x01;
    }
  else
    {
      m_reserved1 = 0x00;
    }
}

uint8_t CapillaryMacHeader::GetReserved2 () const
{
  return m_reserved2;
}

void CapillaryMacHeader::SetReserved2 (bool is)
{
  if (is)
    {
      m_reserved2 = 0x01;
    }
  else
    {
      m_reserved2 = 0x00;
    }
}

uint8_t CapillaryMacHeader::GetReserved3 () const
{
  return m_reserved3;
}

void CapillaryMacHeader::SetReserved3 (bool is)
{
  if (is)
    {
      m_reserved3 = 0x01;
    }
  else
    {
      m_reserved3 = 0x00;
    }

}

uint8_t CapillaryMacHeader::GetReserved4 () const
{
  return m_reserved4;
}

void CapillaryMacHeader::SetReserved4 (bool is)
{
  if (is)
    {
      m_reserved4 = 0x01;
    }
  else
    {
      m_reserved4 = 0x00;
    }
}

uint8_t CapillaryMacHeader::GetRetry () const
{
  return m_retry;
}

void CapillaryMacHeader::SetRetry (bool is)
{
  if (is)
    {
      m_retry = 0x01;
    }
  else
    {
      m_retry = 0x00;
    }
}

uint16_t CapillaryMacHeader::GetPayloadSize (void) const
{
  return m_payloadSize;
}

void CapillaryMacHeader::SetPayloadSize (uint16_t size)
{
  m_payloadSize = size;
}

CapillaryMacHeader::FrameType CapillaryMacHeader::GetFrameType (void) const
{
  return (CapillaryMacHeader::FrameType) m_frameType;
}

TypeId CapillaryMacHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CapillaryMacHeader")
    .SetParent<Header> ()
    .SetGroupName ("m2m-capillary")
    .AddConstructor<CapillaryMacHeader> ();
  return tid;
}

TypeId CapillaryMacHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void CapillaryMacHeader::Print (std::ostream &os) const
{
  os << "Source Address = " << m_srcAddr << ", ";
  os << "Destination Address = " << m_dstAddr << ", ";

  os << "Frame Control( ";
  os << "Frame Type=" << (uint32_t) m_frameType << ", ";

  os << "More Fragment=" << (uint32_t) m_moreFrag << ", ";
  os << "Retry=" << (uint32_t) m_retry << ", ";
  os << "Reserved1=" << (uint32_t) m_reserved1 << ", ";
  os << "Reserved2=" << (uint32_t) m_reserved2 << ", ";
  os << "Reserved3=" << (uint32_t) m_reserved3 << ", ";
  os << "Reserved4=" << (uint32_t) m_reserved4 << "), ";

  os << "Payload Size = " << m_payloadSize << ", ";
  os << "Sequence Num = " << (uint16_t) m_SeqNum << "";
}

uint32_t CapillaryMacHeader::GetSerializedSize (void) const
{

  return (8 + 8 + 16 + 64 + 64) / 8;
}

void CapillaryMacHeader::Serialize (Buffer::Iterator start) const
{

  Buffer::Iterator i = start;
  WriteTo (i, m_dstAddr);
  WriteTo (i, m_srcAddr);
  i.WriteU16 (m_payloadSize);
  i.WriteU8 (GetFrameControl ());
  i.WriteU8 (m_SeqNum);
}

uint32_t CapillaryMacHeader::Deserialize (Buffer::Iterator start)
{

  Buffer::Iterator i = start;

  ReadFrom (i, m_dstAddr);
  ReadFrom (i, m_srcAddr);
  SetPayloadSize (i.ReadU16 ());
  SetFrameControl (i.ReadU8 ());
  SetSeqNum (i.ReadU8 ());

  return i.GetDistanceFrom (start);
}

std::ostream& operator << (std::ostream& os, CapillaryMacHeader header)
{
  header.Print (os);
  return os;
}

uint8_t CapillaryMacHeader::GetFrameControl (void) const
{
  uint8_t val = 0;
  val |= (m_frameType << 6) & (0x03 << 6);
  val |= (m_moreFrag << 5) & (0x01 << 5);
  val |= (m_retry << 4) & (0x01 << 4);
  val |= (m_reserved1 << 3) & (0x01 << 3);
  val |= (m_reserved2 << 2) & (0x01 << 2);
  val |= (m_reserved3 << 1) & (0x01 << 1);
  val |= (m_reserved4)      & (0x01);

  return val;
}

void CapillaryMacHeader::SetFrameControl (uint8_t control)
{
  m_frameType = (control >> 6) & 0x03;
  m_moreFrag  = (control >> 5) & 0x01;
  m_retry     = (control >> 4) & 0x01;
  m_reserved1 = (control >> 3) & 0x01;
  m_reserved2 = (control >> 2) & 0x01;
  m_reserved3 = (control >> 1) & 0x01;
  m_reserved4 = (control)      & 0x01;
}

} // namespace ns3
