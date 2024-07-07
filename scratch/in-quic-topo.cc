#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/quic-module.h"
#include "ns3/innetwork-train-module.h"
#include "ns3/traffic-control-module.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

using namespace ns3;

const std::string linkFilePath = "./topofile/link-2.txt";
const std::string aggGropuFilePath = "./topofile/aggtree.txt";
const std::string conName = "con";
const std::string proName = "pro";
const std::string fowName = "forwarder";
const std::string aggName = "agg";
const uint32_t conNum = 1;
const uint32_t proNum = 30;
const uint32_t fowNum = 10;
const uint32_t aggNum = 6;
std::string cc = "cubic";
uint16_t basetime = 1000;
uint16_t starttime = 1;
uint16_t stoptime = 5000;

// name nodes
void NameNodes(NodeContainer &nodes, std::string baseName) {
    //Name cons
    for (uint8_t i = 0; i < nodes.GetN(); ++i)
    {
        std::string nodeName = baseName + std::to_string(i);
        if (Names::Find<Node> (nodeName) == 0) {
            Names::Add(nodeName, nodes.Get(i));
            //std::cout << "Name added successfully: " << nodeName << std::endl;

        } else {
            std::cout << "Error: Name already exists: " << nodeName << std::endl;
        }
    }
}

// read link.txt to create links
void BuileTopo(const std::string &linkFile,  NodeContainer &cons, NodeContainer &pros, 
            NodeContainer &fows, NodeContainer &aggs) {
    // create nodes
    cons.Create (conNum);  
    pros.Create (proNum);
    fows.Create (fowNum);
    aggs.Create (aggNum);

    // name nodes
    NameNodes (cons, conName);
    NameNodes (pros, proName);
    NameNodes (fows, fowName);
    NameNodes (aggs, aggName);
    //std::cout << "name end" << std::endl;

    // install internet stack
    InternetStackHelper stack;
    stack.Install (cons);
    stack.Install (pros);
    stack.Install (fows);
    stack.Install (aggs);
    //std::cout << "install internet end" << std::endl;

    std::ifstream infile (linkFile);
    std::string line;
    // create p2p links
    PointToPointHelper p2p;
    Ipv4AddressHelper address;
    uint32_t ia = 1, ic = 1;
    while (std::getline (infile, line)) {
        std::istringstream iss (line);
        std::string node1, node2, dataRate, delay, queueSize;
        double lossRate;
        if (!(iss >> node1 >> node2 >> dataRate >> lossRate >> delay >> queueSize)) {
            continue; // jump error-format lines
        }

        p2p.SetDeviceAttribute("DataRate", StringValue (dataRate));
        p2p.SetChannelAttribute("Delay", StringValue (delay));
        p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue (queueSize + "p"));
        //std::cout << "set p2p linkAttribute end" << std::endl;

        if (Names::Find<Node> (node1) == 0 || Names::Find<Node> (node2) == 0) {
            std::cout << "Name does not exist, node1 : " << node1 << " node2 : " << node2 << std::endl;
        }

        //attach error model to router
        int nidx = 0;
        if (node1.find ("pro") != std::string::npos || node1.find ("con") != std::string::npos) {
            nidx = 1;
        }

        NodeContainer n1n2 = NodeContainer (Names::Find<Node> (node1), Names::Find<Node> (node2));
        // install p2p
        NetDeviceContainer d1d2 = p2p.Install (n1n2);
        
        // assign ip address
        std::string ipBaseAddr;
        if (node1.find ("agg") != std::string::npos || node2.find ("agg") != std::string::npos) {
            ipBaseAddr = "10.2." + std::to_string (ia) + ".0";
            ++ia;

        } else {
            ipBaseAddr = "10.1." + std::to_string (ic) + ".0";
            ++ic;
        }

        address.SetBase(Ipv4Address(ipBaseAddr.c_str()), "255.255.255.0");
        address.Assign(d1d2);

    }
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    infile.close ();
}

