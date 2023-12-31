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

#ifndef PBT_HEADER_H
#define PBT_HEADER_H

#include <stdint.h>
#include "ns3/header.h"
#include "ns3/buffer.h"

namespace ns3 {

/**
 * \ingroup Pbt
 * \brief Header for Postcard Based Telemetry
 *
 */

class PbtHeader : public Header
{
public:
  PbtHeader (uint32_t rxBytes,uint32_t queueSize,uint32_t recentRxBytes,uint32_t recentMarkedRxBytes,uint64_t linkBw, uint32_t switchID, uint16_t srcPort, uint16_t dstPort, uint16_t pg);
  PbtHeader ();
  virtual ~PbtHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint32_t m_rxBytes;
  uint32_t m_queueSize;
  uint32_t m_recentRxBytes;
  uint32_t m_recentMarkedRxBytes;
  uint64_t m_linkBw;
  uint32_t m_switchID;
  uint16_t m_srcPort;
  uint16_t m_dstPort;
  uint16_t m_pg;
};

}; // namespace ns3

#endif /* Pbt_HEADER */
