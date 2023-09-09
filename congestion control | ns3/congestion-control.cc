/*
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
 */

#include "packet-source.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/error-model.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-dumbbell.h"
#include "ns3/point-to-point-module.h"
using namespace ns3;

#include <fstream>
#include <string>
using namespace std;

ofstream cwndOutputFileStream1;
ofstream cwndOutputFileStream2;

static void
CwndTracedValueCallback1(uint32_t oldValue, uint32_t newValue)
{
    cwndOutputFileStream1 << Simulator::Now().GetSeconds() << "\t" << newValue << endl;
}

static void
CwndTracedValueCallback2(uint32_t oldValue, uint32_t newValue)
{
    cwndOutputFileStream2 << Simulator::Now().GetSeconds() << "\t" << newValue << endl;
}

int
main(int args, char* argv[])
{
    // inputs from cmd
    bool verbose = false;
    int packetLossExp = -1;
    uint64_t bottleneckDataRate = 1; // in Mbps
    string tcpVariant1 = "ns3::TcpNewReno";
    string tcpVariant2 = "ns3::TcpHighSpeed";
    string metricOutputFile = "simulation_metrics.csv";
    string cwndOutputFile1 = "cwnd1.txt";
    string cwndOutputFile2 = "cwnd2.txt";

    CommandLine cmd;
    cmd.AddValue("verbose", "turn on log components", verbose);
    cmd.AddValue("bottleneckDataRate", "bottleneck data rate", bottleneckDataRate);
    cmd.AddValue("packetLossExp", "packet loss exponent", packetLossExp);
    cmd.AddValue("tcpVariant1", "tcp variant 1", tcpVariant1);
    cmd.AddValue("tcpVariant2", "tcp variant 2", tcpVariant2);
    cmd.AddValue("metricOutputFile", "metric output file", metricOutputFile);
    cmd.AddValue("cwndOutputFile1", "cwnd output file 1", cwndOutputFile1);
    cmd.AddValue("cwndOutputFile2", "cwnd output file 2", cwndOutputFile2);
    cmd.Parse(args, argv);

    // config constants
    uint32_t port = 2677;
    uint32_t nLeftLeaf = 2;
    uint32_t nRightLeaf = 2;
    uint64_t payloadSize = 1440;    // in bytes
    uint64_t leafDataRate = 1024;   // in Mbps(1Gbps)
    uint64_t leafDelay = 1;         // in ms
    uint64_t bottleneckDelay = 100; // in ms
    double waitingTime = 5;
    double senderSimulationStartTime = 1.0;
    double senderSimulationStopTime = 20;
    double receiverSimulationStartTime = 0.0;
    double receiverSimulationStopTime = senderSimulationStopTime + waitingTime;

    // results metrics
    uint64_t txPackets1 = 0;
    uint64_t txPackets2 = 0;
    uint64_t rxPackets1 = 0;
    uint64_t rxPackets2 = 0;
    uint64_t txAckPackets1 = 0;
    uint64_t txAckPackets2 = 0;
    uint64_t rxAckPackets1 = 0;
    uint64_t rxAckPackets2 = 0;
    double throughput1 = 0;
    double throughput2 = 0;
    double jainIndexNum = 0;
    double jainIndexDen = 0;
    double jainIndex = 0;

    if (verbose)
    {
        LogComponentEnable("PacketSink", LOG_LEVEL_DEBUG);
    }

    ofstream metricOutputFileStream(metricOutputFile, ios::app);
    cwndOutputFileStream1.open(cwndOutputFile1);
    cwndOutputFileStream2.open(cwndOutputFile2);

    uint64_t bandwidthDelayProduct = 1024 * 1024 * bottleneckDataRate * (bottleneckDelay / 1000);
    uint64_t nPackets = (senderSimulationStopTime - senderSimulationStartTime) * leafDataRate *
                        1024 * 1024 / (payloadSize * 8);
    double packetLossRate;
    if (packetLossExp < 0)
    {
        packetLossRate = 1.0 / pow(10, -packetLossExp);
    }
    else
    {
        packetLossRate = pow(10, packetLossExp);
    }

    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize));

    PointToPointHelper leftHelper, rightHelper, bottleneckHelper;
    leftHelper.SetDeviceAttribute("DataRate", StringValue(to_string(leafDataRate) + "Mbps"));
    leftHelper.SetChannelAttribute("Delay", StringValue(to_string(leafDelay) + "ms"));
    leftHelper.SetQueue("ns3::DropTailQueue",
                        "MaxSize",
                        StringValue(to_string(bandwidthDelayProduct) + "p"));
    rightHelper.SetDeviceAttribute("DataRate", StringValue(to_string(leafDataRate) + "Mbps"));
    rightHelper.SetChannelAttribute("Delay", StringValue(to_string(leafDelay) + "ms"));
    rightHelper.SetQueue("ns3::DropTailQueue",
                         "MaxSize",
                         StringValue(to_string(bandwidthDelayProduct) + "p"));
    bottleneckHelper.SetDeviceAttribute("DataRate",
                                        StringValue(to_string(bottleneckDataRate) + "Mbps"));
    bottleneckHelper.SetChannelAttribute("Delay", StringValue(to_string(bottleneckDelay) + "ms"));
    PointToPointDumbbellHelper dumbbellHelper(nLeftLeaf,
                                              leftHelper,
                                              nRightLeaf,
                                              rightHelper,
                                              bottleneckHelper);
    // install internet stack
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue(tcpVariant1));
    InternetStackHelper internetStackHelper1;
    for (uint32_t i = 0; i < dumbbellHelper.LeftCount(); i += 2)
    {
        internetStackHelper1.Install(dumbbellHelper.GetLeft(i));
    }
    for (uint32_t i = 0; i < dumbbellHelper.RightCount(); i += 2)
    {
        internetStackHelper1.Install(dumbbellHelper.GetRight(i));
    }
    // todo: think about the following two lines
    internetStackHelper1.Install(dumbbellHelper.GetLeft());
    internetStackHelper1.Install(dumbbellHelper.GetRight());

    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue(tcpVariant2));
    InternetStackHelper internetStackHelper2;
    for (uint32_t i = 1; i < dumbbellHelper.LeftCount(); i += 2)
    {
        internetStackHelper2.Install(dumbbellHelper.GetLeft(i));
    }
    for (uint32_t i = 1; i < dumbbellHelper.RightCount(); i += 2)
    {
        internetStackHelper2.Install(dumbbellHelper.GetRight(i));
    }

    // assign ipv4 addresses
    dumbbellHelper.AssignIpv4Addresses(Ipv4AddressHelper("10.1.1.0", "255.255.255.0"),
                                       Ipv4AddressHelper("10.2.1.0", "255.255.255.0"),
                                       Ipv4AddressHelper("10.3.1.0", "255.255.255.0"));

    // install error model
    Ptr<RateErrorModel> errorModel = CreateObject<RateErrorModel>();
    errorModel->SetAttribute("ErrorRate", DoubleValue(packetLossRate));
    // todo: think about the following line
    dumbbellHelper.m_routerDevices.Get(1)->SetAttribute("ReceiveErrorModel",
                                                        PointerValue(errorModel));

    // install applications at sender nodes
    ApplicationContainer senderApps, receiverApps;
    Ptr<PacketSource> packetSource = CreateObject<PacketSource>();
    for (uint32_t i = 0; i < dumbbellHelper.LeftCount(); ++i)
    {
        Address remoteAddress(InetSocketAddress(dumbbellHelper.GetRightIpv4Address(i), port));
        Ptr<Socket> ns3TcpSocket =
            Socket::CreateSocket(dumbbellHelper.GetLeft(i), TcpSocketFactory::GetTypeId());
        Ptr<PacketSource> packetSource = CreateObject<PacketSource>();
        packetSource->Setup(ns3TcpSocket,
                            remoteAddress,
                            payloadSize,
                            nPackets,
                            DataRate(to_string(bottleneckDataRate) + "Mbps"));
        dumbbellHelper.GetLeft(i)->AddApplication(packetSource);
        senderApps.Add(packetSource);
        if (i % 2 == 0)
        {
            ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow",
                                                     MakeCallback(&CwndTracedValueCallback1));
        }
        else
        {
            ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow",
                                                     MakeCallback(&CwndTracedValueCallback2));
        }
    }
    senderApps.Start(Seconds(senderSimulationStartTime));
    senderApps.Stop(Seconds(senderSimulationStopTime));

    // install applications at receiver nodes
    Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);
    for (uint32_t i = 0; i < dumbbellHelper.RightCount(); ++i)
    {
        receiverApps.Add(packetSinkHelper.Install(dumbbellHelper.GetRight(i)));
    }
    receiverApps.Start(Seconds(receiverSimulationStartTime));
    receiverApps.Stop(Seconds(receiverSimulationStopTime));

    // setting up flow monitor
    FlowMonitorHelper flowMonitorHelper;
    flowMonitorHelper.SetMonitorAttribute("MaxPerHopDelay", TimeValue(Seconds(waitingTime)));
    Ptr<FlowMonitor> flowMonitor = flowMonitorHelper.InstallAll();
    Ptr<Ipv4FlowClassifier> flowClassifier =
        DynamicCast<Ipv4FlowClassifier>(flowMonitorHelper.GetClassifier());

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Simulator::Stop(Seconds(receiverSimulationStopTime));
    Simulator::Run();

    // calculate throughput1 and throughput2 from flow monitor
    FlowMonitor::FlowStatsContainer flowStatsContainer = flowMonitor->GetFlowStats();
    for (auto it = flowStatsContainer.begin(); it != flowStatsContainer.end(); ++it)
    {
        Ipv4FlowClassifier::FiveTuple fiveTuple = flowClassifier->FindFlow(it->first);
        double currentThroughput = it->second.rxBytes * 8 /
                                   (receiverSimulationStopTime - receiverSimulationStartTime) /
                                   1000; // in Kbps
        jainIndexNum += currentThroughput;
        jainIndexDen += pow(currentThroughput, 2);

        for (uint32_t i = 0; i < dumbbellHelper.LeftCount(); ++i)
        {
            if (dumbbellHelper.GetLeftIpv4Address(i) == fiveTuple.sourceAddress)
            {
                txPackets1 += ((i & 1) ? 0 : it->second.txPackets);
                txPackets2 += ((i & 1) ? it->second.txPackets : 0);
                rxPackets1 += ((i & 1) ? 0 : it->second.rxPackets);
                rxPackets2 += ((i & 1) ? it->second.rxPackets : 0);
                throughput1 += ((i & 1) ? 0 : it->second.rxBytes * 8);
                throughput2 += ((i & 1) ? it->second.rxBytes * 8 : 0);
                break;
            }
            if (dumbbellHelper.GetLeftIpv4Address(i) == fiveTuple.destinationAddress)
            {
                txAckPackets1 += ((i & 1) ? 0 : it->second.txPackets);
                txAckPackets2 += ((i & 1) ? it->second.txPackets : 0);
                rxAckPackets1 += ((i & 1) ? 0 : it->second.rxPackets);
                rxAckPackets2 += ((i & 1) ? it->second.rxPackets : 0);
                throughput1 += ((i & 1) ? it->second.rxBytes * 8 : 0);
                throughput2 += ((i & 1) ? 0 : it->second.rxBytes * 8);
                break;
            }
        }
    }
    throughput1 /= (receiverSimulationStopTime - receiverSimulationStartTime) * 1000; // in Kbps
    throughput2 /= (receiverSimulationStopTime - receiverSimulationStartTime) * 1000; // in Kbps
    jainIndex = pow(jainIndexNum, 2) / (jainIndexDen * flowStatsContainer.size());
    Simulator::Destroy();

    if (metricOutputFileStream.tellp() == 0)
    {
        metricOutputFileStream << "Sender Simulation Start Time (s),"
                               << "Sender Simulation Stop Time (s),"
                               << "Receiver Simulation Start Time (s),"
                               << "Receiver Simulation Stop Time (s),"
                               << "Bottle Neck Data Rate (Mbps),"
                               << "Packet Loss Exponent,"
                               << "TCP Variant 1,"
                               << "TCP Variant 2,"
                               << "Tx Packets 1,"
                               << "Tx Packets 2,"
                               << "Rx Packets 1,"
                               << "Rx Packets 2,"
                               << "Tx Ack Packets 1,"
                               << "Tx Ack Packets 2,"
                               << "Rx Ack Packets 1,"
                               << "Rx Ack Packets 2,"
                               << "Throughput 1 (Mbps),"
                               << "Throughput 2 (Mbps),"
                               << "Jain Index" << endl;
    }

    metricOutputFileStream << senderSimulationStartTime << "," << senderSimulationStopTime << ","
                           << receiverSimulationStartTime << "," << receiverSimulationStopTime
                           << "," << bottleneckDataRate << "," << packetLossExp << ","
                           << tcpVariant1 << "," << tcpVariant2 << "," << txPackets1 << ","
                           << txPackets2 << "," << rxPackets1 << "," << rxPackets2 << ","
                           << txAckPackets1 << "," << txAckPackets2 << "," << rxAckPackets1 << ","
                           << rxAckPackets2 << "," << throughput1 << "," << throughput2 << ","
                           << jainIndex << endl;
    return 0;
}
