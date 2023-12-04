#include "ns3/ipv4.h"
#include "ns3/packet.h"
#include "ns3/ipv4-header.h"
#include "ns3/pause-header.h"
#include "ns3/flow-id-tag.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "switch-node.h"
#include "qbb-net-device.h"
#include "ppp-header.h"
#include "ns3/int-header.h"
#include <cmath>

namespace ns3 {

TypeId SwitchNode::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SwitchNode")
    .SetParent<Node> ()
    .AddConstructor<SwitchNode> ()
    .AddAttribute("EcnEnabled",
            "Enable ECN marking.",
            BooleanValue(false),
            MakeBooleanAccessor(&SwitchNode::m_ecnEnabled),
            MakeBooleanChecker())
    .AddAttribute("CcMode",
            "CC mode.",
            UintegerValue(0),
            MakeUintegerAccessor(&SwitchNode::m_ccMode),
            MakeUintegerChecker<uint32_t>())
    .AddAttribute("AckHighPrio",
            "Set high priority for ACK/NACK or not",
            UintegerValue(0),
            MakeUintegerAccessor(&SwitchNode::m_ackHighPrio),
            MakeUintegerChecker<uint32_t>())
    .AddAttribute("MaxRtt",
            "Max Rtt of the network",
            UintegerValue(9000),
            MakeUintegerAccessor(&SwitchNode::m_maxRtt),
            MakeUintegerChecker<uint32_t>())
    .AddAttribute("PbtEnabled",
            "Enable Pbt.",
            BooleanValue(false),
            MakeBooleanAccessor(&SwitchNode::m_pbtEnabled),
            MakeBooleanChecker())
    .AddAttribute("PfcEnabled",
            "Enable Pfc.",
            BooleanValue(true),
            MakeBooleanAccessor(&SwitchNode::m_pfcEnabled),
            MakeBooleanChecker())
  ;
  return tid;
}

SwitchNode::SwitchNode(){
    m_ecmpSeed = m_id;
    m_node_type = 1;
    m_mmu = CreateObject<SwitchMmu>();
    for (uint32_t i = 0; i < pCnt; i++)
        m_txBytes[i] = 0;
    for (uint32_t i = 0; i < pCnt; i++)
        m_lastPktSize[i] = m_lastPktTs[i] = 0;
    for (uint32_t i = 0; i < pCnt; i++)
        m_u[i] = 0;

    for (uint32_t i = 0; i < pCnt; i++)
        for (uint32_t j = 0; j < slotTot; j++) 
            m_rxBytesSlots[i][j] = 0;
    for (uint32_t i = 0; i < pCnt; i++)
        for (uint32_t j = 0; j < slotTot; j++) 
            m_rxMarkedBytesSlots[i][j] = 0;
    for (uint32_t i = 0; i < pCnt; i++)
        for (uint32_t j = 0; j < slotTot; j++) 
            m_queueSizes[i][j] = 0;

    m_currentSlot = 0;
    m_previousSlot = slotTot - 1;
}

int SwitchNode::GetOutDev(Ptr<const Packet> p, CustomHeader &ch){
    // look up entries
    auto entry = m_rtTable.find(ch.dip);

    // no matching entry
    if (entry == m_rtTable.end())
        return -1;

    // entry found
    auto &nexthops = entry->second;

    // pick one next hop based on hash
    union {
        uint8_t u8[4+4+2+2];
        uint32_t u32[3];
    } buf;
    buf.u32[0] = ch.sip;
    buf.u32[1] = ch.dip;
    if (ch.l3Prot == 0x6)
        buf.u32[2] = ch.tcp.sport | ((uint32_t)ch.tcp.dport << 16);
    else if (ch.l3Prot == 0x11)
        buf.u32[2] = ch.udp.sport | ((uint32_t)ch.udp.dport << 16);
    else if (ch.l3Prot == 0xFC || ch.l3Prot == 0xFD)
        buf.u32[2] = ch.ack.sport | ((uint32_t)ch.ack.dport << 16);
    else if (ch.l3Prot == 0xFB)
        buf.u32[2] = ch.pbt.sport | ((uint32_t)ch.pbt.dport << 16);

    uint32_t idx = EcmpHash(buf.u8, 12, m_ecmpSeed) % nexthops.size();
    return nexthops[idx];
}

