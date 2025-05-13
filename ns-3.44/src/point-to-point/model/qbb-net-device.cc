#include <stdint.h>
#include <stdio.h>

#include "ns3/core-module.h"
#include "ns3/qbb-net-device.h"

#include <iostream>

NS_LOG_COMPONENT_DEFINE("QbbNetDevice");


namespace ns3 {

  NS_OBJECT_ENSURE_REGISTERED(QbbNetDevice);

#define ADDATTRIBUTE(var, str, type, var1, var2, type1) \
  .AddAttribute(#var, str, type##Value(var1), \
      Make##type##Accessor(&QbbNetDevice::var2), Make##type##Checker<type1>())

#define ADDBOOLATTRIBUTE(var, str, type, var1, var2) \
  .AddAttribute(#var, str, type##Value(var1), \
      Make##type##Accessor(&QbbNetDevice::var2), Make##type##Checker())

  TypeId QbbNetDevice::GetTypeId() {
    static TypeId tid = TypeId("ns3::QbbNetDevice")
      .SetParent<PointToPointNetDevice>()
      .AddConstructor<QbbNetDevice>()

      ADDBOOLATTRIBUTE(QbbEnabled, "Enable the generation of PAUSE packet", Boolean, true, m_qbbEnabled)
      ADDBOOLATTRIBUTE(QcnEnabled, "Enable the generation of PAUSE packet", Boolean, false, m_qcnEnabled)
      ADDBOOLATTRIBUTE(DynamicThreshold, "Enable dynamic threshold", Boolean, false, m_dynamicth)
      ADDBOOLATTRIBUTE(ClampTargetRate, "Clamp target rate", Boolean, false, m_EcnClampTgtRate)
      ADDBOOLATTRIBUTE(ClampTargetRateAfterTimeInc, "Clamp target rate after timer increase", Boolean, false, m_EcnClampTgtRateAfterTimerInc)
      ADDBOOLATTRIBUTE(L2BackToZero, "Layer 2 go back to zero transmission", Boolean, false, m_backto0)
      ADDBOOLATTRIBUTE(L2TestRead, "Layer 2 test read go back to 0 but NACK from n", Boolean, false, m_testRead)
      ADDBOOLATTRIBUTE(L2WaitForAck, "Wait for Ack before sending out next message", Boolean, false, m_waitAck)
      ADDBOOLATTRIBUTE(MinRate, "Minimum rate of a throttled flow", DataRate, DataRate("100Mb/s"), m_minRate)
      ADDBOOLATTRIBUTE(RateAI, "Rate increment unit in AI period", DataRate, DataRate("5Mb/s"), m_rai)
      ADDBOOLATTRIBUTE(RateHAI, "Rate increment unit in hyperactive AI period", DataRate, DataRate("50Mb/s"), m_rhai)

      ADDATTRIBUTE(PauseTime, "Number of microseconds to pause upon congestion", Uinteger, 5, m_pausetime, uint32_t)
      ADDATTRIBUTE(CNPInterval, "The interval of generating CNP", Double, 50.0, m_qcn_interval, double)
      ADDATTRIBUTE(AlphaResumInterval, "The interval of resuming alpha", Double, 55.0, m_alpha_resume_interval, double)
      ADDATTRIBUTE(RPTimer, "The rate increase timer at RP in microseconds", Double, 1500.0, m_rpgTimeReset, double)
      ADDATTRIBUTE(FastRecoveryTimes, "The rate increase timer at RP", Uinteger, 5, m_rpgThreshold, uint32_t)
      ADDATTRIBUTE(DCTCPGain, "control gain parameter which determines the level of rate decrease", Double, 1.0/16, m_g, double)
      ADDATTRIBUTE(ByteCounter, "Byte counter constant for increment process", Uinteger, 150000, m_bc, uint32_t)
      ADDATTRIBUTE(NPSamplingInterval, "The QCN NP sampling interval", Double, 0.0, m_qcn_np_sampling_interval, double)
      ADDATTRIBUTE(NackGenerationInterval, "The NACK Generation interval", Double, 500.0, m_nack_interval, double)
      ADDATTRIBUTE(L2ChunkSize, "Layer 2 chunk size. Disable chunk mode if equals to 0", Uinteger, 0, m_chunk, uint32_t)
      ADDATTRIBUTE(L2AckInterval, "Layer 2 ack intervals. Disable ack if equals to 0", Uinteger, 0, m_ack_interval, uint32_t)
      ADDATTRIBUTE(L2WaitForAckTimer, "Sender's timer of waiting for the ack", Double, 500.0, m_waitAckTimer, double)
    ;
    return tid;
  }

  QbbNetDevice::QbbNetDevice() {
    NS_LOG_FUNCTION(this);
    m_ecn_source = new std::vector<ECNAccount>;
    for(uint32_t i = 0; i < qCnt; i++) {
      m_paused[i] = false;
    }
    m_qcn_np_sampling = 0;
    for(uint32_t i = 0; i < fCnt; i++) {
      m_credits[i] = 0;
      m_nextAvail[i] = Time(0);
      m_findex_udpport_map[i] = 0;
      m_findex_qindex_map[i] = 0;
      m_waitingAck[i] = false;
      for(uint32_t j = 0; j < maxHop; j++) {
        m_txBytes[i][j] = m_bc;
        m_rpWhile[i][j] = m_rpgTimeReset;
        m_rpByteStage[i][j] = 0;
        m_rpTimeStage[i][j] = 0;
        m_alpha[i][j] = 0.5;
        m_rpStage[i][j] = 0;
      }
    }
    for(uint32_t i = 0; i < pCnt; i++){
        m_ECNState[i] = 0;
        m_ECNIngressCount[i] = 0;
        m_ECNEgressCount[i] = 0;
    }
  }

  QbbNetDevice::~QbbNetDevice() {
    NS_LOG_FUNCTION(this);
  }

  void QbbNetDevice::DoDispose(){
    NS_LOG_FUNCTION(this);
    // cancle all the qbb events
    for(uint32_t i = 0; i < qCnt; i++){
      Simulator::Cancel(m_resumeEvt[i]);
    }
    for(uint32_t i = 0; i < fCnt; i++){
      Simulator::Cancel(m_rateIncrease[i]);
    }
    for(uint32_t i = 0; i < pCnt; i++){
      for(uint32_t j = 0; j < qCnt; j++){
        Simulator::Cancel(m_recheckEvt[i][j]);
      }
    }
    Object::DoDispose();
  }

  void QbbNetDevice::TransmitComplete(){}
























}


