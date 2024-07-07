#ifndef SIMPLE_MPI_H
#define SIMPLE_MPI_H

#include <functional>
#include <unordered_map>
#include "ns3/utils.h"
#include "ns3/object.h"
#include "ns3/vectorop.h"
#include "ns3/parameter.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/quic-module.h"
#include "ns3/ipv4-address.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

namespace ns3 {
    class SimpleMPI : public Object {
            bool asEnd;
            uint8_t rank; // process identification and used to generate random number
            uint16_t m_peerPort; // remote port
            //bool m_serverMode;
            uint16_t vsize;
            Ptr<Node> node;
            uint16_t curIter;
            uint16_t maxIteration;
            std::ofstream outFile; //save result for server
            Time startTime; // record each start time

            std::vector<uint64_t> avg; // result
            std::vector<Address> cGroup; // communicator group as cliet to connect to remote
            std::vector<Address> sGroup; // communicator group as server to connect to remote
            std::unordered_map<uint16_t, MyChunk> chunkMap; // for Synchronous processing
            //std::unordered_map <std::string, MyQuicClient*> clientPool; // client pool
            //std::unordered_map <std::string, MyQuicServer*> serverPool; // client pool
            std::unordered_map <std::string, BaseClass*> socketPool;
            //command processing function
            std::unordered_map <std::string, std::function<void(std::string, uint8_t*)>> func_map;

        public:
            static TypeId GetTypeId (void);
            SimpleMPI ();
            ~SimpleMPI ();
            void SetupMPI (uint16_t port, uint16_t itr, uint8_t rank, std::vector<Address> &sGroup, 
                                        std::vector<Address> &cGroup, Ptr<Node> node, bool asEnd);
            void CreateSocketPool (std::string cc_name);
            void ReceiveDataFrom (std::string fromStr);
            void ReceiveDataFromAll ();
            void HandleResponseV (std::string fromStr, uint8_t *buffer);
            void HandleRequestV (std::string fromStr, uint8_t *buffer);
            void SendRequestVTo (std::string toStr, uint16_t size);
            void SendRequestVToAll (uint16_t size);
            void SendResponseVToP ();
            void AVG ();
            void SendResponseVTo (std::string toStr, std::vector<uint64_t> &vec);
            void SetVSize (uint16_t size);
            void AvgEnd (uint16_t size);
            
            bool GetAsEnd ();
            void SetAsEnd (bool v);
            void ClearChunkMap ();
            void WriteChunk2Vec (std::vector<uint64_t> &vec, uint16_t size);
            void SaveResult ();
            void SetOutFile (const std::string fileName);
    };

}; /*namespace ns3*/

#endif