void SwitchNode::CheckAndSendPfc(uint32_t inDev, uint32_t qIndex){
    Ptr<QbbNetDevice> device = DynamicCast<QbbNetDevice>(m_devices[inDev]);
    if (m_mmu->CheckShouldPause(inDev, qIndex)){
        device->SendPfc(qIndex, 0);
        m_mmu->SetPause(inDev, qIndex);
    }
}
void SwitchNode::CheckAndSendResume(uint32_t inDev, uint32_t qIndex){
    Ptr<QbbNetDevice> device = DynamicCast<QbbNetDevice>(m_devices[inDev]);
    if (m_mmu->CheckShouldResume(inDev, qIndex)){
        device->SendPfc(qIndex, 1);
        m_mmu->SetResume(inDev, qIndex);
    }
}

int64_t SwitchNode::GetIntervalBytes(uint32_t interval, uint32_t slot_r, int64_t *byte_slots, uint32_t *last_slot_idx) {
    int64_t sum = 0;

    sum += byte_slots[m_currentSlot];
    uint32_t total_segment_num = (interval - slot_r) / (slot_len / accuracy);

    if ((interval - slot_r) % (slot_len / accuracy) != 0) {
        total_segment_num++;
    }

    uint32_t full_slot_num = total_segment_num / accuracy;
    for (uint32_t i = 0; i < full_slot_num; i++) {
        sum += byte_slots[(m_currentSlot + slotTot - i - 1) % slotTot];
        total_segment_num -= accuracy;
    }

    uint32_t last_idx = (m_currentSlot + slotTot - full_slot_num - 1) % slotTot;

    int64_t slot_multiples = (int64_t)total_segment_num * byte_slots[last_idx];
    sum += slot_multiples / accuracy;

    if (last_slot_idx != NULL) {
        *last_slot_idx = last_idx;
    }

    return sum;
}

void SwitchNode::CheckAndSendPBT(uint32_t inDev, uint32_t outDev, Ptr<Packet> p){
    CustomHeader ch(CustomHeader::L2_Header | CustomHeader::L3_Header | CustomHeader::L4_Header);
    p->PeekHeader(ch);
    uint8_t tos_val = ch.m_tos;
    if (tos_val & 0x4) {
        uint64_t ts = Simulator::Now().GetTimeStep();
        uint32_t slot_r = ts - m_slotStartTs;
        int64_t sum_rx_data = 0;
        uint32_t last_slot_idx;

        int64_t last_slot = GetIntervalBytes(dc_max_rtt / slotN, slot_r, m_rxBytesSlots[outDev], NULL);
        
        // add future slots
        sum_rx_data += w * last_slot;

        // add past slots
        sum_rx_data += GetIntervalBytes(dc_max_rtt / 2, slot_r, m_rxBytesSlots[outDev], &last_slot_idx);

        uint32_t prior_queue_size = m_queueSizes[outDev][last_slot_idx];

        Ptr<QbbNetDevice> device = DynamicCast<QbbNetDevice>(m_devices[outDev]);
        int64_t rxThresh = ((device->GetDataRate()).GetBitRate() * dc_max_rtt * 1e-9)/8;
        if (sum_rx_data + prior_queue_size > rxThresh) {
            int64_t recent_rx_data = GetIntervalBytes(marked_win_tx_time, slot_r, m_rxBytesSlots[outDev], NULL);
            int64_t recent_marked_rx_data = GetIntervalBytes(marked_win_tx_time, slot_r, m_rxMarkedBytesSlots[outDev], NULL);
            
            Ptr<QbbNetDevice> in_device = DynamicCast<QbbNetDevice>(m_devices[inDev]);
            in_device->SendPBT(sum_rx_data, prior_queue_size, recent_rx_data, recent_marked_rx_data, (device->GetDataRate()).GetBitRate(), ch);
        }
    }
}

