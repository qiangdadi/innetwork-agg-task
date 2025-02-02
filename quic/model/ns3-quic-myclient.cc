#include "gquiche/quic/tools/fake_proof_verifier.h"
#include "gquiche/quic/core/quic_session.h"

#include "ns3-quic-poll-server.h"
#include "ns3-packet-writer.h"
#include "ns3-quic-client.h"
#include "ns3-quic-backend.h"
#include "ns3-quic-alarm-engine.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3-quic-addr-convert.h"
#include "ns3-quic-tag.h"
#include "ns3-quic-myclient.h"
#include "ns3-quic-clock.h"
#include "ns3-quic-flags.h"
#include "ns3-quic-private.h"
#include <iostream>
#include <string.h>

namespace ns3{

NS_LOG_COMPONENT_DEFINE("MyQuicClient");
class MyQuicClientImpl:public quic::Ns3QuicPollServer,
public quic::Ns3PacketWriter::Delegate,
public quic::QuicSession::Visitor{
public:
    MyQuicClientImpl(MyQuicClient *outer,quic::BackendType backend_type,quic::CongestionControlType cc);
    ~MyQuicClientImpl() override;
    void OnIncomingData(quic::QuicSocketAddress self_addr,
                        quic::QuicSocketAddress peer_addr,
                        const char *buffer, size_t sz);
    //from Ns3QuicPollServer
    bool CreateUDPSocketAndBind(quic::QuicSocketAddress server_address,
                                quic::QuicIpAddress bind_to_address,
                                int bind_to_port) override;
    quic::QuicSocketAddress GetLatestClientAddress() const override;
    void RegisterFD(quic::Ns3PacketInCallback *cb) override;
    void UnregisterFD(quic::Ns3PacketInCallback *cb) override;
    quic::QuicPacketWriter* CreateQuicPacketWriter() override;
    //from Ns3PacketWriter::Delegate
    int WritePacket(const char* buffer,
                    size_t buf_len,
                    const quic::QuicIpAddress& self_address,
                    const quic::QuicSocketAddress& peer_address) override;
    void OnWriterDestroy() override {}
    //from Session::Visitor
    // Called when the connection is closed after the streams have been closed.
    void OnConnectionClosed(quic::QuicConnectionId server_connection_id,
                            quic::QuicErrorCode error,
                            const std::string& error_details,
                            quic::ConnectionCloseSource source) override;

    // Called when the session has become write blocked.
    void OnWriteBlocked(quic::QuicBlockedWriterInterface* blocked_writer) override;

    // Called when the session receives reset on a stream from the peer.
    void OnRstStreamReceived(const quic::QuicRstStreamFrame& frame) override;

    // Called when the session receives a STOP_SENDING for a stream from the
    // peer.
    void OnStopSendingReceived(const quic::QuicStopSendingFrame& frame) override;

    // Called when a NewConnectionId frame has been sent.
    void OnNewConnectionIdSent(
        const quic::QuicConnectionId& server_connection_id,
        const quic::QuicConnectionId& new_connection_id) override;

    // Called when a ConnectionId has been retired.
    void OnConnectionIdRetired(const quic::QuicConnectionId& server_connection_id) override;
private:
    void InitialAndConnect(quic::QuicSocketAddress self_addr,
                                    quic::QuicSocketAddress peer_addr);
    friend class MyQuicClient;
    MyQuicClient *m_outer=nullptr;
    quic::CongestionControlType m_cc;
    std::unique_ptr<quic::Ns3QuicBackendBase> m_backend;
    std::unique_ptr<quic::Ns3QuicAlarmEngine> m_engine;
    quic::Ns3PacketInCallback *m_packetProcessor=nullptr;
    std::unique_ptr<quic::Ns3QuicClient> m_quicClient;
    quic::QuicClock *m_clock=nullptr;
};
static quic::CongestionControlType GetCongestionType(const char *cc_name){
    quic::CongestionControlType cc=quic::kCubicBytes;
    if(0 == strcmp(cc_name,"bbr")) {
        cc=quic::kBBR;
    }else if(0 == strcmp(cc_name,"bbrv2")) {
        cc=quic::kBBRv2;
    }else if (0 == strcmp(cc_name,"copa")) {
        cc=static_cast<quic::CongestionControlType>(quic::kCopa);
    }else if (0 == strcmp(cc_name,"vegas")) {
        cc=static_cast<quic::CongestionControlType>(quic::kVegas);
    }else if(0 == strcmp(cc_name,"reno")) {
        cc=quic::kRenoBytes;
    }
    return cc;
}
MyQuicClient::MyQuicClient(const char *cc_name, Ptr<Node> node, quic::BackendType backend_type){
    m_impl.reset(new MyQuicClientImpl(this,backend_type,GetCongestionType(cc_name)));
    this->node = node;
    //NS_LOG_INFO ("client start application success");
}

MyQuicClient::~MyQuicClient(){
    m_impl=nullptr;
    m_running=false;
    if(m_socket){
        m_socket->Close();
        m_socket=nullptr;
    }
}

void MyQuicClient::Bind(uint16_t port){
    if (NULL==m_socket){
        m_socket = Socket::CreateSocket (node, UdpSocketFactory::GetTypeId ());
        auto local = InetSocketAddress{Ipv4Address::GetAny (),port};
        auto res = m_socket->Bind (local);
        NS_LOG_INFO ("bind on port " << port);
        NS_ASSERT (res == 0);
        Address addr;
        m_socket->GetSockName(addr);
        InetSocketAddress self_sock_addr=InetSocketAddress::ConvertFrom(addr);
        m_bindPort=self_sock_addr.GetPort();
        m_socket->SetRecvCallback (MakeCallback(&MyQuicClient::RecvPacket,this));
    }
}

void MyQuicClient::set_peer(const Ipv4Address& ip_addr,uint16_t port){
    m_peerIp=ip_addr;
    m_peerPort=port;
    std::stringstream ss;
    ss << Ipv4Address::ConvertFrom (ip_addr);
    NS_LOG_INFO ("[MyQuicClient:set_peer] client connect to addr " << ss.str());
}

InetSocketAddress MyQuicClient::GetLocalAddress() const{
    //Ptr<Node> node=GetNode();
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    Ipv4Address local_ip = ipv4->GetAddress (1, 0).GetLocal();
    return InetSocketAddress{local_ip,m_bindPort}; 
}

void MyQuicClient::RecvPacket(Ptr<Socket> socket){
    if (!m_running) {return ;}

    Address remoteAddr;
    auto packet = socket->RecvFrom (remoteAddr);
    uint32_t length=packet->GetSize();
    //NS_LOG_INFO(this<<" MyQuicClient::recv "<<ns3_peer_sock_addr.GetIpv4()<<" "<<length);
    //Ns3QuicTag tag;
    //packet->RemovePacketTag(tag);
    char buffer[1024 * 2]={'\0'};
    packet->CopyData((uint8_t*)buffer,length);
    if(m_impl){
        InetSocketAddress ns3_self_sock_addr(m_bindIp,m_bindPort);
        quic::QuicSocketAddress self_addr=GetQuicSocketAddr(ns3_self_sock_addr);
        InetSocketAddress ns3_peer_sock_addr=InetSocketAddress::ConvertFrom(remoteAddr);
        quic::QuicSocketAddress peer_addr=GetQuicSocketAddr(ns3_peer_sock_addr);
        NS_ASSERT((ns3_peer_sock_addr.GetIpv4()==m_peerIp)&&(ns3_peer_sock_addr.GetPort()==m_peerPort));
        m_impl->OnIncomingData(self_addr,peer_addr,buffer,(size_t)length);
    }
}

void 
MyQuicClient::SendToNetwork(const char *buffer,size_t sz){
    if(!m_socket) {return ;}
    Time now=Simulator::Now();
    InetSocketAddress remote(m_peerIp,m_peerPort);
    Ptr<Packet> packet = Create<Packet>((const uint8_t*)buffer,sz);
    //NS_LOG_INFO(this<<" MyQuicClient::send "<<remote.GetIpv4()<<" "<<remote.GetPort()<<" "<<now.GetSeconds());
    m_seq++;
    m_socket->SendTo(packet,0,remote);
}

Ptr<ns3::CircularBuffer>
MyQuicClient::GetBuffer() {
    if ( ((quic::MyClientBackend*)(m_impl->m_backend. get ()))->endpoint_ != nullptr) {
            return ((quic::MyClientBackend*)(m_impl->m_backend. get ()))->endpoint_->GetBuffer ();
    }
    return nullptr;
}

void 
MyQuicClient::Start () {
    m_running=true;
    m_bindIp=GetLocalAddress().GetIpv4();
    if(m_impl){
        InetSocketAddress ns3_peer_sock_addr(m_peerIp,m_peerPort); 
        InetSocketAddress ns3_self_sock_addr(m_bindIp,m_bindPort);
        quic::QuicSocketAddress self_addr=GetQuicSocketAddr(ns3_self_sock_addr);
        quic::QuicSocketAddress peer_addr=GetQuicSocketAddr(ns3_peer_sock_addr);
        m_impl->InitialAndConnect(self_addr,peer_addr);
        NS_LOG_INFO ("[MyQuicClient::Start] m_impl start");
    }
}

size_t 
MyQuicClient::Send (const char* buffer, size_t len, bool fin) {
    size_t wlen = 0;
    if (dynamic_cast<quic::MyClientBackend*>(m_impl->m_backend. get ()) != nullptr) {
        if ( ((quic::MyClientBackend*)(m_impl->m_backend. get ()))->endpoint_ != nullptr) {
            wlen = ((quic::MyClientBackend*)(m_impl->m_backend. get ()))->endpoint_->Send(buffer, len, fin);
        }
    }
    return wlen;
}

MyQuicClientImpl::MyQuicClientImpl(MyQuicClient *outer,quic::BackendType backend_type,quic::CongestionControlType cc):
m_outer(outer),
m_cc(cc){
    m_clock=quic::Ns3QuicClock::Instance();
    m_engine.reset(new quic::Ns3QuicAlarmEngine(quic::Perspective::IS_CLIENT));
    if(quic::BackendType::HELLO_BIDI==backend_type){
        m_backend.reset(new quic::HelloClientBackend(m_engine.get(),true));
    }
    if (quic::BackendType::BANDWIDTH==backend_type) {
        m_backend.reset(new quic::BandwidthServerBackend(m_engine.get()));
    }
    if (quic::BackendType::MYBACKEND==backend_type) {
        NS_LOG_INFO ("client create mybackend");
        m_backend.reset(new quic::MyClientBackend(m_engine.get()));
    }
}

MyQuicClientImpl::~MyQuicClientImpl(){
    if(m_quicClient){
         m_quicClient->Disconnect();
    }
    m_quicClient=nullptr;
    m_backend=nullptr;
    m_engine=nullptr;
}

void MyQuicClientImpl::OnIncomingData(quic::QuicSocketAddress self_addr,
                    quic::QuicSocketAddress peer_addr,
                    const char *buffer, size_t sz){
    quic::QuicTime now=m_clock->Now();
    quic::QuicReceivedPacket quic_packet(buffer,sz,now);
    if(m_packetProcessor){
        m_packetProcessor->ProcessPacket(self_addr,peer_addr,quic_packet);
    }
}
                    
bool MyQuicClientImpl::CreateUDPSocketAndBind(quic::QuicSocketAddress server_address,
                            quic::QuicIpAddress bind_to_address,
                            int bind_to_port){
    //NS_LOG_INFO(server_address.ToString()<<" "<<bind_to_address.ToString()<<" "<<bind_to_port);
    return true;
}
quic::QuicSocketAddress MyQuicClientImpl::GetLatestClientAddress() const{
    InetSocketAddress ns3_socket_addr=m_outer->GetLocalAddress();
    return GetQuicSocketAddr(ns3_socket_addr);
}
void MyQuicClientImpl::RegisterFD(quic::Ns3PacketInCallback *cb){
    m_packetProcessor=cb;
    m_packetProcessor->OnRegistration();
    
}
void MyQuicClientImpl::UnregisterFD(quic::Ns3PacketInCallback *cb){
    if(m_packetProcessor==cb){
        cb->OnUnregistration();
        m_packetProcessor=nullptr;
    }
    
}
quic::QuicPacketWriter* MyQuicClientImpl::CreateQuicPacketWriter(){
    quic::QuicPacketWriter *writer=new quic::Ns3PacketWriter(this);
    return writer;
}
int MyQuicClientImpl::WritePacket(const char* buffer,
size_t buf_len,
const quic::QuicIpAddress& self_address,
const quic::QuicSocketAddress& peer_address){
    m_outer->SendToNetwork(buffer,buf_len);
    return buf_len;
}

void MyQuicClientImpl::OnConnectionClosed(quic::QuicConnectionId server_connection_id,
                        quic::QuicErrorCode error,
                        const std::string& error_details,
                        quic::ConnectionCloseSource source){
    NS_LOG_ERROR("OnConnectionClosed "<<error_details);
}
void MyQuicClientImpl::OnWriteBlocked(quic::QuicBlockedWriterInterface* blocked_writer){
    
}
void MyQuicClientImpl::OnRstStreamReceived(const quic::QuicRstStreamFrame& frame){
    
}
void MyQuicClientImpl::OnStopSendingReceived(const quic::QuicStopSendingFrame& frame){
    
}
void  MyQuicClientImpl::OnNewConnectionIdSent(
        const quic::QuicConnectionId& server_connection_id,
        const quic::QuicConnectionId& new_connection_id){
            
}
void MyQuicClientImpl::OnConnectionIdRetired(const quic::QuicConnectionId& server_connection_id){
    
}

void MyQuicClientImpl::InitialAndConnect(quic::QuicSocketAddress self_addr,
                                    quic::QuicSocketAddress peer_addr){
    if(m_quicClient){return ;}

    quic::ParsedQuicVersionVector versions = quic::CurrentSupportedVersions();

    std::string quic_version_string =quic::get_quic_version();
    if (quic::get_quic_ietf_draft()) {
        quic::QuicVersionInitializeSupportForIetfDraft();
        versions = {};
        for (const quic::ParsedQuicVersion& version : quic::AllSupportedVersions()) {
            if (version.HasIetfQuicFrames() &&
                version.handshake_protocol == quic::PROTOCOL_TLS1_3) {
            versions.push_back(version);
            }
        }
        quic::QuicEnableVersion(versions[0]);
    
    } else if (!quic_version_string.empty()) {
        quic::ParsedQuicVersion parsed_quic_version =
            quic::ParseQuicVersionString(quic_version_string);
        if (parsed_quic_version.transport_version ==
            quic::QUIC_VERSION_UNSUPPORTED) {
            return ;
        }
        versions = {parsed_quic_version};
        quic::QuicEnableVersion(parsed_quic_version);
    }
    
    if (quic::get_force_version_negotiation()) {
        versions.insert(versions.begin(),
                        quic::QuicVersionReservedForNegotiation());
    }
    
    std::unique_ptr<quic::ProofVerifier> proof_verifier=std::make_unique<quic::FakeProofVerifier>();
    quic::QuicServerId server_id(peer_addr.host().ToString(),
                                 peer_addr.port());
    m_quicClient.reset(new quic::Ns3QuicClient(this,peer_addr,server_id,
                       versions,std::move(proof_verifier),m_backend->get_engine(),m_backend.get(),this,m_cc));
    m_quicClient->set_bind_to_address(self_addr.host());
    m_quicClient->set_initial_max_packet_length(quic::kDefaultMaxPacketSize);
    if(!m_quicClient->Initialize()){
        NS_LOG_ERROR("failed to initial client");
        return ;
    }
    m_quicClient->AsynConnect();
}

}