void Createpro (uint16_t port, uint16_t itr, uint8_t rank, std::vector<Address> &sGroup, 
                std::vector<Address> &cGroup, Ptr<Node> node) {
    Ptr<Producer> producer = CreateObject<Producer> ();
    node->AddApplication (producer);
    producer->SetupProducer (port, itr, rank, sGroup, cGroup, basetime * 6, cc);
    producer->SetStartTime (Seconds(starttime));
    producer->SetStopTime (Seconds(stoptime));
}

void Createagg (uint16_t port, uint16_t itr, uint8_t rank, std::vector<Address> &sGroup, 
                                        std::vector<Address> &cGroup, Ptr<Node> node) {
    Ptr<Aggregator> aggragator = CreateObject<Aggregator> ();
    node->AddApplication (aggragator);
    aggragator->SetupAggregator (port, itr, rank, sGroup, cGroup, basetime, cc);
    aggragator->SetStartTime (Seconds(starttime + 1));
    aggragator->SetStopTime (Seconds(stoptime));
}

void Createcon (uint16_t port, uint16_t itr, uint8_t rank, uint16_t vsize,
                    std::vector<Address> &sGroup, std::vector<Address> &cGroup, Ptr<Node> node) {
    Ptr<Consumer> consumer = CreateObject<Consumer> ();
    node->AddApplication (consumer);
    consumer->SetupConsumer (port, itr, rank, vsize, sGroup, cGroup, basetime, cc);
    consumer->SetStartTime (Seconds(starttime + 2));
    consumer->SetStopTime (Seconds(stoptime));
}

void CreateAggGroup (const std::string aggGroupFile, 
                        std::unordered_map<std::string, std::vector<std::string>> &aggGroups) {
    std::ifstream infile (aggGroupFile);
    std::string line;
    while (std::getline (infile, line)) {
        std::istringstream iss (line);
        std::vector<std::string> nodes;
        std::string nodeName, childName;
        iss >> nodeName;
        nodeName. pop_back (); // delete ':' character
    
        while (iss >> childName) {
            nodes. push_back (childName);
        }
        aggGroups[nodeName] = nodes;
    }
}

void CreateAggTree (std::string &nodeName, std::vector<Address> pa,
                std::unordered_map<std::string, std::vector<std::string>> &aggGroups,
                uint8_t rank, uint16_t itr, uint16_t vsize, uint16_t server_port) {
    std::vector<Address> sGroup;
    Ptr<Ipv4> ipv4 = Names::Find<Node> (nodeName)->GetObject<Ipv4>();
    Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1, 0);
    Ipv4Address addr = iaddr.GetLocal ();
    sGroup. push_back (addr);

    std::vector<Address> cGroup;
    for (std::string &str : aggGroups[nodeName]) {
        rank += 1;
        CreateAggTree (str, sGroup, aggGroups, rank, itr, vsize, server_port);
        ipv4 = Names::Find<Node> (str)->GetObject<Ipv4>();
        iaddr = ipv4->GetAddress (1, 0);
        addr = iaddr.GetLocal ();
        cGroup. push_back (addr);
    }
    //std::cout << "current node " << nodeName << std::endl;
    if (nodeName. find ("con") != std::string::npos)
        Createcon(server_port, itr, rank, vsize, pa, cGroup, Names::Find<Node> (nodeName));
    if (nodeName. find ("agg") != std::string::npos)
        Createagg(server_port, 0, rank, pa, cGroup, Names::Find<Node> (nodeName));
    if (nodeName. find ("pro") != std::string::npos)
        Createpro(server_port, 0, rank, pa, cGroup, Names::Find<Node> (nodeName));
}

void CreateAggTreeTopo (uint16_t itr, uint16_t vsize, uint16_t server_port) {
    // get Aggragator Group
    std::unordered_map<std::string, std::vector<std::string>> aggGroups;
    CreateAggGroup (aggGropuFilePath, aggGroups);

    // //create agg tree
    std::string root = "con0";
    CreateAggTree (root, std::vector<Address>(), aggGroups, 0, itr, vsize, server_port);
}

