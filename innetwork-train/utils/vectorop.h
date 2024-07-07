#ifndef VECTOROP_H
#define VECTOROP_H

#include <vector>
#include <cstdint>  
#include <cstdlib>  
#include <cstring>
#include <ctime>
#include "ns3/ipv4-address.h"   
#include "ns3/utils.h"
#include "ns3/parameter.h"

namespace ns3 {

void SumVector (std::vector<uint64_t> &sum, std::vector<uint64_t> &vec1, 
                    std::vector<uint64_t> &vec2);

void AvgVector (std::vector<uint64_t> &sum, uint8_t count);

void GenetrateRandomVector (std::vector<uint64_t> &vec, uint8_t rank);

// void SerializeVector (const std::vector<int64_t> &vec, uint8_t *buffer, uint16_t bufferSzie);

// void DeserializeVector (std::vector<int64_t> &vec, uint8_t *buffer);

void DeepVectorCopy (std::vector<Address> &vec1, 
                    const std::vector<Address> &vec2);


} /*namespace ns3*/

#endif /*VECTOROP_H*/