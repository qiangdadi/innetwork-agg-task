#include "circularbuffer.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("CircularBuffer");
    NS_OBJECT_ENSURE_REGISTERED (CircularBuffer);

    TypeId 
    CircularBuffer::GetTypeId(void) {
        static TypeId tid = TypeId ("ns3::CircularBuffer")
            .SetParent<Object> ()
            .SetGroupName("quic")
            .AddConstructor<CircularBuffer> ()
        ;
        return tid;
    }

    // 构造函数
    CircularBuffer::CircularBuffer() {
        size_t capacity = 1024 * 10;
        init(capacity);
    }

    // 初始化缓冲区
    void CircularBuffer::init(size_t capacity) {
        this->capacity = capacity;
        this->size = 0;
        this->read_pos = 0;
        this->write_pos = 0;
        this->buffer = (uint8_t*)malloc(capacity);
    }

    // 析构函数
    CircularBuffer::~CircularBuffer() {
        free(this->buffer);
    }

    // 写入数据到缓冲区
    size_t CircularBuffer::write(uint8_t *data, size_t bytes) {
        // if (bytes > (this->capacity - this->size)) {
        //     bytes = this->capacity - this->size; // 防止溢出
        // }
        NS_ASSERT (bytes <= (this->capacity - this->size));
        //NS_LOG_INFO ("write buffer");
        size_t first_part = this->capacity - this->write_pos;
        if (bytes > first_part) {
            memcpy(this->buffer + this->write_pos, data, first_part);
            memcpy(this->buffer, data + first_part, bytes - first_part);
            //this->write_pos = bytes - first_part;
            //NS_LOG_INFO ("memcpy 1 error");
        } else {
            memcpy(this->buffer + this->write_pos, data, bytes);
            //NS_LOG_INFO ("memcpy 2 error");
        }
        this->write_pos = (this->write_pos + bytes) % this->capacity;
        this->size += bytes;
        NS_LOG_INFO ("[CircularBuffer:write] buffer write end");
        return bytes;
    }

    // 从缓冲区读取数据
    size_t 
    CircularBuffer::read(uint8_t *dest, size_t bytes) {
        if (bytes > this->size) {
            bytes = this->size; // 防止读取超过现有数据
        }

        size_t first_part = this->capacity - this->read_pos;
        if (bytes > first_part) {
            memcpy(dest, this->buffer + this->read_pos, first_part);
            memcpy(dest + first_part, this->buffer, bytes - first_part);
            this->read_pos = bytes - first_part;
        } else {
            memcpy(dest, this->buffer + this->read_pos, bytes);
            this->read_pos = (this->read_pos + bytes) % this->capacity;
        }
        this->size -= bytes;
        return bytes;
    }

    size_t 
    CircularBuffer::getSize () {
        return this->size;
    }

    uint8_t *
    CircularBuffer::getData () {
        return this->buffer;
    }

    uint16_t
    CircularBuffer::getHeadUint16 () {
        uint16_t dataLen = 0;
        std::memcpy (&dataLen, this->buffer + this->read_pos, sizeof (uint16_t));
        //NS_LOG_INFO ("[CircularBuffer:getHeadUint16] datalen = " << dataLen << "read_pos = " << this->read_pos);
        NS_ASSERT (dataLen <= this->capacity);
        return dataLen;
    }

}; /*namespace ns3*/