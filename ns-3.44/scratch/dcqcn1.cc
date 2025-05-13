#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"


#include <iostream>
#include <fstream>
#include <ranges>
#include <time.h>

std::string insert_blank(int count) {
  std::string result; 
  for(int _: std::views::iota(0, count)) {
    result += "\t";
  }
  return result;
};

template<typename T>
void convert_config(std::ifstream& conf, const std::string& key, T& var, const std::string& str, int blank_count) {
  if(key == str) {
    conf >> var;
    if constexpr(std::is_same_v<T, bool>) {
      std::cout << str << insert_blank(blank_count) << (var ? "Yes" : "No") <<"\n";
    } else {
      std::cout << str << insert_blank(blank_count) << var <<"\n";
    }
  }
}

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("DCQCN1");

int
main(int argc, char* argv[])
{

  bool        enable_qcn = true, use_dynamic_pfc_threshold = true, packet_level_ecmp = false, flow_level_ecmp = false;
  uint32_t    packet_payload_size = 1000, l2_chunk_size = 0, l2_ack_interval = 0;
  double      pause_time = 5, simulator_stop_time =3.01, app_start_time = 1.0, app_stop_time = 9.0;
  std::string data_rate, link_delay, topology_file, flow_file, tcp_flow_file, trace_file, trace_output_file;
  [[maybe_unused]] bool        used_port[65536] = {0};

  double      cnp_interval = 50, alpha_resume_interval = 55, rp_timer, dctcp_gain = 1/16, np_sampling_interval = 0, pmax = 1;
  uint32_t    byte_counter, fast_recovery_times = 5, kmax = 60, kmin = 60;
  std::string rate_ai, rate_hai;

  bool        clamp_target_rate = false, clamp_target_rate_after_timer = false, send_in_chunks = true, l2_wait_for_ack = false;
  bool        l2_back_to_zero = false, l2_test_read = false;
  double      error_rate_per_link = 0.0;

  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  [[maybe_unused]] clock_t begint, endt;
  begint = clock();

  std::cout << "start read config\n";

  // read conf and display
  std::ifstream conf;
  conf.open("scratch/mix/config.txt");
  if(!conf.is_open()) {
    std::cerr << "cann't open config file!" << std::endl;
    return 1;
  } else {
    while(!conf.eof()) {
      std::string key;
      conf >> key;

      convert_config(conf, key, enable_qcn,                     "ENABLE_QCN", 3);
      convert_config(conf, key, use_dynamic_pfc_threshold,      "USE_DYNAMIC_PFC_THRESHOLD", 1);
      convert_config(conf, key, clamp_target_rate,              "CLAMP_TARGET_RATE", 2);
      convert_config(conf, key, clamp_target_rate_after_timer,  "CLAMP_TARGET_RATE_AFTER_TIMER", 1);
      convert_config(conf, key, packet_level_ecmp,              "PACKET_LEVEL_ECMP", 2);
      convert_config(conf, key, flow_level_ecmp,                "FLOW_LEVEL_ECMP", 3);

      convert_config(conf, key, pause_time,                     "PAUSE_TIME", 3);
      convert_config(conf, key, data_rate,                      "DATA_RATE", 3);
      convert_config(conf, key, link_delay,                     "LINK_DELAY", 3);
      convert_config(conf, key, packet_payload_size,            "PACKET_PAYLOAD_SIZE", 2);
      convert_config(conf, key, l2_chunk_size,                  "L2_CHUNK_SIZE", 3);
      convert_config(conf, key, l2_ack_interval,                "L2_ACK_INTERVAL", 3);

      convert_config(conf, key, l2_wait_for_ack,                "L2_WAIT_FOR_ACK", 3);
      convert_config(conf, key, l2_back_to_zero,                "L2_BACK_TO_ZERO", 3);
      convert_config(conf, key, l2_test_read,                   "L2_TEST_READ",    3);

      convert_config(conf, key, topology_file,                  "TOPOLOGY_FILE", 3);
      convert_config(conf, key, flow_file,                      "FLOW_FILE", 3);
      convert_config(conf, key, tcp_flow_file,                  "TCP_FLOW_FILE", 3);
      convert_config(conf, key, trace_file,                     "TRACE_FILE", 3);
      convert_config(conf, key, trace_output_file,              "TRACE_OUTPUT_FILE", 2);

      convert_config(conf, key, app_start_time,                 "APP_START_TIME", 3);
      convert_config(conf, key, app_stop_time,                  "APP_STOP_TIME", 3);
      convert_config(conf, key, simulator_stop_time,            "SIMULATOR_STOP_TIME", 2);
      convert_config(conf, key, cnp_interval,                   "CNP_INTERVAL", 3);
      convert_config(conf, key, alpha_resume_interval,          "ALPHA_RESUME_INTERVAL", 2);
      convert_config(conf, key, rp_timer,                       "RP_TIMER", 3);
      convert_config(conf, key, byte_counter,                   "BYTE_COUNTER", 3);

      convert_config(conf, key, kmax,                           "KMAX", 4);
      convert_config(conf, key, kmin,                           "KMIN", 4);
      convert_config(conf, key, pmax,                           "PMAX", 4);

      convert_config(conf, key, dctcp_gain,                     "DCTCP_GAIN", 3);
      convert_config(conf, key, fast_recovery_times,            "FAST_RECOVERY_TIMES", 2);

      convert_config(conf, key, rate_ai,                        "RATE_AI", 4);
      convert_config(conf, key, rate_hai,                       "RATE_HAI", 3);

      convert_config(conf, key, np_sampling_interval,           "NP_SAMPLING_INTERVAL", 2);
      convert_config(conf, key, send_in_chunks,                 "SEND_IN_CHUNKS", 3);
      convert_config(conf, key, error_rate_per_link,            "ERROR_RATE_PER_LINK", 2);
      fflush(stdout);
    }
    conf.close();
  }

  [[maybe_unused]]bool dynamicth = use_dynamic_pfc_threshold;

  NS_ASSERT(packet_level_ecmp + flow_level_ecmp < 2);
  Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(packet_level_ecmp));
  Config::SetDefault("ns3::Ipv4GlobalRouting::FlowEcmpRouting", BooleanValue(flow_level_ecmp));

  Config::SetDefault("ns3::QbbNetDevice::PauseTime", UintegerValue(pause_time));
  Config::SetDefault("ns3::QbbNetDevice::QcnEnabled", BooleanValue(enable_qcn));
  Config::SetDefault("ns3::QbbNetDevice::DynamicThreshold", BooleanValue(dynamicth));
  Config::SetDefault("ns3::QbbNetDevice::ClampTargetRate", BooleanValue(clamp_target_rate));
  Config::SetDefault("ns3::QbbNetDevice::ClampTargetRateAfterTimeInc", BooleanValue(clamp_target_rate_after_timer));
  Config::SetDefault("ns3::QbbNetDevice::CNPInterval", DoubleValue(cnp_interval));
  Config::SetDefault("ns3::QbbNetDevice::NPSamplingInterval", DoubleValue(np_sampling_interval));
  Config::SetDefault("ns3::QbbNetDevice::AlphaResumInterval", DoubleValue(alpha_resume_interval));
  Config::SetDefault("ns3::QbbNetDevice::RPTimer", DoubleValue(rp_timer));
  Config::SetDefault("ns3::QbbNetDevice::ByteCounter", UintegerValue(byte_counter));
  Config::SetDefault("ns3::QbbNetDevice::FastRecoveryTimes", UintegerValue(fast_recovery_times));
  Config::SetDefault("ns3::QbbNetDevice::DCTCPGain", DoubleValue(dctcp_gain));
  Config::SetDefault("ns3::QbbNetDevice::RateAI", DataRateValue(DataRate(rate_ai)));
  Config::SetDefault("ns3::QbbNetDevice::RateHAI", DataRateValue(DataRate(rate_hai)));
  Config::SetDefault("ns3::QbbNetDevice::L2BackToZero", BooleanValue(l2_back_to_zero));
  Config::SetDefault("ns3::QbbNetDevice::L2TestRead", BooleanValue(l2_test_read));
  Config::SetDefault("ns3::QbbNetDevice::L2ChunkSize", UintegerValue(l2_chunk_size));
  Config::SetDefault("ns3::QbbNetDevice::L2AckInterval", UintegerValue(l2_ack_interval));
  Config::SetDefault("ns3::QbbNetDevice::L2WaitForAck", BooleanValue(l2_wait_for_ack));









  Time::SetResolution(Time::NS);
  LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create(2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install(nodes);

  InternetStackHelper stack;
  stack.Install(nodes);

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  UdpEchoServerHelper echoServer(9);

  ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
  serverApps.Start(Seconds(1));
  serverApps.Stop(Seconds(10));

  UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
  echoClient.SetAttribute("MaxPackets", UintegerValue(1));
  echoClient.SetAttribute("Interval", TimeValue(Seconds(1)));
  echoClient.SetAttribute("PacketSize", UintegerValue(1024));

  ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
  clientApps.Start(Seconds(2));
  clientApps.Stop(Seconds(10));

  Simulator::Run();
  Simulator::Destroy();
  return 0;

}
