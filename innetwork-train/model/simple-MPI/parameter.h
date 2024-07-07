#ifndef PARAMETER_H
#define PARAMETER_H

#include <vector>
#include <string>

namespace ns3 {

    //chunk size
    extern uint16_t k;
    extern uint16_t BASESIZE;
    //extern uint16_t vsize;
    extern std::vector<std::string> commands;
    enum COMMAND {
        REQUESTV,
        RESPONSEV,
        AGGREGATE,
        COMMAND_COUNT
    };

    //struch data chunk
    struct MyChunk {
        //count the proucer
        uint8_t count;
        //data
        std::vector<uint64_t> data;
        MyChunk () {
            count = 0;
            data = std::vector<uint64_t>(k, 0);
        }
    };

}; /*namespace ns3*/

#endif /*PARAMETER_H*/