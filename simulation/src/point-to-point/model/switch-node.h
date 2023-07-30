#ifndef SWITCH_NODE_H
#define SWITCH_NODE_H

#include <unordered_map>
#include <ns3/node.h>
#include "qbb-net-device.h"
#include "switch-mmu.h"
#include "pint.h"

namespace ns3 {

class Packet;

class SwitchNode : public Node{
    static const uint32_t pCnt = 257;   // Number of ports used
    static const uint32_t qCnt = 8; // Number of queues/priorities used

public:
    static const uint32_t slotM = 25; // PBT: Number of register slots
    static const uint32_t slotN = 16; // PBT: Number of time slots by which we divide the RTT (2N in the paper)
    static const uint32_t dc_max_rtt = 24960; // PBT: Data Center Max Round Trip Time (nanoseconds)
    static const uint32_t slot_len = 512; // PBT: length of the register slot (nanoseconds)
    static const uint32_t accuracy = 32;  // PBT: accuracy constant used to divide register slot into segments
    static const uint32_t overheadSlotNum = 1; // PBT: extra register slots so that when a packet is handled all slots in the last RTT are correct
    static const uint32_t slotTot = slotM + overheadSlotNum; // PBT: total number of register slots
    static const uint32_t w = slotN / 2; // PBT: Number of future time slots to predict
    static const uint32_t marked_win_tx_time = 1920;

private:
    uint64_t loss_count_end_time;
    uint32_t m_ecmpSeed;
    std::unordered_map<uint32_t, std::vector<int> > m_rtTable; // map from ip address (u32) to possible ECMP port (index of dev)
    
    uint64_t m_txBytes[pCnt]; // counter of tx bytes

    int64_t m_rxBytesSlots[pCnt][slotTot]; // PBT: counter of rx bytes in the last RTT
    int64_t m_rxMarkedBytesSlots[pCnt][slotTot]; // PBT: counter of rx marked bytes in the last RTT
    uint32_t m_queueSizes[pCnt][slotTot]; // PBT: queue size at the start of each slot (in bytes)
    uint32_t m_currentSlot; // PBT: current register slot number
    uint32_t m_previousSlot; // PBT: previous register slot number
    uint64_t m_slotStartTs; // PBT: timestamp for the start of the current register slot

    uint32_t m_lastPktSize[pCnt];
    uint64_t m_lastPktTs[pCnt]; // ns
    double m_u[pCnt];

protected:
    bool m_ecnEnabled;
    bool m_pbtEnabled;
    bool m_pfcEnabled;
    uint32_t m_ccMode;
    uint64_t m_maxRtt;

    uint32_t m_ackHighPrio; // set high priority for ACK/NACK

private:
    int GetOutDev(Ptr<const Packet>, CustomHeader &ch);
    void SendToDev(Ptr<Packet>p, CustomHeader &ch);
    static uint32_t EcmpHash(const uint8_t* key, size_t len, uint32_t seed);
    void CheckAndSendPfc(uint32_t inDev, uint32_t qIndex);
    void CheckAndSendResume(uint32_t inDev, uint32_t qIndex);
    void CheckAndSendPBT(uint32_t inDev, uint32_t outDev, Ptr<Packet> p);

    int64_t GetIntervalBytes(uint32_t interval, uint32_t slot_r, int64_t *byte_slots, uint32_t *last_slot_idx);
public:
    Ptr<SwitchMmu> m_mmu;
    static uint64_t tot_num_drops;
    EventId slotResetEvent;

    static TypeId GetTypeId (void);
    SwitchNode();
    void SetEcmpSeed(uint32_t seed);
    void AddTableEntry(Ipv4Address &dstAddr, uint32_t intf_idx);
    void ClearTable();
    bool SwitchReceiveFromDevice(Ptr<NetDevice> device, Ptr<Packet> packet, CustomHeader &ch);
    void SwitchNotifyDequeue(uint32_t ifIndex, uint32_t qIndex, Ptr<Packet> p);
    void SlotReset(void);
    void ScheduleSlotReset(uint64_t startTime);

    // for approximate calc in PINT
    int logres_shift(int b, int l);
    int log2apprx(int x, int b, int m, int l); // given x of at most b bits, use most significant m bits of x, calc the result in l bits
};

} /* namespace ns3 */

#endif /* SWITCH_NODE_H */