void SwitchNode::SlotReset() {
    m_slotStartTs = Simulator::Now().GetTimeStep();
    m_previousSlot = m_currentSlot;
    m_currentSlot = (m_currentSlot == slotTot - 1) ? 0 : m_currentSlot + 1; 
    for (int i = 1; i < GetNDevices(); i++) {
        m_rxBytesSlots[i][m_currentSlot] = 0;
        m_rxMarkedBytesSlots[i][m_currentSlot] = 0;
        Ptr<QbbNetDevice> dev = DynamicCast<QbbNetDevice>(m_devices[i]);
        uint32_t queueSize = dev->GetQueue()->GetNBytesTotal();
        m_queueSizes[i][m_currentSlot] = queueSize;
    }

    slotResetEvent = Simulator::Schedule(NanoSeconds(slot_len), &SwitchNode::SlotReset, this);
}

void SwitchNode::ScheduleSlotReset(uint64_t startTime) {
    slotResetEvent = Simulator::Schedule(NanoSeconds(startTime), &SwitchNode::SlotReset, this);
}

void SwitchNode::SendToDev(Ptr<Packet>p, CustomHeader &ch){
    int idx = GetOutDev(p, ch);
    if (idx >= 0){
        NS_ASSERT_MSG(m_devices[idx]->IsLinkUp(), "The routing table look up should return link that is up");

        // determine the qIndex
        uint32_t qIndex;
        if (ch.l3Prot == 0xFF || ch.l3Prot == 0xFE || ch.l3Prot == 0xFB || (m_ackHighPrio && (ch.l3Prot == 0xFD || ch.l3Prot == 0xFC))){  //QCN or PFC or NACK or PBT, go highest priority
            qIndex = 0;
        }else{
            qIndex = (ch.l3Prot == 0x06 ? 1 : ch.udp.pg); // if TCP, put to queue 1
        }

        // admission control
        FlowIdTag t;
        p->PeekPacketTag(t);
        uint32_t inDev = t.GetFlowId();

        // PBT

        uint64_t ingress_ts = Simulator::Now().GetTimeStep();
        Ptr<QbbNetDevice> device = DynamicCast<QbbNetDevice>(m_devices[idx]);

        uint32_t transmission_time = 1e9 * (device->GetDataRate()).CalculateTxTime(p->GetSize());
        
        uint32_t arrival_data = 0;
        uint32_t end_data;
        
        if (ingress_ts - transmission_time >= m_slotStartTs) {
            end_data = p->GetSize();
        } else {
            uint32_t end_transmission_time = ingress_ts - m_slotStartTs;
            end_data = end_transmission_time * ((device->GetDataRate()).GetBitRate() / 8) * 1e-9;
            if (end_data > p->GetSize()) {
                end_data = p->GetSize();
			}
			arrival_data = p->GetSize() - end_data;
        }

        // updating slots
        m_rxBytesSlots[idx][m_previousSlot] += arrival_data;
        m_rxBytesSlots[idx][m_currentSlot] += end_data;

        uint8_t tos_val = ch.m_tos;

        if (tos_val & 0x4) {
            m_rxMarkedBytesSlots[idx][m_previousSlot] += arrival_data;
            m_rxMarkedBytesSlots[idx][m_currentSlot] += end_data;
        }

        if (m_pbtEnabled) { // if PBT enabled
            CheckAndSendPBT(inDev, idx, p);
        }

        // END PBT

        if (qIndex != 0){ //not highest priority
            if (m_mmu->CheckIngressAdmission(inDev, qIndex, p->GetSize()) && m_mmu->CheckEgressAdmission(idx, qIndex, p->GetSize())){           // Admission control
                m_mmu->UpdateIngressAdmission(inDev, qIndex, p->GetSize());
                m_mmu->UpdateEgressAdmission(idx, qIndex, p->GetSize());
            }else{
                return; // Drop
            }
            if (m_pfcEnabled) {
                CheckAndSendPfc(inDev, qIndex);
            }
        }

        m_devices[idx]->SwitchSend(qIndex, p, ch);
    }else
        return; // Drop
}