void CreateDirectTopo (NodeContainer &cons, NodeContainer &pros, uint16_t itr, 
                       uint16_t vsize, uint16_t server_port) {
    std::vector<Address> consumers;
    Ptr<Ipv4> ipv4 = cons. Get (0)->GetObject<Ipv4>();
    Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1, 0);
    Ipv4Address addr = iaddr.GetLocal ();
    consumers. push_back (addr);

    std::vector<Address> producers;
    for (uint8_t i = 0; i < pros. GetN (); ++i) { //
        ipv4 = pros. Get (i)->GetObject<Ipv4>();
        iaddr = ipv4->GetAddress (1, 0);
        addr = iaddr.GetLocal ();
        producers. push_back (addr);
    }

    uint8_t rank = 0;
    std::vector<Address> nullV;
    for (uint8_t i = 0; i < pros. GetN (); ++i) {
        Ptr<Producer> producer = CreateObject<Producer> ();
        pros. Get (i)->AddApplication (producer);
        producer->SetupProducer (server_port, 0, rank, consumers, nullV, basetime * 2, cc);
        rank++;
        producer->SetStartTime (Seconds(starttime));
        producer->SetStopTime (Seconds(stoptime));
    }
    

    Ptr<Consumer> consumer = CreateObject<Consumer> ();
    cons. Get (0)->AddApplication (consumer);
    consumer->SetupConsumer (server_port, itr, rank, vsize, nullV, producers, basetime, cc);
    consumer->SetStartTime (Seconds(starttime + 1));
    consumer->SetStopTime (Seconds(stoptime));
}

int main(int argc, char *argv[]) {
    CommandLine cmd;
    uint16_t itr = 1000;
    uint16_t vsize = 1000;
    uint16_t topotype = 0;
    cmd.AddValue("itr", "max iteration consumer performed", itr);
    cmd.AddValue("vsize", "vector size", vsize);
    cmd.AddValue("topotype", "choose test topo type", topotype);
    cmd.AddValue("cc", "choose test congestion control", cc);
    cmd.AddValue("basetime", "set base time", basetime);
    cmd.AddValue("starttime", "set start time", starttime);
    cmd.AddValue("stoptime", "set stop time", stoptime);

    cmd.Parse (argc, argv);
    vsize = (vsize / k) == 0 ? vsize : ((vsize + k - 1) / k) * k; // expand vsize
    // create links
    NodeContainer cons, pros, fows, aggs;
    BuileTopo (linkFilePath, cons, pros, fows, aggs);

    // run application to test topo connectivity
    std::cout
      << "\n\n#################### SIMULATION SET-UP ####################\n\n\n";
    LogLevel log_precision = LOG_LEVEL_INFO; //LOG_LEVEL_ALL
    // LogComponentEnable("Ns3QuicChannelBase",log_precision);
    // LogComponentEnable("Consumer",log_precision);
    //LogComponentEnable("CircularBuffer",log_precision);
    //LogComponentEnable("Ns3QuicBackendBase",log_precision);
    // LogComponentEnable("Producer",log_precision);
    // LogComponentEnable("MyQuicServer",log_precision);
    // LogComponentEnable("MyQuicClient",log_precision);
    // LogComponentEnable("Aggregator",log_precision);
    LogComponentEnable("SimpleMPI",log_precision);
    
    const char *envKey="QUICHE_SRC_DIR";
    char *envValue = getenv(envKey);
    if(envValue){
        std::cout<<envValue<<std::endl;
    }
    std::string quic_cert_path=std::string(envValue)+"utils/data/quic-cert/";
    ns3::set_quic_cert_key(quic_cert_path);

    uint16_t server_port=1234;
    if (topotype == 0) {
        CreateDirectTopo (cons, pros, itr, vsize, server_port);

    } else {
        CreateAggTreeTopo (itr, vsize, server_port);
    }

    // PointToPointHelper pointToPoint;
    // pointToPoint.EnablePcapAll  ("/home/hxq/ns-allinone-3.37/ns-3.37/topo/topo");

    // run
    //Simulator::Stop (Seconds (50000));
    Simulator::Run();
    
    //end
    Simulator::Destroy();
    std::cout
      << "\n\n#################### SIMULATION END ####################\n\n\n";
    return 0;
}