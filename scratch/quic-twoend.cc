#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/quic-module.h"
#include "ns3/innetwork-train-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("quic-twoend");

int
main (int argc, char *argv[])
{
    CommandLine cmd;
    cmd.Parse (argc, argv);
    LogLevel log_precision = LOG_LEVEL_INFO; //LOG_LEVEL_ALL
    //LogComponentEnable("Ns3QuicChannelBase",log_precision);
    //LogComponentEnable("Consumer",log_precision);
    //LogComponentEnable("CircularBuffer",log_precision);
    //LogComponentEnable("Ns3QuicBackendBase",log_precision);
    //LogComponentEnable("Producer",log_precision);
    //LogComponentEnable("MyQuicServer",log_precision);
    //LogComponentEnable("MyQuicClient",log_precision);
    LogComponentEnable("SimpleMPI",log_precision);

    std::cout
        << "\n\n#################### SIMULATION SET-UP ####################\n\n\n";

    NodeContainer nodes;
    nodes.Create (2);
    auto n1 = nodes.Get (0);
    auto n2 = nodes.Get (1);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

    NetDeviceContainer devices;
    devices = pointToPoint.Install (nodes);

    Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
    uv->SetStream (50);
    RateErrorModel error_model;
    error_model.SetRandomVariable (uv);
    error_model.SetUnit (RateErrorModel::ERROR_UNIT_PACKET);
    error_model.SetRate (0.01);
    devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (&error_model));

    InternetStackHelper stack;
    stack.Install (nodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces = address.Assign (devices);

    Ptr<Node> h1=nodes.Get(0);
    Ptr<Node> h2=nodes.Get(1);

    const char *envKey="QUICHE_SRC_DIR";
    char *envValue=getenv(envKey);
    if(envValue){
        std::cout<<envValue<<std::endl;
    }
    std::string quic_cert_path=std::string(envValue)+"utils/data/quic-cert/";
    ns3::set_quic_cert_key(quic_cert_path);

    std::string cc = "cubic";
    uint16_t basetime = 1000;
    uint16_t server_port=1234;
    std::vector<Address> ssGroup;
    std::vector<Address> scGroup;
    Ptr<Ipv4> ipv4 = h1->GetObject<Ipv4> ();
    Ipv4Address client_ip = ipv4->GetAddress (1, 0).GetLocal();
    ssGroup. push_back (client_ip);
    std::stringstream ss;
    ss << Ipv4Address::ConvertFrom (client_ip);
    std::cout << "[MyQuicClient:Bind] client listening on addr " << ss.str() << std::endl;

    InetSocketAddress server_addr(server_port);
    Ptr<Producer> producer=CreateObject<Producer>();
    h2->AddApplication(producer);
    producer->SetupProducer (server_port, 0, 1, ssGroup, scGroup, basetime, cc);
    producer->SetStartTime (Seconds(1));
    producer->SetStopTime (Seconds(6));

    std::vector<Address> sGroup;
    std::vector<Address> cGroup;
    ipv4 = h2->GetObject<Ipv4> ();
    Ipv4Address server_ip = ipv4->GetAddress (1, 0).GetLocal();
    cGroup. push_back (server_ip);
    ss << Ipv4Address::ConvertFrom (server_ip);
    std::cout << "[MyQuicServer:Bind] server listening on addr " << ss.str() << std::endl;

    //install client
    Ptr<Consumer> consumer= CreateObject<Consumer>();
    h1->AddApplication(consumer);
    consumer->SetupConsumer (server_port, 1000, 0, 1000, sGroup, cGroup, basetime, cc);
    consumer->SetStartTime (Seconds(2));
    consumer->SetStopTime (Seconds(6));

    std::cout << "\n\n#################### STARTING RUN ####################\n\n";
    Simulator::Run ();
    std::cout
        << "\n\n#################### RUN FINISHED ####################\n\n\n";
    Simulator::Destroy ();

    std::cout
        << "\n\n#################### SIMULATION END ####################\n\n\n";
    return 0;
}