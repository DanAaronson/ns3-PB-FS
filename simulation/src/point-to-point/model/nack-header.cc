#include <stdint.h>
#include <iostream>
#include "nack-header.h"
#include "ns3/buffer.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("NackHeader");

namespace ns3 {

	NS_OBJECT_ENSURE_REGISTERED(NackHeader);

	NackHeader::NackHeader(uint16_t pg)
		: m_pg(pg), sport(0), dport(0), flags(0), m_seq(0), m_nackSeq(0)
	{
	}

	NackHeader::NackHeader()
		: m_pg(0), sport(0), dport(0), flags(0), m_seq(0), m_nackSeq(0)
	{}

	NackHeader::~NackHeader()
	{}

	void NackHeader::SetPG(uint16_t pg)
	{
		m_pg = pg;
	}

	void NackHeader::SetSeq(uint32_t seq)
	{
		m_seq = seq;
	}

	void NackHeader::SetNackSeq(uint32_t nackSeq)
	{
		m_nackSeq = nackSeq;
	}

	void NackHeader::SetSport(uint32_t _sport){
		sport = _sport;
	}
	void NackHeader::SetDport(uint32_t _dport){
		dport = _dport;
	}

	void NackHeader::SetTs(uint64_t ts){
		NS_ASSERT_MSG(IntHeader::mode == 1, "NackHeader cannot SetTs when IntHeader::mode != 1");
		ih.ts = ts;
	}
	void NackHeader::SetCnp(){
		flags |= 1 << FLAG_CNP;
	}
	void NackHeader::SetIntHeader(const IntHeader &_ih){
		ih = _ih;
	}

	uint16_t NackHeader::GetPG() const
	{
		return m_pg;
	}

	uint32_t NackHeader::GetSeq() const
	{
		return m_seq;
	}

	uint32_t NackHeader::GetNackSeq() const
	{
		return m_nackSeq;
	}

	uint16_t NackHeader::GetSport() const{
		return sport;
	}
	uint16_t NackHeader::GetDport() const{
		return dport;
	}

	uint64_t NackHeader::GetTs() const {
		NS_ASSERT_MSG(IntHeader::mode == 1, "NackHeader cannot GetTs when IntHeader::mode != 1");
		return ih.ts;
	}
	uint8_t NackHeader::GetCnp() const{
		return (flags >> FLAG_CNP) & 1;
	}

	TypeId
		NackHeader::GetTypeId(void)
	{
		static TypeId tid = TypeId("ns3::NackHeader")
			.SetParent<Header>()
			.AddConstructor<NackHeader>()
			;
		return tid;
	}
	TypeId
		NackHeader::GetInstanceTypeId(void) const
	{
		return GetTypeId();
	}
	void NackHeader::Print(std::ostream &os) const
	{
		os << "qbb:" << "pg=" << m_pg << ",seq=" << m_seq << ",nackseq=" << m_nackSeq;
	}
	uint32_t NackHeader::GetSerializedSize(void)  const
	{
		return GetBaseSize() + IntHeader::GetStaticSize();
	}
	uint32_t NackHeader::GetBaseSize() {
		NackHeader tmp;
		return sizeof(tmp.sport) + sizeof(tmp.dport) + sizeof(tmp.flags) + sizeof(tmp.m_pg) + sizeof(tmp.m_seq) + sizeof(tmp.m_nackSeq);
	}
	void NackHeader::Serialize(Buffer::Iterator start)  const
	{
		Buffer::Iterator i = start;
		i.WriteU16(sport);
		i.WriteU16(dport);
		i.WriteU16(flags);
		i.WriteU16(m_pg);
		i.WriteU32(m_seq);
		i.WriteU32(m_nackSeq);

		// write IntHeader
		ih.Serialize(i);
	}

	uint32_t NackHeader::Deserialize(Buffer::Iterator start)
	{
		Buffer::Iterator i = start;
		sport = i.ReadU16();
		dport = i.ReadU16();
		flags = i.ReadU16();
		m_pg = i.ReadU16();
		m_seq = i.ReadU32();
		m_nackSeq = i.ReadU32();

		// read IntHeader
		ih.Deserialize(i);
		return GetSerializedSize();
	}
}; // namespace ns3
