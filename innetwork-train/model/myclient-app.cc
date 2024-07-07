// #include "myclient-app.h"

// namespace ns3 {
//     NS_LOG_COMPONENT_DEFINE ("Consumer");
//     NS_OBJECT_ENSURE_REGISTERED (Consumer);

//     TypeId 
//     Consumer::GetTypeId (void) {
//         static TypeId tid = TypeId ("ns3::Consumer")
//             .SetParent<Application> ()
//             .SetGroupName("innetwork-train")
//             .AddConstructor<Consumer> ()
//         ;
//         return tid;
//     };

//     Consumer::Consumer () {
//         NS_LOG_FUNCTION (this);
//     }

//     void 
//     Consumer::SetupConsumer (const char *cc_name, const Ipv4Address& peer_addr, uint16_t peer_port) {
//         NS_LOG_FUNCTION (this);
//         myclient = new MyQuicClient(cc_name, GetNode ());
//         myclient->Bind ();
//         myclient->set_peer (peer_addr, peer_port);
//     }

//     void
//     Consumer::StartApplication () {
//         NS_LOG_FUNCTION (this);
//         myclient->Start ();
//         Simulator::Schedule(MilliSeconds(100), &Consumer::SendHello, this);
//         Simulator::Schedule(MilliSeconds(110), &Consumer::ReceiveDataFrom, this);
//     }

//     void 
//     Consumer::SendHello() {
//         const char *hello="my hello world";
//         myclient->Send (hello, strlen(hello), true);
//     }

//     void 
//     Consumer::ReceiveDataFrom () {
//         //NS_LOG_FUNCTION (this);
//         uint16_t bufferSize = myclient->GetBuffer ()->getSize ();
//         // buffer has a whole packet, try to read and prase
//         while (bufferSize > 0) 
//         {
//             uint16_t bs = bufferSize > 1024 * 2 ? bufferSize : 1024 * 2;
//             uint8_t *buffer = new uint8_t[bs];
//             myclient->GetBuffer ()->read (buffer, bufferSize);
//             //NS_LOG_INFO ("[Consumer:ReceiveDataFrom] receive data " << (char*)buffer);
//             delete [] buffer; //delete buffer
//         }
//         Simulator::Schedule(MilliSeconds(10), &Consumer::ReceiveDataFrom, this);
//     }

//     Consumer::~Consumer() {
//         NS_LOG_FUNCTION (this);

//     }

//     void 
//     Consumer::StopApplication () {
//         NS_LOG_FUNCTION (this);
//     }

// }; /*namespace ns3*/