#include <cstdint> // Include this header for uint8_t
#include <vector>
#include <string>

namespace ns3 {
    //chunk size
    uint16_t k = 100;
    //uint16_t vsize = 1000;
    uint16_t BASESIZE = 1024;
    std::vector<std::string> commands = {"REQUESTV", "RESPONSEV", "AGGREGATE"};
}; /*namespace ns3*/