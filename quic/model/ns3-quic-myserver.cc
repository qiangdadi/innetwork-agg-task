#include "gquiche/quic/platform/api/quic_default_proof_providers.h"
#include "gquiche/quic/platform/api/quic_logging.h"

#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3-quic-clock.h"
#include "ns3-quic-tag.h"
#include "ns3-quic-flags.h"
#include "ns3-quic-addr-convert.h"
#include "ns3-quic-server.h"
#include "ns3-quic-myserver.h"
#include "ns3-packet-writer.h"
#include "ns3-quic-alarm-engine.h"
#include "ns3-quic-backend.h"
#include <iostream>
namespace ns3{

NS_LOG_COMPONENT_DEFINE("MyQuicServer");

class MyQuicServerImpl:public quic::Ns3PacketWriter::Delegate{
public:
    MyQuicServerImpl(MyQuicServer *outer,quic::BackendType type);
    ~MyQuicServerImpl() override;

    //from quic::Ns3PacketWriter::Delegate
    int WritePacket(const char* buffer,
                    size_t buf_len,
                    const quic::QuicIpAddress& self_address,
                    const quic::QuicSocketAddress& peer_address) override;
    void OnWriterDestroy() override{}
    void OnIncomingData(quic::QuicSocketAddress self_addr,
                        quic::QuicSocketAddress peer_addr,
                        const char *buffer, size_t sz);
private:
    int Start();
    void Stop();
    MyQuicServer *m_outer=nullptr;
    quic::QuicClock *m_clock=nullptr;
    std::unique_ptr<quic::Ns3QuicServer> m_quicServer;
    std::unique_ptr<quic::Ns3QuicBackendBase> m_backend;
    std::unique_ptr<quic::Ns3QuicAlarmEngine> m_engine;
    friend class MyQuicServer;
};

MyQuicServer::MyQuicServer(Ptr<Node> node, quic::BackendType type){
    m_impl.reset(new MyQuicServerImpl(this,type));
    m_running=true;
    this->node = node;
    m_bindIp=GetLocalAddress().GetIpv4();
    if (m_impl) {
        m_impl->Start();
    }
}

MyQuicServer::~MyQuicServer(){
    if(m_impl){
        m_impl->Stop();
    }
    m_running=false;
    if(m_socket){
        m_socket->Close();
        m_socket=nullptr;
    }
}

void MyQuicServer::Bind(uint16_t port){
    if (NULL==m_socket){
        m_socket = Socket::CreateSocket (node,UdpSocketFactory::GetTypeId ());
        auto local = InetSocketAddress{Ipv4Address::GetAny (),port};
        auto res = m_socket->Bind (local);
        NS_ASSERT (res == 0);
        Address addr;
        m_socket->GetSockName(addr);
        InetSocketAddress self_sock_addr=InetSocketAddress::ConvertFrom(addr);
        std::stringstream ss;
        Ipv4Address ipv4Addr = self_sock_addr.GetIpv4 ();
        ss << Ipv4Address::ConvertFrom (ipv4Addr);
        //NS_LOG_INFO ("[MyQuicServer:Bind] server listening on addr " << ss.str());
        m_bindPort=self_sock_addr.GetPort();
        m_socket->SetRecvCallback (MakeCallback(&MyQuicServer::RecvPacket,this));
    }
}

InetSocketAddress MyQuicServer::GetLocalAddress() const{
    //Ptr<Node> node=GetNode();
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    Ipv4Address local_ip = ipv4->GetAddress (1, 0).GetLocal();
    return InetSocketAddress{local_ip,m_bindPort}; 
}

void MyQuicServer::RecvPacket(Ptr<Socket> socket){
    if (!m_running) {return ;}
    InetSocketAddress ns3_self_sock_addr(m_bindIp,m_bindPort);
    quic::QuicSocketAddress quic_self_sock_addr=GetQuicSocketAddr(ns3_self_sock_addr);
    
    Address remoteAddr;
    auto packet = socket->RecvFrom (remoteAddr);
    InetSocketAddress ns3_peer_sock_addr=InetSocketAddress::ConvertFrom(remoteAddr);
    quic::QuicSocketAddress quic_peer_sock_addr=GetQuicSocketAddr(ns3_peer_sock_addr);
    uint32_t length=packet->GetSize();
    //NS_LOG_INFO(this<<" MyQuicServer::recv "<<ns3_peer_sock_addr.GetIpv4()<<" "<<length);
    char buffer[1024 * 2]={'\0'};
    packet->CopyData((uint8_t*)buffer,length);
    if(m_impl){
        m_impl->OnIncomingData(quic_self_sock_addr,quic_peer_sock_addr,buffer,(size_t)length);
    }
    
}

void MyQuicServer::SendToNetwork(const char *buffer,size_t sz,const InetSocketAddress& dest){
    Time now=Simulator::Now();
    Ptr<Packet> packet=Create<Packet>((const uint8_t*)buffer,sz);
    //NS_LOG_INFO(this<<" MyQuicServer::send "<<dest.GetIpv4()<<" "<<dest.GetPort()<<" "<<now.GetSeconds());
    if (!m_socket || !m_running) {return ;}
    m_seq++;
    m_socket->SendTo(packet,0,dest);
}

Ptr<ns3::CircularBuffer>
MyQuicServer::GetBuffer() {
    if ( ((quic::MyServerBackend*)(m_impl->m_backend. get ()))->endpoint_ != nullptr) {
            return ((quic::MyServerBackend*)(m_impl->m_backend. get ()))->endpoint_->GetBuffer ();
    }
    return nullptr;
}

void 
MyQuicServer::Start () {
    m_running=true;
    m_bindIp=GetLocalAddress().GetIpv4();
    if (m_impl) {
        m_impl->Start();
        NS_LOG_INFO ("[MyQuicServer::Start] m_impl start");
    }
}

size_t 
MyQuicServer::Send (const char* buffer, size_t len, bool fin) {
    size_t wlen = 0;
    if (dynamic_cast<quic::MyServerBackend*>(m_impl->m_backend. get ()) != nullptr) {
        if ( ((quic::MyServerBackend*)(m_impl->m_backend. get ()))->endpoint_ != nullptr) {
            wlen = ((quic::MyServerBackend*)(m_impl->m_backend. get ()))->endpoint_->Send(buffer, len, fin);
        }
    }
    return wlen;
}

MyQuicServerImpl::MyQuicServerImpl(MyQuicServer *outer,quic::BackendType type):
m_outer(outer){
    m_clock=quic::Ns3QuicClock::Instance();
    m_engine.reset(new quic::Ns3QuicAlarmEngine(quic::Perspective::IS_SERVER));
    if(quic::BackendType::HELLO_UNIDI==type||quic::BackendType::HELLO_BIDI==type){
        m_backend.reset(new quic::HelloServerBackend(m_engine.get()));
    }
    if (quic::BackendType::BANDWIDTH==type) {
        m_backend.reset(new quic::BandwidthServerBackend(m_engine.get()));
    }
    if (quic::BackendType::MYBACKEND==type) {
        NS_LOG_INFO ("server create mybackend");
        m_backend.reset(new quic::MyServerBackend(m_engine.get()));
    }
}

MyQuicServerImpl::~MyQuicServerImpl(){
    Stop();
}


int MyQuicServerImpl::WritePacket(const char* buffer,
size_t buf_len,
const quic::QuicIpAddress& self_address,
const quic::QuicSocketAddress& peer_address){
    InetSocketAddress ns3_sock_addr=GetNs3SocketAddr(peer_address);
    if (m_outer){
        m_outer->SendToNetwork(buffer,buf_len,ns3_sock_addr);
    }
    return buf_len;
}
void MyQuicServerImpl::OnIncomingData(quic::QuicSocketAddress self_addr,
                    quic::QuicSocketAddress peer_addr,
                    const char *buffer, size_t sz){
    quic::QuicTime now=m_clock->Now();
    quic::QuicReceivedPacket quic_packet(buffer,sz,now);
    if(m_quicServer){
        m_quicServer->ProcessPacket(self_addr,peer_addr,quic_packet);
    }
}

int MyQuicServerImpl::Start(){
    //NS_LOG_INFO ("server imp start");
    quic::ParsedQuicVersionVector supported_versions;
    if (quic::get_quic_ietf_draft()) {
        quic::QuicVersionInitializeSupportForIetfDraft();
        for (const quic::ParsedQuicVersion& version : quic::AllSupportedVersions()) {
            // Add all versions that supports IETF QUIC.
            if (version.HasIetfQuicFrames() &&
                version.handshake_protocol == quic::PROTOCOL_TLS1_3) {
                supported_versions.push_back(version);
            }
        }
    } else {
        supported_versions = quic::AllSupportedVersions();
    }
    std::string versions_string = quic::get_quic_version();;
    if (!versions_string.empty()) {
        supported_versions = quic::ParseQuicVersionVectorString(versions_string);
    }
    if (supported_versions.empty()) {
        return 1;
    }
    for (const auto& version : supported_versions) {
        quic::QuicEnableVersion(version);
    }
    auto proof_source = quic::CreateDefaultProofSource();
    QUICHE_CHECK(proof_source!=nullptr);
    //NS_LOG_INFO ("server impl proof_source");
    quic::Ns3QuicAlarmEngine *engine=m_backend->get_engine();
    m_quicServer.reset(new quic::Ns3QuicServer(std::move(proof_source),
                                               engine,m_backend.get()));
    quic::QuicPacketWriter *writer=new quic::Ns3PacketWriter(this);
    if(!m_quicServer->ConfigureDispatcher(writer)){
        return 1;
    }
    //NS_LOG_INFO ("server impl end");
    return 0;
}
void MyQuicServerImpl::Stop(){
    if(m_quicServer){
        m_quicServer->Shutdown();
    }
    m_quicServer=nullptr;
    m_backend=nullptr;
    m_engine=nullptr; 
}

}
