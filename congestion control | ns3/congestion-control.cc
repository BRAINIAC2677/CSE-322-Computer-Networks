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

#include <string>
using namespace std;

int
main(int args, char* argv[])
{
    bool verbose = false;
    uint32_t port = 2677;
    uint32_t nLeftLeaf = 2;
    uint32_t nRightLeaf = 2;
    uint64_t payloadSize = 1024;                // in bytes
    uint64_t leafDataRate = 1024 * 1024 * 1024; // in bps(1Gbps)
    uint64_t bottleneckDataRate = 1024 * 1024;  // in bps(1Mbps)
    uint64_t leafDelay = 1;                     // in ms
    uint64_t bottleneckDelay = 100;             // in ms
    double packetLossRate = 0.000001;
    double waitingTime = 5;
    double senderSimulationStartTime = 1.0;
    double senderSimulationStopTime = 2;
    double receiverSimulationStartTime = 0.0;
    double receiverSimulationStopTime = senderSimulationStopTime + waitingTime;
    string tcpVariant1 = "ns3::TcpNewReno";
    string tcpVariant2 = "ns3::TcpWestwoodPlus";

    // results
    double throughput1 = 0;
    double throughput2 = 0;

    CommandLine cmd;
    cmd.AddValue("verbose", "turn on log components", verbose);
    cmd.AddValue("bottleneckDataRate", "bottleneck data rate", bottleneckDataRate);
    cmd.AddValue("packetLossRate", "packet loss rate", packetLossRate);
    cmd.Parse(args, argv);

    if (verbose)
    {
        LogComponentEnable("OnOffApplication", LOG_LEVEL_DEBUG);
        LogComponentEnable("PacketSink", LOG_LEVEL_DEBUG);
    }

    uint64_t bandwidthDelayProduct = bottleneckDataRate * (bottleneckDelay / 1000);

    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize));

    PointToPointHelper leftHelper, rightHelper, bottleneckHelper;
    leftHelper.SetDeviceAttribute("DataRate", StringValue(to_string(leafDataRate) + "bps"));
    leftHelper.SetChannelAttribute("Delay", StringValue(to_string(leafDelay) + "ms"));
    leftHelper.SetQueue("ns3::DropTailQueue",
                        "MaxSize",
                        StringValue(to_string(bandwidthDelayProduct) + "p"));
    rightHelper.SetDeviceAttribute("DataRate", StringValue(to_string(leafDataRate) + "bps"));
    rightHelper.SetChannelAttribute("Delay", StringValue(to_string(leafDelay) + "ms"));
    rightHelper.SetQueue("ns3::DropTailQueue",
                         "MaxSize",
                         StringValue(to_string(bandwidthDelayProduct) + "p"));
    bottleneckHelper.SetDeviceAttribute("DataRate",
                                        StringValue(to_string(bottleneckDataRate) + "bps"));
    bottleneckHelper.SetChannelAttribute("Delay", StringValue(to_string(bottleneckDelay) + "ms"));
    PointToPointDumbbellHelper dumbbellHelper(nLeftLeaf,
                                              leftHelper,
                                              nRightLeaf,
                                              rightHelper,
                                              bottleneckHelper);

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

    dumbbellHelper.AssignIpv4Addresses(Ipv4AddressHelper("10.1.1.0", "255.255.255.0"),
                                       Ipv4AddressHelper("10.2.1.0", "255.255.255.0"),
                                       Ipv4AddressHelper("10.3.1.0", "255.255.255.0"));

    Ptr<RateErrorModel> errorModel = CreateObject<RateErrorModel>();
    errorModel->SetAttribute("ErrorRate", DoubleValue(packetLossRate));
    // todo: think about the following line
    dumbbellHelper.m_routerDevices.Get(1)->SetAttribute("ReceiveErrorModel",
                                                        PointerValue(errorModel));

    ApplicationContainer senderApps, receiverApps;

    OnOffHelper onOffHelper("ns3::TcpSocketFactory", Address());
    onOffHelper.SetAttribute("PacketSize", UintegerValue(payloadSize));
    onOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    onOffHelper.SetAttribute("DataRate",
                             DataRateValue(DataRate(to_string(bottleneckDataRate) + "bps")));
    for (uint32_t i = 0; i < dumbbellHelper.LeftCount(); ++i)
    {
        AddressValue remoteAddress(InetSocketAddress(dumbbellHelper.GetRightIpv4Address(i), port));
        onOffHelper.SetAttribute("Remote", remoteAddress);
        senderApps.Add(onOffHelper.Install(dumbbellHelper.GetLeft(i)));
    }
    senderApps.Start(Seconds(senderSimulationStartTime));
    senderApps.Stop(Seconds(senderSimulationStopTime));

    Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);
    for (uint32_t i = 0; i < dumbbellHelper.RightCount(); ++i)
    {
        receiverApps.Add(packetSinkHelper.Install(dumbbellHelper.GetRight(i)));
    }
    receiverApps.Start(Seconds(receiverSimulationStartTime));
    receiverApps.Stop(Seconds(receiverSimulationStopTime));

    FlowMonitorHelper flowMonitorHelper;
    // flowMonitorHelper.SetMonitorAttribute("MaxPerHopDelay", DoubleValue(waitingTime));
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
        for (uint32_t i = 0; i < dumbbellHelper.LeftCount(); ++i)
        {
            if (dumbbellHelper.GetLeftIpv4Address(i) == fiveTuple.sourceAddress)
            {
                throughput1 += ((i & 1) ? 0 : it->second.rxBytes * 8);
                throughput2 += ((i & 1) ? it->second.rxBytes * 8 : 0);
                break;
            }
        }
    }
    throughput1 /= (receiverSimulationStopTime - receiverSimulationStartTime) * 1000000; // in Mbps
    throughput2 /= (receiverSimulationStopTime - receiverSimulationStartTime) * 1000000; // in Mbps
    Simulator::Destroy();

    cout << "throughput1: " << throughput1 << endl;
    cout << "throughput2: " << throughput2 << endl;

    return 0;
}
