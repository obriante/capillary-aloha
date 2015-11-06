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
/* python2 waf --run "test-runner" --command-template="gdb --args %s --suite=capillary-mac-packet" */
#include <ns3/test.h>
#include <ns3/packet.h>
#include <ns3/capillary-mac-header.h>
#include <ns3/capillary-mac-trailer.h>
#include <ns3/mac64-address.h>
#include <ns3/log.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("capillary-packet-test");

// This is an example TestCase.
class CapillaryPacketTestCase : public TestCase
{
public:
  CapillaryPacketTestCase ();
  virtual ~CapillaryPacketTestCase ();

private:
  virtual void DoRun (void);
};

CapillaryPacketTestCase::CapillaryPacketTestCase () :
  TestCase ("Test the CAPILLARY MAC header and trailer classes")
{
}

CapillaryPacketTestCase::~CapillaryPacketTestCase ()
{
}

void CapillaryPacketTestCase::DoRun (void)
{

  uint16_t PayloadSize = 120; // 120 bytes of dummy data

  CapillaryMacHeader macHdr (CapillaryMacHeader::CAPILLARY_MAC_RFD);

  Mac64Address srcAddr = Mac64Address::Allocate ();
  macHdr.SetSrcAddr (srcAddr);
  macHdr.SetDstAddr (Mac64Address ("ff:ff:ff:ff:ff:ff:ff:ff"));
  macHdr.SetPayloadSize (PayloadSize);

  std::cout << "Header Size Serialized: " << macHdr.GetSerializedSize () << std::endl;

  Ptr<Packet> p = Create<Packet> (PayloadSize);
  NS_TEST_ASSERT_MSG_EQ (p->GetSize (), PayloadSize, "Packet created with unexpected size");
  p->AddHeader (macHdr);
  p->RemoveHeader (macHdr);

  macHdr.SetRetry (true);
  p->AddHeader (macHdr);

  std::cout << " <--Mac Header added " << std::endl;

  NS_TEST_ASSERT_MSG_EQ (p->GetSize (), 140, "Packet wrong size after macHdr addition");

  CapillaryMacTrailer macTrailer (p);
  p->AddTrailer (macTrailer);
  std::cout << " <--Mac trailer added " << std::endl;
  NS_TEST_ASSERT_MSG_EQ (p->GetSize (), 142, "Packet wrong size after macTrailer addition");

  // Test serialization and deserialization
  uint32_t size = p->GetSerializedSize ();
  uint8_t buffer[size];
  p->Serialize (buffer, size);
  Ptr<Packet> p2 = Create<Packet> (buffer, size, true);

  p2->Print (std::cout);
  std::cout << " <--Packet P2 " << std::endl;

  NS_TEST_ASSERT_MSG_EQ (p2->GetSize (), 142, "Packet wrong size after deserialization");

  CapillaryMacHeader receivedMacHdr;
  p2->RemoveHeader (receivedMacHdr);

  receivedMacHdr.Print (std::cout);
  std::cout << " <--P2 Mac Header " << std::endl;

  NS_TEST_ASSERT_MSG_EQ (p2->GetSize (), 122, "Packet wrong size after removing macHdr");

  CapillaryMacTrailer receivedMacTrailer;
  p2->RemoveTrailer (receivedMacTrailer);

  p2->Print (std::cout);
  std::cout << " <--Payload P2 " << std::endl;

  NS_TEST_ASSERT_MSG_EQ (p2->GetSize (), PayloadSize, "Packet wrong size after removing headers and trailers");
}

// ==============================================================================
class CapillaryPacketTestSuite : public TestSuite
{
public:
  CapillaryPacketTestSuite ();
};

CapillaryPacketTestSuite::CapillaryPacketTestSuite () :
  TestSuite ("capillary-packet-test", UNIT)
{
  AddTestCase (new CapillaryPacketTestCase, TestCase::QUICK);
}

static CapillaryPacketTestSuite CapillaryPacketTestSuite;
