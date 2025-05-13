#ifndef QBB_NET_DEVICE_H
#define QBB_NET_DEVICE_H

#include "ns3/point-to-point-net-device.h"

namespace ns3 {

  class QbbNetDevice: public PointToPointNetDevice {
    public:
      /* number of queues/priorities used */
      static const uint32_t qCnt = 8;
      /* max number of flows on a NIC, for TX and RX respectively. TX + RX = fCnt * 2 */
      static const uint32_t fCnt = 128;
      /* number of ports used */
      static const uint32_t pCnt = 64;
      /* max hop count in the network. should not exceed 16 */
      static const uint32_t maxHop = 1;

      static TypeId GetTypeId();
      QbbNetDevice();
      ~QbbNetDevice() override;





    private:
    

    protected:
      bool m_qbbEnabled;
      bool m_qcnEnabled;
      bool m_dynamicth;

      uint32_t m_pausetime;

      double m_qcn_interval;
      double m_g;
      double m_rpgTimeReset;
      double m_alpha_resume_interval;

      uint32_t m_chunk;
      uint32_t m_ack_interval;
      bool     m_waitAck;
      bool     m_backto0;
      bool     m_testRead;

      DataRate m_minRate;
      uint32_t  m_bc;
      DataRate m_rai;
      DataRate m_rhai;
      DataRate m_nextSend;
      uint32_t m_rpgThreshold;

      double m_qcn_np_sampling_interval;
      double m_qcn_np_sampling;
      double m_nack_interval;
      double m_waitAckTimer;

      bool m_EcnClampTgtRate;
      bool m_EcnClampTgtRateAfterTimerInc;

      struct ECNAccount{
        Ipv4Address source;
        uint32_t    qIndex;
        uint32_t    port;
        uint8_t     ecnbits;
        uint16_t    qfb;
        uint16_t    total;
      };

      std::vector<ECNAccount> *m_ecn_source;

      /* queue level */
      /* whether a queue paused */
      bool      m_paused[qCnt];
      /* keeping the next resume event(PFC) */
      EventId   m_resumeEvt[qCnt];

      /* flow level */
      double    m_credits[fCnt];
      Time      m_nextAvail[fCnt];
      uint32_t  m_findex_udpport_map[fCnt];
      uint32_t  m_findex_qindex_map[fCnt];
      bool      m_waitingAck[fCnt];
      /* rate increase event(QCN) */
      EventId   m_rateIncrease[fCnt];


      /* ports level */
      /* 0 means no state, 1 means ingress, 2 means egress */
      uint32_t  m_ECNState[pCnt];
      uint32_t  m_ECNIngressCount[pCnt];
      uint32_t  m_ECNEgressCount[pCnt];

      /* network level */
      int64_t   m_txBytes[fCnt][maxHop];
      double    m_rpWhile[fCnt][maxHop];
      uint32_t  m_rpByteStage[fCnt][maxHop];
      uint32_t  m_rpTimeStage[fCnt][maxHop];
      double    m_alpha[fCnt][maxHop];
      /* 1 fr 2 ai 3 hi */
      uint32_t  m_rpStage[fCnt][maxHop];

      /* keeping the next recheck queue full event(PFC) */
      EventId   m_recheckEvt[pCnt][qCnt];



      






      void DoDispose() override;
      virtual void TransmitComplete();




  };

}


#endif
