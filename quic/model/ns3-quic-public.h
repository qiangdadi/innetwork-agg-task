#pragma once
#include <stdint.h>
#include "circularbuffer.h"
#include "ns3/ptr.h"
namespace quic{
enum class BackendType{
HELLO_UNIDI,
HELLO_BIDI,
BANDWIDTH,
MYBACKEND
};
void ContentInit(const char v);
}

class BaseClass {
    public :
    virtual size_t Send (const char* buffer, size_t len, bool fin) = 0;
    virtual ns3::Ptr<ns3::CircularBuffer> GetBuffer () = 0;
};
