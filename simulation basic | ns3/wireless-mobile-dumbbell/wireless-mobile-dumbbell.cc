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
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

uint64_t totalTxPackets = 0;
uint64_t totalRxPackets = 0;

void
TxTracedCallback(Ptr<const Packet> _packet)
{
    totalTxPackets++;
}

void
RxTracedCallback(Ptr<const Packet> _packet, const Address& _address)
{
    totalRxPackets++;
}

double
CalculateThroughput(uint64_t _totalRxPackets, uint64_t _packetSize, double _simulationTime)
{
    // Mbit/s
    return (_totalRxPackets * _packetSize * 8) / (_simulationTime * 1000000.0);
}

double
PacketDeliveryRatio(uint64_t _totalTxPackets, uint64_t _totalRxPackets)
{
    if (_totalTxPackets == 0)
    {
        return 0;
    }
    return (double)_totalRxPackets / (double)_totalTxPackets;
}

int
main(int argc, char* argv[])
{
    bool Verbose = true;
    uint64_t NumberOfNodes = 10;
    uint64_t NumberOfFlows = 10;
    uint64_t PacketSize = 1024;
    uint64_t NumberOfPacketsPerSecond = 100;
    double SpeedOfNodes = 5;
    double CoverageArea = 1;
    double SimulationTime = 1.09;
    std::string MetricOutputFile = "simulation_metric.csv";

    double txRange = 5;

    CommandLine cmd(__FILE__);
    cmd.AddValue("Verbose", "Verbose", Verbose);
    cmd.AddValue("NumberOfNodes", "Number of nodes", NumberOfNodes);
    cmd.AddValue("NumberOfFlows", "Number of flows", NumberOfFlows);
    cmd.AddValue("PacketSize", "Packet size", PacketSize);
    cmd.AddValue("NumberOfPacketsPerSecond",
                 "Number of packets per second",
                 NumberOfPacketsPerSecond);
    cmd.AddValue("SpeedOfNodes", "Speed of nodes", SpeedOfNodes);
    cmd.AddValue("CoverageArea", "Coverage area", CoverageArea);
    cmd.AddValue("SimulationTime", "Simulation time", SimulationTime);
    cmd.AddValue("MetricOutputFile", "Metric output file", MetricOutputFile);
    cmd.Parse(argc, argv);

    if (Verbose)
    {
        LogComponentEnable("OnOffApplication", LOG_LEVEL_DEBUG);
        LogComponentEnable("PacketSink", LOG_LEVEL_DEBUG);
        // LogComponentEnable("YansWifiPhy", LOG_LEVEL_DEBUG);
    }

    CoverageArea *= txRange;
    uint64_t calcDataRate = PacketSize * NumberOfPacketsPerSecond * 8;
    std::string dataRate = std::to_string(calcDataRate) + "bps";

    std::ofstream metricOutputFileStream(MetricOutputFile, std::ios::app);

    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(PacketSize));

    NodeContainer bottleneckNodes;
    bottleneckNodes.Create(2);

    PointToPointHelper pointToPointHelper;
    pointToPointHelper.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPointHelper.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer bottleneckDevices;
    bottleneckDevices = pointToPointHelper.Install(bottleneckNodes);

    NodeContainer senderStaNodes;
    senderStaNodes.Create(NumberOfNodes);
    NodeContainer senderApNodes = bottleneckNodes.Get(0);

    NodeContainer receiverStaNodes;
    receiverStaNodes.Create(NumberOfNodes);
    NodeContainer receiverApNodes = bottleneckNodes.Get(1);

    YansWifiChannelHelper wifiChannelHelper = YansWifiChannelHelper::Default();
    YansWifiPhyHelper wifiPhyHelper;
    wifiPhyHelper.SetChannel(wifiChannelHelper.Create());

    WifiMacHelper wifiMacHelper;
    Ssid senderSsid = Ssid("sender-wifi");
    Ssid receiverSsid = Ssid("receiver-wifi");

    WifiHelper wifiHelper;

    NetDeviceContainer senderStaDevices, senderApDevices;
    wifiMacHelper.SetType("ns3::StaWifiMac",
                          "Ssid",
                          SsidValue(senderSsid),
                          "ActiveProbing",
                          BooleanValue(false));
    senderStaDevices = wifiHelper.Install(wifiPhyHelper, wifiMacHelper, senderStaNodes);
    wifiMacHelper.SetType("ns3::ApWifiMac", "Ssid", SsidValue(senderSsid));
    senderApDevices = wifiHelper.Install(wifiPhyHelper, wifiMacHelper, senderApNodes);

    NetDeviceContainer receiverStaDevices, receiverApDevices;
    wifiPhyHelper.SetChannel(wifiChannelHelper.Create());
    wifiMacHelper.SetType("ns3::StaWifiMac",
                          "Ssid",
                          SsidValue(receiverSsid),
                          "ActiveProbing",
                          BooleanValue(false));
    receiverStaDevices = wifiHelper.Install(wifiPhyHelper, wifiMacHelper, receiverStaNodes);
    wifiMacHelper.SetType("ns3::ApWifiMac", "Ssid", SsidValue(receiverSsid));
    receiverApDevices = wifiHelper.Install(wifiPhyHelper, wifiMacHelper, receiverApNodes);

    MobilityHelper mobilityHelper;

    mobilityHelper.SetPositionAllocator("ns3::GridPositionAllocator",
                                        "MinX",
                                        DoubleValue(0.0),
                                        "MinY",
                                        DoubleValue(0.0),
                                        "DeltaX",
                                        DoubleValue(0.1),
                                        "DeltaY",
                                        DoubleValue(0.1),
                                        "GridWidth",
                                        UintegerValue(1),
                                        "LayoutType",
                                        StringValue("RowFirst"));
    Ptr<RandomVariableStream> constanRandomVariable = CreateObject<ConstantRandomVariable>();
    constanRandomVariable->SetAttribute("Constant", DoubleValue(SpeedOfNodes));
    mobilityHelper.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                    "Bounds",
                                    RectangleValue(Rectangle(-50, 50, -50, 50)),
                                    "Speed",
                                    PointerValue(constanRandomVariable));
    mobilityHelper.Install(senderStaNodes);
    mobilityHelper.Install(receiverStaNodes);
    mobilityHelper.Install(bottleneckNodes);

    InternetStackHelper internetStackHelper;
    internetStackHelper.Install(senderStaNodes);
    internetStackHelper.Install(receiverStaNodes);
    internetStackHelper.Install(bottleneckNodes);

    Ipv4AddressHelper addressHelper;

    addressHelper.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer bottleneckInterfaces;
    bottleneckInterfaces = addressHelper.Assign(bottleneckDevices);

    addressHelper.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer senderStaInterfaces, senderApInterface;
    senderStaInterfaces = addressHelper.Assign(senderStaDevices);
    senderApInterface = addressHelper.Assign(senderApDevices);

    addressHelper.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer receiverStaInterfaces, receiverApInterface;
    receiverStaInterfaces = addressHelper.Assign(receiverStaDevices);
    receiverApInterface = addressHelper.Assign(receiverApDevices);

    ApplicationContainer senderApps, receiverApps;

    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory",
                                      InetSocketAddress(Ipv4Address::GetAny(), 9));

    OnOffHelper onOffHelper("ns3::TcpSocketFactory",
                            (InetSocketAddress(receiverStaInterfaces.GetAddress(0), 9)));
    onOffHelper.SetAttribute("PacketSize", UintegerValue(PacketSize));
    onOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    onOffHelper.SetAttribute("DataRate", DataRateValue(DataRate(dataRate)));

    uint64_t startPort = 1024;
    uint64_t numberOfCreatedFlows = 0;

    for (uint64_t currentPort = startPort;; currentPort++)
    {
        for (uint64_t i = 0; i < NumberOfNodes; i++)
        {
            onOffHelper.SetAttribute(
                "Remote",
                AddressValue(InetSocketAddress(receiverStaInterfaces.GetAddress(i), currentPort)));
            onOffHelper.SetAttribute(
                "Local",
                AddressValue(InetSocketAddress(senderStaInterfaces.GetAddress(i), currentPort)));
            packetSinkHelper.SetAttribute(
                "Local",
                AddressValue(InetSocketAddress(receiverStaInterfaces.GetAddress(i), currentPort)));
            senderApps.Add(onOffHelper.Install(senderStaNodes.Get(i)).Get(0));
            receiverApps.Add(packetSinkHelper.Install(receiverStaNodes.Get(i)).Get(0));

            std::ostringstream ossr;
            ossr << "/NodeList/" << receiverStaNodes.Get(i)->GetId() << "/ApplicationList/"
                 << receiverStaNodes.Get(i)->GetNApplications() - 1 << "/$ns3::PacketSink/Rx";
            Config::ConnectWithoutContext(ossr.str(), MakeCallback(&RxTracedCallback));

            std::ostringstream osss;
            osss << "/NodeList/" << senderStaNodes.Get(i)->GetId() << "/ApplicationList/"
                 << senderStaNodes.Get(i)->GetNApplications() - 1 << "/$ns3::OnOffApplication/Tx";
            Config::ConnectWithoutContext(osss.str(), MakeCallback(&TxTracedCallback));

            numberOfCreatedFlows++;
            if (numberOfCreatedFlows == NumberOfFlows)
            {
                break;
            }
        }
        if (numberOfCreatedFlows == NumberOfFlows)
        {
            break;
        }
    }

    receiverApps.Start(Seconds(0.0));
    senderApps.Start(Seconds(1.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Simulator::Stop(Seconds(SimulationTime));

    Simulator::Run();

    Simulator::Destroy();

    if (metricOutputFileStream.tellp() == 0)
    {
        metricOutputFileStream << "NumberOfNodes,NumberOfFlows,PacketSize,"
                               << "NumberOfPacketsPerSecond,SpeedOfNodes,"
                               << "CoverageArea,TotalTxPackets,TotalRxPackets,"
                               << "PacketDeliveryRatio,Throughput" << std::endl;
    }

    metricOutputFileStream << NumberOfNodes << "," << NumberOfFlows << "," << PacketSize << ","
                           << NumberOfPacketsPerSecond << "," << SpeedOfNodes << "," << CoverageArea
                           << "," << totalTxPackets << "," << totalRxPackets << ","
                           << PacketDeliveryRatio(totalTxPackets, totalRxPackets) << ","
                           << CalculateThroughput(totalRxPackets, PacketSize, SimulationTime)
                           << std::endl;

    return 0;
}