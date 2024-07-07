// #include "myserver-app.h"

// namespace ns3 {
//     NS_LOG_COMPONENT_DEFINE ("Producer");
//     NS_OBJECT_ENSURE_REGISTERED (Producer);

//     TypeId 
//     Producer::GetTypeId (void) {
//         static TypeId tid = TypeId ("ns3::Producer")
//             .SetParent<Application> ()
//             .SetGroupName("innetwork-train")
//             .AddConstructor<Producer> ()
//         ;
//         return tid;
//     };

//     Producer::Producer () {
//         NS_LOG_FUNCTION (this);
//     }

//     void 
//     Producer::SetupProducer (uint16_t port) {
//         NS_LOG_FUNCTION (this);
//         myserver = new MyQuicServer(GetNode ());
//         myserver->Bind(port);
//     }

//     void
//     Producer::StartApplication () {
//         NS_LOG_FUNCTION (this);
//         myserver->Start ();
//         Simulator::Schedule(MilliSeconds(2000), &Producer::ReceiveDataFrom, this);
//     }

//     void 
//     Producer::ReceiveDataFrom () {
//         //NS_LOG_FUNCTION (this);
//         uint16_t bufferSize = myserver->GetBuffer ()->getSize ();
//         // buffer has a whole packet, try to read and prase
//         while (bufferSize > 0) 
//         {
//             uint16_t bs = bufferSize > 1024 * 2 ? bufferSize : 1024 * 2;
//             uint8_t *buffer = new uint8_t[bs];
//             myserver->GetBuffer ()->read (buffer, bufferSize);
//             NS_LOG_INFO ("[Producer:ReceiveDataFrom] receive data " << (char*)buffer);
//             delete [] buffer; //delete buffer
//             bufferSize = myserver->GetBuffer ()->getSize ();
//         }
//         Simulator::Schedule(MilliSeconds(10), &Producer::ReceiveDataFrom, this);
//     }

//     Producer::~Producer() {
//         NS_LOG_FUNCTION (this);
//     }

//     void 
//     Producer::StopApplication () {
//         NS_LOG_FUNCTION (this);
//     }

// }; /*namespace ns3*/