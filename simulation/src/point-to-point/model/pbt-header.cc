/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 New York University
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
 * Author: Adrian S.-W. Tam <adrian.sw.tam@gmail.com>
 */

#include <stdint.h>
#include <iostream>
#include "pbt-header.h"
#include "ns3/buffer.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("PbtHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PbtHeader);

PbtHeader::PbtHeader (uint32_t rxBytes,uint32_t queueSize,uint32_t recentRxBytes,uint32_t recentMarkedRxBytes,uint64_t linkBw, uint32_t switchID, uint16_t srcPort, uint16_t dstPort, uint16_t pg)
  : m_rxBytes(rxBytes), m_queueSize(queueSize), m_recentRxBytes(recentRxBytes), m_recentMarkedRxBytes(recentMarkedRxBytes), m_linkBw(linkBw), m_switchID(switchID), m_srcPort(srcPort), m_dstPort(dstPort), m_pg(pg)
{
}

PbtHeader::PbtHeader ()
  : m_rxBytes(0), m_queueSize(0), m_recentRxBytes(0), m_recentMarkedRxBytes(0), m_linkBw(0), m_srcPort(0), m_dstPort(0), m_pg(0)
{}

PbtHeader::~PbtHeader ()
{}

TypeId
PbtHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PbtHeader")
    .SetParent<Header> ()
    .AddConstructor<PbtHeader> ()
    ;
  return tid;
}
TypeId
PbtHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void PbtHeader::Print (std::ostream &os) const
{
  os << "switchid=" << m_switchID << ", srcport=" << m_srcPort << ", dstport=" << m_dstPort << ", pg=" << m_pg << ", rxbytes=" << m_rxBytes << ", queuesize=" << m_queueSize << ", recentrxbytes=" << m_recentRxBytes  << ", recentmarkedrxbytes=" << m_recentMarkedRxBytes << ", linkbw=" << m_linkBw;
}
uint32_t PbtHeader::GetSerializedSize (void)  const
{
  return 34;
}
void PbtHeader::Serialize (Buffer::Iterator start)  const
{
  start.WriteHtonU16 (m_srcPort);
  start.WriteHtonU16 (m_dstPort);
  start.WriteU16 (m_pg);
  start.WriteU32 (m_switchID);
  start.WriteU32 (m_rxBytes);
  start.WriteU32 (m_queueSize);
  start.WriteU32 (m_recentRxBytes);
  start.WriteU32 (m_recentMarkedRxBytes);
  start.WriteU64 (m_linkBw);
}

uint32_t PbtHeader::Deserialize (Buffer::Iterator start)
{
  m_srcPort = start.ReadNtohU16 ();
  m_dstPort = start.ReadNtohU16 ();
  m_pg = start.ReadU16();
  m_switchID = start.ReadU32 ();
  m_rxBytes = start.ReadU32 ();
  m_queueSize = start.ReadU32 ();
  m_recentRxBytes = start.ReadU32 ();
  m_recentMarkedRxBytes = start.ReadU32 ();
  m_linkBw = start.ReadU64 ();
  return GetSerializedSize ();
}


}; // namespace ns3
