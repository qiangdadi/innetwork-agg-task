/*
* all the data are k:v format;
* k is a command, v is potential value;
* command: 
* REQUESTV (request vector) : size (vector size)
* REPONSEV (reponse vector) : vec (vector)
*/

#ifndef CONSUMER_H
#define CONSUMER_H

#include "ns3/simpleMPI.h"
#include <fstream>

namespace ns3 {

class Consumer : public Application {
        //std::vector<uint64_t> avg;
        uint16_t m_peerPort;
        uint16_t basetime;
        std::string cc_name;
        Ptr<SimpleMPI> sMPI;

    public:
        static TypeId GetTypeId (void);
        Consumer ();
        virtual ~Consumer ();
        void SetupConsumer (uint16_t port, uint16_t itr, uint8_t rank, uint16_t vsize, std::vector<Address> &sGroup, 
                                        std::vector<Address> &cGroup, uint16_t basetime, std::string cc_name);

    protected:
        void DoDispose ();

    private:
        void StartApplication ();
        void StopApplication ();
        //void WriteResult ();
        //void SendRequestAll ();
       // void ReceiveData (Ptr<Socket> socket);
        // void SendRequest (Address ip, std::string fill, uint16_t size);
        // void AllRequest (std::string fill, uint16_t size);
        // void InitilizeConsumer ();
        // void ReceiveVector (Address from, uint8_t *recv);

}; /*Consumer class*/

}; /*namespace ns3*/


#endif /*CONSUMER_H*/