#ifndef UTILS_H
#define UTILS_H

#include <fstream>
#include "ns3/packet.h"
#include "ns3/ipv4-address.h"
#include "ns3/parameter.h"

namespace ns3 {

//void SendData (Ptr<BaseProtocol> transport, uint8_t *buffer, uint16_t len);

uint16_t PraseHeader (uint8_t *buffer, std::string &command, uint16_t &dataLen, uint16_t &offset);

void PraseVector (uint8_t *buffer, uint16_t bufferSize, std::vector<uint64_t> &vec);

void PraseInt16 (uint8_t *buffer, uint16_t &size);

uint16_t CreateRequestV (uint8_t *buffer, const std::string &command, uint16_t size);

uint16_t CreatResponseV (uint8_t *buffer, const std::string &command, const std::vector<uint64_t> &vec, 
                            uint16_t offset);

uint16_t CreatResponseVK (uint8_t *buffer, const std::string &command, const std::vector<uint64_t> &vec, 
                            uint16_t offset);

void Addr2Str (Address addr, std::string &str);

}; /*namespace ns3*/

#endif /*UTILS_H*/