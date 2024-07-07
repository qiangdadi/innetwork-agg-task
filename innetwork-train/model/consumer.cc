#include "consumer.h"

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("Consumer");
    NS_OBJECT_ENSURE_REGISTERED (Consumer);

    TypeId 
    Consumer::GetTypeId (void) {
        static TypeId tid = TypeId ("ns3::Consumer")
            .SetParent<Application> ()
            .SetGroupName("innetwork-train")
            .AddConstructor<Consumer> ()
        ;
        return tid;
    };

    Consumer::Consumer() {
        NS_LOG_FUNCTION (this);
    }

    void 
    Consumer::SetupConsumer (uint16_t port, uint16_t itr, uint8_t rank, uint16_t vsize, std::vector<Address> &sGroup, 
                                        std::vector<Address> &cGroup, uint16_t basetime, std::string cc_name) {
        NS_LOG_FUNCTION (this);
        const std::string fileName = "./topofile/data.txt";
        this->m_peerPort = port;
        this->basetime = basetime;
        this->cc_name = cc_name;
        this->sMPI = CreateObject<SimpleMPI> ();
        this->sMPI->SetupMPI (port, itr, rank, sGroup, cGroup, GetNode (), true);
        this->sMPI->SetOutFile (fileName);
        this->sMPI->SetVSize (vsize);
    }

    void
    Consumer::StartApplication () {
        NS_LOG_FUNCTION (this);
        this->sMPI->CreateSocketPool (cc_name);
        ns3::Simulator::Schedule(ns3::MilliSeconds(basetime), &SimpleMPI::AVG, this->sMPI);
        ns3::Simulator::Schedule(ns3::MilliSeconds(basetime + 30), &SimpleMPI::ReceiveDataFromAll, this->sMPI);
    }

    Consumer::~Consumer () {
        NS_LOG_FUNCTION (this);
        //outFile. close ();
    }

    void 
    Consumer::DoDispose () {
        NS_LOG_FUNCTION (this);
        Application::DoDispose ();
    }
    
    void 
    Consumer::StopApplication () {
        NS_LOG_FUNCTION (this);
        NS_LOG_INFO ("Consumer Stop");
    }
    
}; /*namespace ns3*/