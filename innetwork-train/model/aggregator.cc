#include "aggregator.h"

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("Aggregator");
    NS_OBJECT_ENSURE_REGISTERED (Aggregator);

    TypeId Aggregator::GetTypeId () {
        static TypeId tid = TypeId ("ns3::Aggregator")
            .SetParent<Application> ()
            .SetGroupName("innetwork-train")
            .AddConstructor<Aggregator> ()
        ;
        return tid;
    };

    Aggregator::Aggregator() {
        NS_LOG_FUNCTION (this);
    }

    Aggregator::~Aggregator() {
        NS_LOG_FUNCTION (this);
    }

    void 
    Aggregator::SetupAggregator (uint16_t port, uint16_t itr, uint8_t rank, std::vector<Address> &sGroup, 
                                        std::vector<Address> &cGroup, uint16_t basetime, std::string cc_name) {
        NS_LOG_FUNCTION (this);
        this->m_peerPort = port;
        this->basetime = basetime;
        this->cc_name = cc_name;
        this->cMPI = CreateObject<SimpleMPI> ();
        this->cMPI->SetupMPI (port, itr, rank, sGroup, cGroup, GetNode (), false);
    }

    void 
    Aggregator::DoDispose() {
        NS_LOG_FUNCTION (this);
        Application::DoDispose ();
    }

    void
    Aggregator::StartApplication() {
        NS_LOG_FUNCTION (this);
        this->cMPI->CreateSocketPool (cc_name);
        ns3::Simulator::Schedule(ns3::MilliSeconds(2 * basetime + 500), &SimpleMPI::ReceiveDataFromAll, this->cMPI);
    }

    void 
    Aggregator::StopApplication() {
        NS_LOG_FUNCTION (this);
    }

    
}; /*namespace ns3*/