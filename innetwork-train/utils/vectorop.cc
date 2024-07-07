#include "vectorop.h"

namespace ns3 {

void 
SumVector(std::vector<uint64_t> &sum, std::vector<uint64_t> &vec1, 
                std::vector<uint64_t> &vec2) {
    for (uint8_t i = 0; i < uint8_t(sum. size ()); ++i) 
        sum[i] = vec1[i] + vec2[i];

}

void 
AvgVector (std::vector<uint64_t> &sum, uint8_t count) {
    for (uint8_t i = 0; i < sum. size (); ++i)
        sum[i] /= count;
}

void
GenetrateRandomVector (std::vector<uint64_t> &vec, uint8_t rank) {
    // seed the random number generator
    std::srand(std::time (nullptr) + rank);
    
    // generate vSize random uint64_t integers
    for (uint16_t i = 0; i < vec. size (); ++i) 
        vec[i] = (static_cast<uint64_t> (std::rand ()) << 32) | std::rand ();

    //std::cout << "[********************] first element = " << vec[0] << std::endl;

}

void 
DeepVectorCopy(std::vector<Address> &vec1, 
                    const std::vector<Address> &vec2) {
    for (uint16_t i = 0; i < uint16_t (vec2. size ()); ++i) {
        vec1. push_back (vec2[i]);
    }
}

} /*namespace ns3*/