#pragma once
#include <string>
#include <memory>
#include "ns3/event-id.h"
#include "ns3/callback.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/ns3-quic-public.h"
#include "circularbuffer.h"
namespace ns3{
class MyQuicClientImpl;
class MyQuicClient : public BaseClass  {
public:
    MyQuicClient(const char *cc_name, Ptr<Node> node, quic::BackendType backend_type=quic::BackendType::MYBACKEND);
    ~MyQuicClient();
    void Bind(uint16_t port = 0);
    void set_peer(const Ipv4Address& ip_addr,uint16_t port);
    InetSocketAddress GetLocalAddress() const;
    void RecvPacket(Ptr<Socket> socket);
    void SendToNetwork(const char *buffer,size_t sz);
    Ptr<ns3::CircularBuffer> GetBuffer ();
    void Start ();
    size_t Send (const char* buffer, size_t len, bool fin);

private:
    Ptr<Socket> m_socket;
    Ipv4Address m_bindIp;
    uint16_t m_bindPort=0;
    Ipv4Address m_peerIp;
    uint16_t m_peerPort;
    bool m_running=false;
    uint64_t m_seq=1;
    std::unique_ptr<MyQuicClientImpl> m_impl;
    int64_t m_rate=0;
    Ptr<Node> node = nullptr;
};
}
