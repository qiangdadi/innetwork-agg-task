#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <cstddef>
#include <cstring>
#include <cstdlib>
#include "ns3/object.h"
#include "ns3/log.h"

namespace ns3 {
    class CircularBuffer : public Object{
    public:
        CircularBuffer();
        ~CircularBuffer();
        static TypeId GetTypeId(void);

        size_t write(uint8_t *data, size_t bytes);
        size_t read(uint8_t *dest, size_t bytes);
        size_t getSize ();
        uint8_t *getData ();
        uint16_t getHeadUint16 ();

    private:
        uint8_t *buffer;
        size_t capacity;
        size_t size;
        size_t read_pos;
        size_t write_pos;

        void init(size_t capacity);
    };

}; /*namespace ns3*/

#endif // CIRCULARBUFFER_H