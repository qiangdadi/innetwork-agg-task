#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace ns3;

struct LinkInfo {
    std::string node1;
    std::string node2;
    std::string dataRate;
    double packetLossRate;
    std::string delay;
    uint32_t queueSize;
};

std::vector<std::string> readRouteFile(const std::string &filename) {
    std::vector<std::string> routers;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            routers.push_back(line);
        }
    }
    return routers;
}

std::vector<LinkInfo> readLinkFile(const std::string &filename) {
    std::vector<LinkInfo> links;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        LinkInfo link;
        if (iss >> link.node1 >> link.node2 >> link.dataRate >> link.packetLossRate >> link.delay >> link.queueSize) {
            links.push_back(link);
        }
    }
    return links;
}

void createTopologyAndTest(const std::string &routeFile, const std::string &linkFile) {
    // Read the files
    std::vector<std::string> routers = readRouteFile(routeFile);
    std::vector<LinkInfo> links = readLinkFile(linkFile);

    if (routers.empty() || links.empty()) {
        std::cout << "Failed to read input files\n";
        return;
    }

    // Create nodes
    std::unordered_map<std::string, Ptr<Node>> routerMap;
    for (const std::string &routerName : routers) {
        Ptr<Node> node = CreateObject<Node>();
        routerMap[routerName] = node;
        Names::Add(routerName, node);  // For easier reference later
    }

    NodeContainer nodes;
    for (const auto &router : routerMap) {
        nodes.Add(router.second);
    }

    InternetStackHelper internet;
    internet.Install(nodes);

    // Create point-to-point links
    for (const LinkInfo &link : links) {
        NodeContainer linkNodes;
        linkNodes.Add(routerMap[link.node1]);
        linkNodes.Add(routerMap[link.node2]);

        PointToPointHelper p2p;
        p2p.SetDeviceAttribute("DataRate", StringValue(link.dataRate));
        p2p.SetChannelAttribute("Delay", StringValue(link.delay));
        p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(link.queueSize + "p"));

        NetDeviceContainer devices = p2p.Install(linkNodes);

        // Set packet loss rate
        Ptr<RateErrorModel> em = CreateObjectWithAttributes<RateErrorModel>(
            "ErrorRate", DoubleValue(link.packetLossRate/ 100));
        devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

        // Assign IP addresses (you might want to adjust the base addresses)
        Ipv4AddressHelper address;
        std::string ipBase = "10.1." + std::to_string(&link - &links[0] + 1) + ".0";
        address.SetBase(Ipv4Address(ipBase.c_str()), "255.255.255.0");
        address.Assign(devices);
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    for (const LinkInfo &link : links) {
        Ptr<Node> srcNode = routerMap[link.node1];
        Ptr<Node> dstNode = routerMap[link.node2];

        // Create a TCP echo server on the destination node
        uint16_t tcpPort = 50000; // TCP port for testing
        Address tcpAddress(InetSocketAddress(Ipv4Address::GetAny(), tcpPort));
        PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", tcpAddress);
        ApplicationContainer serverApps = packetSinkHelper.Install(dstNode);
        serverApps.Start(Seconds(1.0));
        serverApps.Stop(Seconds(10.0));

        // Create a TCP echo client on the source node
        OnOffHelper onOffHelper("ns3::TcpSocketFactory", Address());
        AddressValue remoteAddress(InetSocketAddress(dstNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), tcpPort));
        onOffHelper.SetAttribute("Remote", remoteAddress);
        onOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        onOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        onOffHelper.SetAttribute("PacketSize", UintegerValue(1024));
        onOffHelper.SetAttribute("DataRate", StringValue("50Mbps"));

        ApplicationContainer clientApps = onOffHelper.Install(srcNode);
        clientApps.Start(Seconds(2.0));
        clientApps.Stop(Seconds(10.0));
    }

    Simulator::Run();
    Simulator::Destroy();
}

int main(int argc, char *argv[]) {
    std::string routeFile = "/root/in-network-task/quic-ns-3/topofile/router.txt";
    std::string linkFile = "/root/in-network-task/quic-ns-3/topofile/link.txt";

    createTopologyAndTest(routeFile, linkFile);

    return 0;
}