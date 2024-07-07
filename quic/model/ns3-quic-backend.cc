#include <memory>
#include "gquiche/quic/platform/api/quic_logging.h"

#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/simulator.h"
#include "ns3-quic-backend.h"
#include "ns3-quic-alarm-engine.h"
#include "ns3-quic-session-base.h"
#include "ns3-transport-stream.h"
#include "ns3/ptr.h"
using namespace ns3;
NS_LOG_COMPONENT_DEFINE("Ns3QuicBackendBase");
namespace quic{
Ns3QuicEndpointBase::Ns3QuicEndpointBase(Ns3QuicBackendBase* backend):
backend_(backend){}
    
Ns3QuicBackendBase::Ns3QuicBackendBase(Ns3QuicAlarmEngine* engine):
engine_(engine){}

HelloClientEndpoint::HelloClientEndpoint(Ns3QuicBackendBase* backend,bool bidi_channel):
Ns3QuicEndpointBase(backend),
bidi_channel_(bidi_channel){}
HelloClientEndpoint::~HelloClientEndpoint(){
    ns3::Time now=ns3::Simulator::Now();
    if(channel_){
        channel_->set_notifier(nullptr);
        channel_=nullptr;
    }
    NS_LOG_INFO("HelloClientEndpoint::dtor "<<now.GetSeconds());
}

void HelloClientEndpoint::OnIncomingBidirectionalStream(Ns3TransportStream *stream){
    
}
void HelloClientEndpoint::OnIncomingUnidirectionalStream(Ns3TransportStream *stream){
    
}
void HelloClientEndpoint::OnSessionReady(Ns3QuicSessionBase *session){
    ready_=true;
    session_=session;
    Ns3TransportStream *stream=nullptr;
    std::unique_ptr<Ns3QuicChannelBase> channel_ptr;
    if(bidi_channel_){
        stream=session_->OpenOutgoingBidirectionalStream();
        channel_=new HelloBidiChannel(stream);
        channel_ptr.reset(channel_);
        channel_ptr->set_notifier(this);
        stream->set_visitor(std::move(channel_ptr));
        ((HelloBidiChannel*)(channel_))->SendHello();
    }else{
        stream=session_->OpenOutgoingUnidirectionalStream();
        channel_=new HelloUnidiWriteChannel(stream);
        channel_ptr.reset(channel_);
        channel_ptr->set_notifier(this);
        stream->set_visitor(std::move(channel_ptr));
        ((HelloUnidiWriteChannel*)(channel_))->SendHello();
    }
}
void HelloClientEndpoint::OnSessionDestroy(Ns3QuicSessionBase *session){
    ns3::Time now=ns3::Simulator::Now();
    NS_LOG_INFO("HelloClientEndpoint::OnSessionDestroy "<<now.GetSeconds());
    session_=nullptr;
}
void HelloClientEndpoint::OnChannelDestroy(Ns3QuicChannelBase *channel){
    ns3::Time now=ns3::Simulator::Now();
    NS_LOG_INFO("HelloClientEndpoint::OnChannelDestroy "<<now.GetSeconds());
    channel_=nullptr;
}

EchoServerEndpoint::EchoServerEndpoint(Ns3QuicBackendBase* backend):
Ns3QuicEndpointBase(backend){}
EchoServerEndpoint::~EchoServerEndpoint(){
    ns3::Time now=ns3::Simulator::Now();
    if(channel_){
        channel_->set_notifier(nullptr);
        channel_=nullptr;
    }
    NS_LOG_INFO("EchoServerEndpoint::dtor "<<now.GetSeconds());
}

void EchoServerEndpoint::OnIncomingBidirectionalStream(Ns3TransportStream *stream){
    channel_=new EchoBidiChannel(stream);
    std::unique_ptr<Ns3QuicChannelBase> channel_ptr;
    channel_ptr.reset(channel_);
    channel_ptr->set_notifier(this);
    stream->set_visitor(std::move(channel_ptr));
}
void EchoServerEndpoint::OnIncomingUnidirectionalStream(Ns3TransportStream *stream){
    channel_=new HelloUnidiReadChannel(stream);
    std::unique_ptr<Ns3QuicChannelBase> channel_ptr;
    channel_ptr.reset(channel_);
    channel_ptr->set_notifier(this);
    stream->set_visitor(std::move(channel_ptr));
}
void EchoServerEndpoint::OnSessionReady(Ns3QuicSessionBase *session){
    ready_=true;
    session_=session;
}
void EchoServerEndpoint::OnSessionDestroy(Ns3QuicSessionBase *session){
    ns3::Time now=ns3::Simulator::Now();
    NS_LOG_INFO("EchoServerEndpoint::OnSessionDestroy "<<now.GetSeconds());
    session_=nullptr;
}
void EchoServerEndpoint::OnChannelDestroy(Ns3QuicChannelBase *channel){
    ns3::Time now=ns3::Simulator::Now();
    NS_LOG_INFO("EchoServerEndpoint::OnChannelDestroy "<<now.GetSeconds());
    channel_=nullptr;
}

HelloClientBackend::HelloClientBackend(Ns3QuicAlarmEngine* engine,bool bidi_channel):
Ns3QuicBackendBase(engine),
bidi_channel_(bidi_channel){}

Ns3QuicEndpointBase* HelloClientBackend::CreateEndpoint(Ns3QuicSessionBase *session){
    return new HelloClientEndpoint(this,bidi_channel_);
}

HelloServerBackend::HelloServerBackend(Ns3QuicAlarmEngine* engine):
Ns3QuicBackendBase(engine){}


Ns3QuicEndpointBase* HelloServerBackend::CreateEndpoint(Ns3QuicSessionBase *session){
    return new EchoServerEndpoint(this);
}


BandwidthClientEndpoint::BandwidthClientEndpoint(Ns3QuicBackendBase* backend):
Ns3QuicEndpointBase(backend){}
BandwidthClientEndpoint::~BandwidthClientEndpoint(){
    ns3::Time now=ns3::Simulator::Now();
    if(channel_){
        channel_->set_notifier(nullptr);
        channel_=nullptr;
    }
    NS_LOG_INFO("BandwidthClientEndpoint::dtor "<<now.GetSeconds());
}
void BandwidthClientEndpoint::OnIncomingBidirectionalStream(Ns3TransportStream *stream){
    
}
void BandwidthClientEndpoint::OnIncomingUnidirectionalStream(Ns3TransportStream *stream){
    
}
void BandwidthClientEndpoint::OnSessionReady(Ns3QuicSessionBase *session){
    ready_=true;
    session_=session;
    Ns3TransportStream *stream=nullptr;
    std::unique_ptr<Ns3QuicChannelBase> channel_ptr;
    stream=session_->OpenOutgoingUnidirectionalStream();
    channel_=new BandwidthWriteChannel(stream,backend_->get_engine());
    channel_ptr.reset(channel_);
    channel_ptr->set_notifier(this);
    stream->set_visitor(std::move(channel_ptr));
    ((BandwidthWriteChannel*)(channel_))->StartFillData();

}
void BandwidthClientEndpoint::OnSessionDestroy(Ns3QuicSessionBase *session){
    ns3::Time now=ns3::Simulator::Now();
    NS_LOG_INFO("BandwidthClientEndpoint::OnSessionDestroy "<<now.GetSeconds());
    session_=nullptr;
}
void BandwidthClientEndpoint::OnChannelDestroy(Ns3QuicChannelBase *channel){
    ns3::Time now=ns3::Simulator::Now();
    NS_LOG_INFO("BandwidthClientEndpoint::OnChannelDestroy "<<now.GetSeconds());
    channel_=nullptr;
}

BandwidthServerEndpoint::BandwidthServerEndpoint(Ns3QuicBackendBase* backend):
Ns3QuicEndpointBase(backend){}
BandwidthServerEndpoint::~BandwidthServerEndpoint(){
    ns3::Time now=ns3::Simulator::Now();
    if(channel_){
        channel_->set_notifier(nullptr);
        channel_=nullptr;
    }
    NS_LOG_INFO("BandwidthServerEndpoint::dtor "<<now.GetSeconds());
}

void BandwidthServerEndpoint::OnIncomingBidirectionalStream(Ns3TransportStream *stream){
    
}
void BandwidthServerEndpoint::OnIncomingUnidirectionalStream(Ns3TransportStream *stream){
    channel_=new BandwidthReadChannel(stream);
    std::unique_ptr<Ns3QuicChannelBase> channel_ptr;
    channel_ptr.reset(channel_);
    channel_ptr->set_notifier(this);
    stream->set_visitor(std::move(channel_ptr));
}
void BandwidthServerEndpoint::OnSessionReady(Ns3QuicSessionBase *session){
    ready_=true;
    session_=session;
}
void BandwidthServerEndpoint::OnSessionDestroy(Ns3QuicSessionBase *session){
    ns3::Time now=ns3::Simulator::Now();
    NS_LOG_INFO("BandwidthServerEndpoint::OnSessionDestroy "<<now.GetSeconds());
    session_=nullptr;
}
void BandwidthServerEndpoint::OnChannelDestroy(Ns3QuicChannelBase *channel){
    ns3::Time now=ns3::Simulator::Now();
    NS_LOG_INFO("BandwidthServerEndpoint::OnChannelDestroy "<<now.GetSeconds());
    channel_=nullptr;
}

BandwidthClientBackend::BandwidthClientBackend(Ns3QuicAlarmEngine* engine):
Ns3QuicBackendBase(engine){}
Ns3QuicEndpointBase* BandwidthClientBackend::CreateEndpoint(Ns3QuicSessionBase *session){
    return new BandwidthClientEndpoint(this);
}

BandwidthServerBackend::BandwidthServerBackend(Ns3QuicAlarmEngine* engine):
Ns3QuicBackendBase(engine){}
Ns3QuicEndpointBase* BandwidthServerBackend::CreateEndpoint(Ns3QuicSessionBase *session){
    return new BandwidthServerEndpoint(this);
}

MyClientEndpoint::MyClientEndpoint(Ns3QuicBackendBase* backend,bool bidi_channel):
Ns3QuicEndpointBase(backend),
bidi_channel_(bidi_channel){}
MyClientEndpoint::~MyClientEndpoint(){
    ns3::Time now=ns3::Simulator::Now();
    if(channel_){
        channel_->set_notifier(nullptr);
        channel_=nullptr;
    }
    NS_LOG_INFO("[MyClientEndpoint:dtor] "<< now.GetSeconds());
}

void 
MyClientEndpoint::OnIncomingBidirectionalStream(Ns3TransportStream *stream){
    
}
void 
MyClientEndpoint::OnIncomingUnidirectionalStream(Ns3TransportStream *stream){
    
}

void 
MyClientEndpoint::OnSessionReady(Ns3QuicSessionBase *session){
    ready_=true;
    session_=session;
    Ns3TransportStream *stream=nullptr;
    std::unique_ptr<Ns3QuicChannelBase> channel_ptr;
    if(!channel_){
        stream=session_->OpenOutgoingBidirectionalStream();
        channel_=new MyChannel(stream);
        channel_ptr.reset(channel_);
        channel_ptr->set_notifier(this);
        stream->set_visitor(std::move(channel_ptr));
    }
    ns3::Time now=ns3::Simulator::Now();
    NS_LOG_INFO("[MyClientEndpoint:OnSessionReady] try to create a stream " << now.GetSeconds());
}

void 
MyClientEndpoint::OnSessionDestroy(Ns3QuicSessionBase *session){
    ns3::Time now=ns3::Simulator::Now();
    NS_LOG_INFO("[MyClientEndpoint:OnSessionDestroy] " << now.GetSeconds());
    session_=nullptr;
}

void 
MyClientEndpoint::OnChannelDestroy(Ns3QuicChannelBase *channel){
    ns3::Time now=ns3::Simulator::Now();
    NS_LOG_INFO("MyClientEndpoint:OnChannelDestroy "<<now.GetSeconds());
    channel_=nullptr;
}

size_t 
MyClientEndpoint::Send (const char* buffer,size_t len, bool fin) {
    //send logic
    size_t wlen = 0;
    if (channel_) {
        wlen = ((MyChannel*)(channel_))->Send(buffer, len, fin);
        NS_LOG_INFO ("[MyClientEndpoint:Send] channel_ try to send, send len = " << wlen);
    } else {
        NS_LOG_ERROR ("[MyClientEndpoint:Send] channel_ is nullptr");
    }
    return wlen;
}

ns3::Ptr<ns3::CircularBuffer>
MyClientEndpoint::GetBuffer () {
    return ((MyChannel*)(channel_))->GetBuffer ();
}

MyServerEndpoint::MyServerEndpoint(Ns3QuicBackendBase* backend,bool bidi_channel):
Ns3QuicEndpointBase(backend),
bidi_channel_(bidi_channel){}
MyServerEndpoint::~MyServerEndpoint(){
    ns3::Time now=ns3::Simulator::Now();
    if(channel_){
        channel_->set_notifier(nullptr);
        channel_=nullptr;
    }
    NS_LOG_INFO("[MyServerEndpoint:dtor] " << now.GetSeconds());
}

void 
MyServerEndpoint::OnIncomingBidirectionalStream(Ns3TransportStream *stream){
    channel_=new MyChannel(stream);
    std::unique_ptr<Ns3QuicChannelBase> channel_ptr;
    channel_ptr.reset(channel_);
    channel_ptr->set_notifier(this);
    stream->set_visitor(std::move(channel_ptr));
    NS_LOG_INFO("[MyServerEndpoint:OnIncomingBidirectionalStream] create stream");
}

void 
MyServerEndpoint::OnIncomingUnidirectionalStream(Ns3TransportStream *stream){
    
}

void 
MyServerEndpoint::OnSessionReady(Ns3QuicSessionBase *session){
    ready_=true;
    session_=session;
    ns3::Time now=ns3::Simulator::Now();
    NS_LOG_INFO("[MyServerEndpoint:OnSessionReady] " << now.GetSeconds());
}
void 
MyServerEndpoint::OnSessionDestroy(Ns3QuicSessionBase *session){
    ns3::Time now=ns3::Simulator::Now();
    NS_LOG_INFO("[MyServerEndpoint:OnSessionDestroy] " << now.GetSeconds());
    session_=nullptr;
}

void 
MyServerEndpoint::OnChannelDestroy(Ns3QuicChannelBase *channel){
    ns3::Time now=ns3::Simulator::Now();
    NS_LOG_INFO("[MyServerEndpoint:OnChannelDestroy] "<<now.GetSeconds());
    channel_=nullptr;
}

size_t 
MyServerEndpoint::Send (const char* buffer,size_t len, bool fin) {
    //send logic
    size_t wlen = 0;
    if (channel_) {
        wlen = ((MyChannel*)(channel_))->Send(buffer, len, fin);
        NS_LOG_INFO ("[MyServerEndpoint:Send] channel_ try to send, send len = " << wlen);
    } else {
        NS_LOG_ERROR ("[MyServerEndpoint:Send] channel_ is nullptr");
    }
    return wlen;
}

ns3::Ptr<ns3::CircularBuffer>
MyServerEndpoint::GetBuffer () {
    return ((MyChannel*)(channel_))->GetBuffer ();
}

MyClientBackend::MyClientBackend(Ns3QuicAlarmEngine* engine,bool bidi_channel):
Ns3QuicBackendBase(engine),
bidi_channel_(bidi_channel){}

Ns3QuicEndpointBase* 
MyClientBackend::CreateEndpoint(Ns3QuicSessionBase *session){
    endpoint_ = new MyClientEndpoint(this,bidi_channel_);
    NS_LOG_INFO ("[MyClientBackend:CreateEndpoint] create client endpoint");
    return endpoint_;
}

MyServerBackend::MyServerBackend(Ns3QuicAlarmEngine* engine,bool bidi_channel):
Ns3QuicBackendBase(engine),
bidi_channel_(bidi_channel){}

Ns3QuicEndpointBase* 
MyServerBackend::CreateEndpoint(Ns3QuicSessionBase *session){
    endpoint_ = new MyServerEndpoint(this,bidi_channel_);
    NS_LOG_INFO ("[MyServerBackend:CreateEndpoint] create server endpoint");
    return endpoint_;
}

}
