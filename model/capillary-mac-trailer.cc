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
#include <ns3/packet.h>
#include "capillary-mac-trailer.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CapillaryMacTrailer);

CapillaryMacTrailer::CapillaryMacTrailer (void) :
  m_crc (0)
{
}

CapillaryMacTrailer::CapillaryMacTrailer (Ptr<const Packet> p) :
  m_crc (0)
{
  SetCRC (p);
}

CapillaryMacTrailer::~CapillaryMacTrailer (void)
{
}

TypeId CapillaryMacTrailer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CapillaryMacTrailer")
    .SetParent<Trailer> ()
    .SetGroupName ("m2m-capillary")
    .AddConstructor<CapillaryMacTrailer> ();
  return tid;
}

TypeId CapillaryMacTrailer::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void CapillaryMacTrailer::Print (std::ostream &os) const
{
  os << " CRC = " << this->m_crc;
}

uint32_t CapillaryMacTrailer::GetSerializedSize (void) const
{
  return (16) / 8;
}

void CapillaryMacTrailer::Serialize (Buffer::Iterator start) const
{
  start.Prev (GetSerializedSize ());
  start.WriteU16 (this->m_crc);
}

uint32_t CapillaryMacTrailer::Deserialize (Buffer::Iterator start)
{
  start.Prev (GetSerializedSize ());
  m_crc = start.ReadU16 ();

  return GetSerializedSize ();
}

uint16_t CapillaryMacTrailer::GetCRC (void) const
{
  return this->m_crc;
}

void CapillaryMacTrailer::SetCRC (Ptr<const Packet> p)
{
  uint16_t size = p->GetSize ();
  uint8_t *serial_packet = new uint8_t[size];

  p->CopyData (serial_packet, size);

  this->m_crc = GenerateCrc16 (serial_packet, size);
  delete[] serial_packet;
}

bool CapillaryMacTrailer::CheckCRC (Ptr<const Packet> p)
{

  uint16_t checkCrc;
  uint16_t size = p->GetSize ();
  uint8_t *serial_packet = new uint8_t[size];

  p->CopyData (serial_packet, size);

  checkCrc = GenerateCrc16 (serial_packet, size);
  delete[] serial_packet;
  return (checkCrc == GetCRC ());
}

uint16_t CapillaryMacTrailer::GenerateCrc16 (uint8_t *data, int length)
{
  int i;
  uint16_t accumulator = 0;

  for (i = 0; i < length; ++i)
    {
      accumulator ^= *data;
      accumulator = (accumulator >> 8) | (accumulator << 8);
      accumulator ^= (accumulator & 0xff00) << 4;
      accumulator ^= (accumulator >> 8) >> 4;
      accumulator ^= (accumulator & 0xff00) >> 5;
      ++data;
    }
  return accumulator;
}

std::ostream&
operator << (std::ostream& os, CapillaryMacTrailer trailer)
{
  trailer.Print (os);
  return os;
}
} // namespace ns3
