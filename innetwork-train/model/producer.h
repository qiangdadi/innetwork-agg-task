/*
* all the data are k:v format;  ----> packet (|dataLen|command (16)|offset(uint16)|v|)
* k is a command, v is potential value;
* command: 
* REQUESTV (request vector) : size (vector size)
* vector split:
* REPONSEV (reponse vector) : vec (vector 1-100)
* REPONSEV (reponse vector) : vec (vector 1-100)
* .  .  .
* .  .  .
* REPONSEV (reponse vector) : vec (vector 901-1000)----->end
*/

#ifndef PRODUCER_H
#define PRODUCER_H

#include "ns3/simpleMPI.h"

namespace ns3 {

class Producer : public Application {
        uint16_t m_peerPort;
        uint16_t basetime;
        std::string cc_name;
        Ptr<SimpleMPI> cMPI;
    
    public:
        static TypeId GetTypeId (void);
        Producer();
        virtual ~Producer ();
        void SetupProducer (uint16_t port, uint16_t itr, uint8_t rank, std::vector<Address> &sGroup, 
                                        std::vector<Address> &cGroup, uint16_t basetime, std::string cc_name);

    protected:
        void DoDispose ();

    private:
        void StartApplication ();
        void StopApplication ();
        
}; /*Producer class*/

}; /*namespace ns3*/

#endif /*PRODUCER_H*/