uint32_t SwitchNode::EcmpHash(const uint8_t* key, size_t len, uint32_t seed) {
  uint32_t h = seed;
  if (len > 3) {
    const uint32_t* key_x4 = (const uint32_t*) key;
    size_t i = len >> 2;
    do {
      uint32_t k = *key_x4++;
      k *= 0xcc9e2d51;
      k = (k << 15) | (k >> 17);
      k *= 0x1b873593;
      h ^= k;
      h = (h << 13) | (h >> 19);
      h += (h << 2) + 0xe6546b64;
    } while (--i);
    key = (const uint8_t*) key_x4;
  }
  if (len & 3) {
    size_t i = len & 3;
    uint32_t k = 0;
    key = &key[i - 1];
    do {
      k <<= 8;
      k |= *key--;
    } while (--i);
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    h ^= k;
  }
  h ^= len;
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}

void SwitchNode::SetEcmpSeed(uint32_t seed){
    m_ecmpSeed = seed;
}

void SwitchNode::AddTableEntry(Ipv4Address &dstAddr, uint32_t intf_idx){
    uint32_t dip = dstAddr.Get();
    m_rtTable[dip].push_back(intf_idx);
}

void SwitchNode::ClearTable(){
    m_rtTable.clear();
}

// This function can only be called in switch mode
bool SwitchNode::SwitchReceiveFromDevice(Ptr<NetDevice> device, Ptr<Packet> packet, CustomHeader &ch){
    SendToDev(packet, ch);
    return true;
}

