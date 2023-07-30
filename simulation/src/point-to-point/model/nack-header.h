#ifndef NACK_HEADER_H
#define NACK_HEADER_H

#include <stdint.h>
#include "ns3/header.h"
#include "ns3/buffer.h"
#include "ns3/int-header.h"

namespace ns3 {

/**
 * \ingroup Pause
 * \brief Header for the Negative Acknowledgement of IRN
 */
 
class NackHeader : public Header
{
public:
 
  enum {
	  FLAG_CNP = 0
  };
  NackHeader (uint16_t pg);
  NackHeader ();
  virtual ~NackHeader ();

//Setters
  /**
   * \param pg The PG
   */
  void SetPG (uint16_t pg);
  void SetSeq(uint32_t seq);
  void SetNackSeq(uint32_t nackSeq);
  void SetSport(uint32_t _sport);
  void SetDport(uint32_t _dport);
  void SetTs(uint64_t ts);
  void SetCnp();
  void SetIntHeader(const IntHeader &_ih);

//Getters
  /**
   * \return The pg
   */
  uint16_t GetPG () const;
  uint32_t GetSeq() const;
  uint32_t GetNackSeq() const;
  uint16_t GetPort() const;
  uint16_t GetSport() const;
  uint16_t GetDport() const;
  uint64_t GetTs() const;
  uint8_t GetCnp() const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  static uint32_t GetBaseSize(); // size without INT

private:
  uint16_t sport, dport;
  uint16_t flags;
  uint16_t m_pg;
  uint32_t m_seq; // the qbb sequence number.
  uint32_t m_nackSeq;
  IntHeader ih;
  
};

}; // namespace ns3

#endif /* NACK_HEADER */
