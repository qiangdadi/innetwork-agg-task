#include "utils.h"

namespace ns3 {

uint16_t 
PraseHeader (uint8_t *buffer, std::string &header, uint16_t &dataLen, uint16_t &offset) {
    // prase dataLen
    std::memcpy (&dataLen, buffer, sizeof (uint16_t));
    //std::cout << "[PraseHeader] dataLen = " << dataLen << std::endl;

    uint16_t startOff = sizeof (uint16_t);
    uint16_t len = 0;
    while (reinterpret_cast<char *> (buffer + startOff + len)[0] != ':' && (len < ( dataLen - startOff))) {
        ++len;

    }
    header = std::string (reinterpret_cast<const char*> (buffer + startOff), len);

    // prase offset
    std::memcpy (&offset, buffer + startOff + len + 1, sizeof (uint16_t));

    return startOff + len + 1 + sizeof (uint16_t);

}

void 
PraseVector (uint8_t *buffer, uint16_t bufferSize, std::vector<uint64_t> &vec) {
    uint64_t ele;
    uint8_t type = sizeof (uint64_t);
    for (uint8_t i = 0; i < (bufferSize / type); ++i) {
        memcpy (&ele, buffer + i * type, type);
        vec[i] = ele;

    }

}

void 
PraseInt16 (uint8_t *buffer, uint16_t &size) {
    std::memcpy (&size, buffer, sizeof (uint16_t));

}

uint16_t 
CreateRequestV (uint8_t *buffer, const std::string &header, uint16_t size) {
    std::string newheader = header + ":";
    uint16_t bufferSize = newheader. size () + sizeof (uint16_t) + sizeof (uint16_t) + sizeof (uint16_t);
    uint16_t offset = 0;
    //uint8_t *buffer = new uint8_t [bufferSize];
    //write dataLen
    std::memcpy (buffer, &bufferSize, sizeof (uint16_t));
    //write key
    std::memcpy (buffer + sizeof (uint16_t), (uint8_t *)newheader. c_str (), newheader. size ());
    //write offset 
    std::memcpy (buffer + sizeof (uint16_t) + + newheader. size (), &offset, sizeof (uint16_t));
    //write value
    std::memcpy (buffer + 2* sizeof (uint16_t) + newheader. size (), &size, sizeof (uint16_t));

    return bufferSize;
}

uint16_t 
CreatResponseV (uint8_t *buffer, const std::string &header, const std::vector<uint64_t> &vec,
                    uint16_t offset) {
    std::string newheader = header + ":";
    uint16_t vSize = vec. size ();
    uint8_t step = sizeof (uint64_t);
    uint16_t end = (vSize - offset < k) ? vSize : (offset + k - 1);
    uint16_t dataLenOff = sizeof (uint16_t);
    uint16_t headerOff = dataLenOff + newheader. size ();
    uint16_t offOff = headerOff + sizeof (uint16_t);
    uint16_t bufferSize = offOff + step * (end - offset + 1);
    //uint8_t *buffer = new uint8_t[bufferSize];

    //std::cout << "end = " << end << "bufferSize = " << bufferSize << std::endl;
    //write dataLen
    std::memcpy (buffer, &bufferSize, sizeof (uint16_t));
    // write key
    std::memcpy (buffer + dataLenOff, (uint8_t *)newheader. c_str (), newheader. size ());
    // write offset
    std::memcpy (buffer + headerOff, &offset, sizeof (uint16_t));
    // write value
    for (uint16_t i = 0; i < (end - offset + 1); ++i) {
        std::memcpy (buffer + offOff + i * step, &vec[offset + i], step);

    }
    // std::cout << "end = " << end << "bufferSize = " << bufferSize << std::endl;
    return bufferSize;
}

uint16_t 
CreatResponseVK (uint8_t *buffer, const std::string &header, const std::vector<uint64_t> &vec,
                    uint16_t offset) {
    std::string newheader = header + ":";
    uint16_t vSize = vec. size ();
    uint8_t step = sizeof (uint64_t);
    uint16_t end = vSize;
    uint16_t dataLenOff = sizeof (uint16_t);
    uint16_t headerOff = dataLenOff + newheader. size ();
    uint16_t offOff = headerOff + sizeof (uint16_t);
    uint16_t bufferSize = offOff + step * (end);
    //uint8_t *buffer = new uint8_t[bufferSize];

    //std::cout << "end = " << end << "bufferSize = " << bufferSize << std::endl;
    //write dataLen
    std::memcpy (buffer, &bufferSize, sizeof (uint16_t));
    // write key
    std::memcpy (buffer + dataLenOff, (uint8_t *)newheader. c_str (), newheader. size ());
    // write offset
    std::memcpy (buffer + headerOff, &offset, sizeof (uint16_t));
    // write value
    for (uint16_t i = 0; i < end; ++i) {
        std::memcpy (buffer + offOff + i * step, &vec[i], step);

    }
    // std::cout << "end = " << end << "bufferSize = " << bufferSize << std::endl;
    return bufferSize;
}

void 
Addr2Str (Address addr, std::string &str) {
    std::stringstream ss;
    ss << Ipv4Address::ConvertFrom (addr);
    str = ss.str();
}

}; /*namespace ns3*/