void SwitchNode::SwitchNotifyDequeue(uint32_t ifIndex, uint32_t qIndex, Ptr<Packet> p){
    FlowIdTag t;
    p->PeekPacketTag(t);
    uint32_t inDev = t.GetFlowId();

    if (qIndex != 0){
        m_mmu->RemoveFromIngressAdmission(inDev, qIndex, p->GetSize());
        m_mmu->RemoveFromEgressAdmission(ifIndex, qIndex, p->GetSize());
        if (m_ecnEnabled){
            bool egressCongested = m_mmu->ShouldSendCN(ifIndex, qIndex);
            if (egressCongested){
                PppHeader ppp;
                Ipv4Header h;
                p->RemoveHeader(ppp);
                p->RemoveHeader(h);
                h.SetEcn((Ipv4Header::EcnType)0x03);
                p->AddHeader(h);
                p->AddHeader(ppp);
            }
        }
        if (m_pfcEnabled) {
            //CheckAndSendPfc(inDev, qIndex);
            CheckAndSendResume(inDev, qIndex);
        }
    }
    if (1){
        uint8_t* buf = p->GetBuffer();
        if (buf[PppHeader::GetStaticSize() + 9] == 0x11){ // udp packet
            IntHeader *ih = (IntHeader*)&buf[PppHeader::GetStaticSize() + 20 + 8 + 6]; // ppp, ip, udp, SeqTs, INT
            Ptr<QbbNetDevice> dev = DynamicCast<QbbNetDevice>(m_devices[ifIndex]);
            if (m_ccMode == 3){ // HPCC
                ih->PushHop(Simulator::Now().GetTimeStep(), m_txBytes[ifIndex], dev->GetQueue()->GetNBytesTotal(), dev->GetDataRate().GetBitRate());
            }else if (m_ccMode == 10){ // HPCC-PINT
                uint64_t t = Simulator::Now().GetTimeStep();
                uint64_t dt = t - m_lastPktTs[ifIndex];
                if (dt > m_maxRtt)
                    dt = m_maxRtt;
                uint64_t B = dev->GetDataRate().GetBitRate() / 8; //Bps
                uint64_t qlen = dev->GetQueue()->GetNBytesTotal();
                double newU;

                /**************************
                 * approximate calc
                 *************************/
                int b = 20, m = 16, l = 20; // see log2apprx's paremeters
                int sft = logres_shift(b,l);
                double fct = 1<<sft; // (multiplication factor corresponding to sft)
                double log_T = log2(m_maxRtt)*fct; // log2(T)*fct
                double log_B = log2(B)*fct; // log2(B)*fct
                double log_1e9 = log2(1e9)*fct; // log2(1e9)*fct
                double qterm = 0;
                double byteTerm = 0;
                double uTerm = 0;
                if ((qlen >> 8) > 0){
                    int log_dt = log2apprx(dt, b, m, l); // ~log2(dt)*fct
                    int log_qlen = log2apprx(qlen >> 8, b, m, l); // ~log2(qlen / 256)*fct
                    qterm = pow(2, (
                                log_dt + log_qlen + log_1e9 - log_B - 2*log_T
                                )/fct
                            ) * 256;
                    // 2^((log2(dt)*fct+log2(qlen/256)*fct+log2(1e9)*fct-log2(B)*fct-2*log2(T)*fct)/fct)*256 ~= dt*qlen*1e9/(B*T^2)
                }
                if (m_lastPktSize[ifIndex] > 0){
                    int byte = m_lastPktSize[ifIndex];
                    int log_byte = log2apprx(byte, b, m, l);
                    byteTerm = pow(2, (
                                log_byte + log_1e9 - log_B - log_T
                                )/fct
                            );
                    // 2^((log2(byte)*fct+log2(1e9)*fct-log2(B)*fct-log2(T)*fct)/fct) ~= byte*1e9 / (B*T)
                }
                if (m_maxRtt > dt && m_u[ifIndex] > 0){
                    int log_T_dt = log2apprx(m_maxRtt - dt, b, m, l); // ~log2(T-dt)*fct
                    int log_u = log2apprx(int(round(m_u[ifIndex] * 8192)), b, m, l); // ~log2(u*512)*fct
                    uTerm = pow(2, (
                                log_T_dt + log_u - log_T
                                )/fct
                            ) / 8192;
                    // 2^((log2(T-dt)*fct+log2(u*512)*fct-log2(T)*fct)/fct)/512 = (T-dt)*u/T
                }
                newU = qterm+byteTerm+uTerm;

                #if 0
                /**************************
                 * accurate calc
                 *************************/
                double weight_ewma = double(dt) / m_maxRtt;
                double u;
                if (m_lastPktSize[ifIndex] == 0)
                    u = 0;
                else{
                    double txRate = m_lastPktSize[ifIndex] / double(dt); // B/ns
                    u = (qlen / m_maxRtt + txRate) * 1e9 / B;
                }
                newU = m_u[ifIndex] * (1 - weight_ewma) + u * weight_ewma;
                printf(" %lf\n", newU);
                #endif

                /************************
                 * update PINT header
                 ***********************/
                uint16_t power = Pint::encode_u(newU);
                if (power > ih->GetPower())
                    ih->SetPower(power);

                m_u[ifIndex] = newU;
            }
        }
    }
    m_txBytes[ifIndex] += p->GetSize();
    m_lastPktSize[ifIndex] = p->GetSize();
    m_lastPktTs[ifIndex] = Simulator::Now().GetTimeStep();
}

int SwitchNode::logres_shift(int b, int l){
    static int data[] = {0,0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5};
    return l - data[b];
}

int SwitchNode::log2apprx(int x, int b, int m, int l){
    int x0 = x;
    int msb = int(log2(x)) + 1;
    if (msb > m){
        x = (x >> (msb - m) << (msb - m));
        #if 0
        x += + (1 << (msb - m - 1));
        #else
        int mask = (1 << (msb-m)) - 1;
        if ((x0 & mask) > (rand() & mask))
            x += 1<<(msb-m);
        #endif
    }
    return int(log2(x) * (1<<logres_shift(b, l)));
}

} /* namespace ns3 */
