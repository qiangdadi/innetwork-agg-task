#pragma once
#include <memory>
#include <memory.h>
#include "ns3/simulator.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/callback.h"
#include "ns3/ns3-quic-public.h"
#include "circularbuffer.h"
namespace ns3{
class MyQuicServerImpl;
class MyQuicServer : public BaseClass {
public:
    MyQuicServer(Ptr<Node> node, quic::BackendType type=quic::BackendType::MYBACKEND);
    ~MyQuicServer();
    void Bind(uint16_t port);
    InetSocketAddress GetLocalAddress() const;
    void SendToNetwork(const char *buffer,size_t sz,const InetSocketAddress& dest);
    Ptr<ns3::CircularBuffer> GetBuffer ();
    void Start ();
    size_t Send (const char* buffer, size_t len, bool fin);

private:
    void RecvPacket(Ptr<Socket> socket);
    std::unique_ptr<MyQuicServerImpl> m_impl;
    Ptr<Socket> m_socket;
    Ipv4Address m_bindIp;
    uint16_t m_bindPort=0;
    bool m_running=false;
    uint64_t m_seq=1;
    Ptr<Node> node = nullptr;
};
}
