#ifndef AGGREGATOR_H
#define AGGREGATOR_H

/*
* all the data are k:v format;  ----> packet (|dataLen|command (16)|offset(uint16)|v|)
* k is a command, v is potential value;
* command: 
* AGGREGATE (request vector) : size (vector size)
*/
#include "ns3/simpleMPI.h"
#include <fstream>

namespace ns3 {

    class Aggregator : public Application {
        uint16_t m_peerPort;
        uint16_t basetime;
        std::string cc_name;
        Ptr<SimpleMPI> cMPI;

        public:
            static TypeId GetTypeId(void);
            Aggregator();
            ~Aggregator();
            void SetupAggregator (uint16_t port, uint16_t itr, uint8_t rank, std::vector<Address> &sGroup, 
                                        std::vector<Address> &cGroup, uint16_t basetime, std::string cc_name);

        protected:
            void DoDispose();

        private:
            void StartApplication();
            void StopApplication();
            void ReceiveData ();
            void SendRequest ();
            void SendVector ();
    }; /*Aggregator class*/

}; /*namespace ns3*/


#endif /*AGGREGATOR_H*/