#include "simpleMPI.h"

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("SimpleMPI");
    NS_OBJECT_ENSURE_REGISTERED (SimpleMPI);

    TypeId 
    SimpleMPI::GetTypeId (void) {
        static TypeId tid = TypeId ("ns3::SimpleMPI")
            .SetParent<Object> ()
            .SetGroupName("innetwork-train")
            .AddConstructor<SimpleMPI> ()
        ;
        return tid;
    };

    SimpleMPI::SimpleMPI () {
        NS_LOG_FUNCTION (this);
    }

    SimpleMPI::~SimpleMPI () {
        NS_LOG_FUNCTION (this);
    }

    void 
    SimpleMPI::SetupMPI (uint16_t port, uint16_t itr, uint8_t rank, std::vector<Address> &sGroup, 
                        std::vector<Address> &cGroup, Ptr<Node> node, bool asEnd) {
        NS_LOG_FUNCTION (this);
        this->asEnd = asEnd;
        this->m_peerPort = port;
        this->node = node;
        this->maxIteration = itr;
        this->curIter = 0;
        this->rank = rank;
        DeepVectorCopy (this->sGroup, sGroup);
        DeepVectorCopy (this->cGroup, cGroup);
        //memory friendly, move 
        //this->aggGroup = std::move(group);
        //NS_LOG_INFO ("aggGroup size = " << aggGroup. size () << " group size = " << group. size () );
        this->func_map[commands[REQUESTV]] = std::bind(&SimpleMPI::HandleRequestV, this, 
                                                std::placeholders::_1, std::placeholders::_2);
        this->func_map[commands[RESPONSEV]] = std::bind(&SimpleMPI::HandleResponseV, this, 
                                                std::placeholders::_1, std::placeholders::_2);
        //this->func_map[commands[AGGREGATE]] = std::bind(&SimpleMPI::HandleAggregate, this, 
                                                //std::placeholders::_1, std::placeholders::_2);
    }

    void
    SimpleMPI::SetVSize (uint16_t size) {
        NS_LOG_FUNCTION (this);
        this->vsize = size;
        avg = std::vector<uint64_t>(size, 0);
    }

    void 
    SimpleMPI::CreateSocketPool (std::string cc_name) {
        NS_LOG_FUNCTION (this);
        std::string addrStr;
        for (uint8_t i = 0; i < this->sGroup. size (); ++i) {
            // setup QUIC
            Addr2Str (sGroup[i], addrStr);
            MyQuicServer* myserver = new MyQuicServer(node);
            myserver->Bind(m_peerPort);
            socketPool[addrStr] = myserver;
            myserver->Start ();
        }

        for (uint8_t i = 0; i < this->cGroup. size (); ++i) {
            MyQuicClient* myclient = new MyQuicClient(cc_name.c_str(), node);
            myclient->Bind (m_peerPort + i + 1);
            myclient->set_peer (Ipv4Address::ConvertFrom (cGroup[i]), m_peerPort);
            Addr2Str (cGroup[i], addrStr);
            socketPool[addrStr] = myclient;
            myclient->Start ();
        }
    }

    void 
    SimpleMPI::ReceiveDataFrom (std::string fromStr) {
        //NS_LOG_FUNCTION (this);

        // std::string fromStr;
        // Addr2Str (from, fromStr);
        // try to read packet from an unknown ip
        NS_ASSERT (socketPool. find (fromStr) != socketPool. end ());
        BaseClass* transport = socketPool[fromStr];

        // to eliminate the effect of auto PACKET-SPLIT operation performed by transport protocol
        uint16_t dataLen = 0;
        uint16_t bufferSize = transport->GetBuffer ()->getSize ();
        
        if (bufferSize > 2) {
            dataLen = transport->GetBuffer ()->getHeadUint16 ();
            // NS_LOG_INFO ("[SimpleMPI:ReceiveDataFrom] Ready Receive Data from " << fromStr <<
            //                 " dataLen = " << dataLen << " current data buffer size = " << bufferSize);
        }

        // buffer has a whole packet, try to read and prase
        while (dataLen <= bufferSize && dataLen > 0) 
        {
            uint16_t bs = dataLen > BASESIZE * 2 ? dataLen : BASESIZE * 2;
            uint8_t *buffer = new uint8_t[bs];
            transport->GetBuffer ()->read (buffer, dataLen);
            std::string key;
            uint16_t offset = 0;
            PraseHeader (buffer, key, dataLen, offset);
            // NS_LOG_INFO ("[SimpleMPI:ReceiveDataFrom] Receive Command : " << key << " len = " << dataLen
            //                 << " from " << fromStr << " current data buffer size = " << transport->GetBuffer ()->getSize ());

            auto it = func_map. find (key);
            if (it != func_map. end ()) {
                it->second(fromStr, buffer);  // call funcation
            } else {
                NS_LOG_INFO ("[SimpleMPI:ReceiveDataFrom] Invalid Command");
            }
            delete [] buffer; //delete buffer

            bufferSize = transport->GetBuffer ()->getSize ();
            if (bufferSize <= 2) 
                break; 
            dataLen = transport->GetBuffer ()->getHeadUint16();
        }
    }

    void 
    SimpleMPI::ReceiveDataFromAll () {
        //NS_LOG_INFO ("[SimpleMPI:ReceiveDataFromAll] receive data from all start");
        for (auto item : socketPool) {
            ReceiveDataFrom (item.first);
        }

        if (!asEnd || cGroup. size () == 0)
            ns3::Simulator::Schedule(ns3::MilliSeconds(30), &SimpleMPI::ReceiveDataFromAll, this);
        //NS_LOG_INFO ("[SimpleMPI:ReceiveDataFromAll] receive data from all end");
    }

    void 
    SimpleMPI::HandleResponseV (std::string fromStr, uint8_t *buffer) {
        NS_LOG_FUNCTION (this);

        // parse vector
        std::vector<uint64_t> vec(k, 0);
        std::string key;
        uint16_t dataLen = 0;
        uint16_t offset = 0;
        uint16_t headerOff = PraseHeader (buffer, key, dataLen, offset);
        //NS_LOG_INFO ("[SimpleMPI:HandleReponseV] Command : " << key << " len = " 
                        //<< dataLen << " offset = " << offset << " headerOff = " << headerOff);

        NS_ASSERT (dataLen != 0);
        PraseVector (buffer + headerOff, dataLen - headerOff, vec);

        // Receive First Vector Chunk, create it
        if (chunkMap. find (offset) == chunkMap. end ()) {
            MyChunk chunk;
            chunkMap[offset] = chunk;
        }

        //NS_ASSERT (vec. size () != 0);
        NS_ASSERT (chunkMap[offset]. data. size () == vec. size ());

        SumVector (chunkMap[offset]. data, chunkMap[offset]. data, vec);
        chunkMap[offset]. count++;
        // Receive all the corresponding Vector Chunk from Peer
        //NS_LOG_INFO ("[SimpleMPI:HandleReponseV] Received chunk count " << int(chunkMap[offset]. count)
                        //<< " cGroupSize " << cGroup. size ());
        if (chunkMap[offset]. count == cGroup. size ()) {
            AvgVector (chunkMap[offset]. data, chunkMap[offset]. count);
            //NS_LOG_INFO ("[SimpleMPI:HandleReponseV] Receive all the corresponding Vector Chunk for offset "
                            //<< offset);
            AvgEnd(vsize);
        }
        //NS_LOG_INFO ("[SimpleMPI:HandleResponseV] HandleResponseV End");
    }

    void 
    SimpleMPI::HandleRequestV (std::string fromStr, uint8_t *buffer) {
        NS_LOG_FUNCTION (this);
        std::string key;
        uint16_t dataLen;
        uint16_t size;
        uint16_t offset;
        uint16_t headerOff = PraseHeader (buffer, key, dataLen, offset);
        
        PraseInt16 (buffer + headerOff, size);
        // NS_LOG_INFO ("[SimpleMPI:HandleRequestV] Command : " << key << " len = " << dataLen <<
        //                 " Request vector Size = " << size);

        // command downword the tree
        this->vsize = size;
        avg = std::vector<uint64_t>(size, 0);
        SendRequestVToAll (size);

        // root node Reponse to the request immediately
        if (cGroup. size () <= 0) {
            std::vector<uint64_t> randVec(size);
            GenetrateRandomVector (randVec, this->rank);
            SendResponseVTo (fromStr, randVec);
            //ns3::Simulator::Schedule(ns3::MilliSeconds(10), &SimpleMPI::SendResponseVTo, this, from, randVec);
        }
    }

    // void 
    // SimpleMPI::HandleAggregate (Address from, uint8_t *buffer) {
    //     NS_LOG_FUNCTION (this);
    //     std::string key;
    //     uint16_t dataLen;
    //     uint16_t size;
    //     uint16_t offset;
    //     uint16_t headerOff = PraseHeader (buffer, key, dataLen, offset);
        
    //     PraseInt16 (buffer + headerOff, size);
    //     //NS_LOG_INFO ("[SimpleMPI:HandleRequestV] Command : " << key << " len = " << dataLen <<
    //                     //" Request vector Size = " << size);
        
    //     // forward the command
    //     SendRequestVToAll (size);
    //     //waiting reponse and reponse to consumer
    // }

    void 
    SimpleMPI::SendRequestVTo (std::string toStr, uint16_t size) {
        NS_LOG_FUNCTION (this);
        // std::string toStr;
        // Addr2Str (to, toStr);
        //NS_LOG_INFO ("[SimpleMPI:SendRequestVTo] Ready Sent Request Data to " << toStr);

        // setup QUIC
        // if (socketPool. find (toStr) == socketPool. end ()) {
        //     Ptr<BaseProtocol> transport = CreateObject<QUICProtocol> ();
        //     //transport->SetupQUIC (1, aggGroup[i], m_peerPort, m_serverMode);
        //     transport->SetRemote (to, m_peerPort);
        //     transport->SetIsServer (m_serverMode);
        //     transport->Connect(this->node);
        //     transport->m_socket->SetRecvCallback (MakeCallback (&SimpleMPI::ReceiveData, this));
        //     socketPool[toStr] = transport;
        // }

        // Sent to an unknown ip
        NS_ASSERT (socketPool. find (toStr) != socketPool. end ());

        uint8_t *buffer = new uint8_t[BASESIZE];
        uint16_t bufferSize = CreateRequestV (buffer, commands[REQUESTV], size);
        //NS_LOG_INFO ("[SimpleMPI:SendRequestVTo] CreateRequestV Size = " << bufferSize);

        //SendData (socketPool[toStr], buffer, bufferSize);
        //Ptr<Packet> p = Create<Packet> (buffer, bufferSize);
        socketPool[toStr]->Send((char *)buffer, bufferSize, false);
        delete [] buffer;
    }

    void
    SimpleMPI::SendRequestVToAll (uint16_t size) {
        NS_LOG_FUNCTION (this);
        //NS_LOG_INFO ("[SimpleMPI:SendRequestVToAll] send request to all");
        for (uint8_t i = 0; i < this->cGroup. size (); ++i) {
            // send request
            std::string toStr;
            Addr2Str (this->cGroup[i], toStr);
            SendRequestVTo (toStr, size);
        }
    }

    void 
    SimpleMPI::SendResponseVTo (std::string toStr, std::vector<uint64_t> &vec) {
        NS_LOG_FUNCTION (this);
        // NS_LOG_INFO ("[SimpleMPI:SendResponseVTo] CreatResponseV ");
        uint16_t pos = 0;
        uint16_t end = vec. size ();
        uint8_t *bufferV = new uint8_t[BASESIZE * 2];
        uint16_t bufferSize = 0;
        std::string key;
        key = commands[RESPONSEV]; 
        // std::string toStr;
        // Addr2Str (to, toStr);
        while (pos < end) {
            memset (bufferV, 0, BASESIZE * 2);
            bufferSize = CreatResponseV (bufferV, key, vec, pos); 
            NS_ASSERT (bufferSize != 0);
            
            //Ptr<Packet> p = Create<Packet> (bufferV, bufferSize);
            socketPool[toStr]->Send((char *)bufferV, bufferSize, false);
            //SendData (socketPool[toStr], bufferV, bufferSize);
            // NS_LOG_INFO ("[SimpleMPI:SendResponseVTo] CreatResponseV key = " << key << " pos = " 
            //                     << pos << " bufferSize = " << bufferSize << " Send to " << toStr);
            pos += k;
        }
        delete[] bufferV;
        // NS_LOG_INFO ("[SimpleMPI:SendResponseVTo] CreatResponseV key = " << key << " pos = " 
        //                         << pos << " bufferSize = " << bufferSize << " Send to " << toStr);
    }

    void 
    SimpleMPI::SendResponseVToP () {
        //NS_LOG_INFO ("[SimpleMPI:SendResponseVToP] maxIteration " << maxIteration
                            //<< "sGroup.size()" << this->sGroup. size ());
        for (uint8_t i = 0; i < this->sGroup. size (); ++i) {
            std::string toStr;
            Addr2Str (this->sGroup[i], toStr);
            SendResponseVTo (toStr, avg);
        }
    }

    void 
    SimpleMPI::AVG () {
        NS_LOG_FUNCTION (this);
        if (this->asEnd) {
            this->asEnd = false;
            if (this->curIter > 0) {
                //NS_LOG_INFO ("[SimpleMPI:AVG] one AVG end");
                //write ans
                WriteChunk2Vec (avg, vsize);
                //root node, save result
                if (this->sGroup. size () <= 0) {
                    //AvgVector (avg, );
                    Time currentTime = Simulator::Now();
                    Time duration = currentTime - this->startTime;
                    NS_LOG_INFO ("one AVG operation time elapsed : " << duration.GetMilliSeconds()  << "ms");
                }
                //SaveResult ();

                //reply result to parent
                SendResponseVToP ();
                
                //clear chunk
                ClearChunkMap ();
            }

            // simulator end early
            if (this->curIter >= this->maxIteration) {
                Simulator::Stop (Seconds(Simulator::Now().GetSeconds() + 1));
            }

            if (this->curIter < this->maxIteration) {
                NS_LOG_INFO ("[SimpleMPI:AVG] start a new request at time " << Simulator::Now().GetSeconds () << 
                                " count = " << this->curIter << " maxIteration = " << this->maxIteration);
                this->startTime = Simulator::Now();
                SendRequestVToAll (vsize);
                //this->curIter++;
            }
        }
    }

    void
    SimpleMPI::AvgEnd (uint16_t size) {
        NS_LOG_FUNCTION (this);

        bool res = true;
        uint8_t aggSize = cGroup. size ();
        uint8_t range = size / k + (size % k != 0);
        for (uint8_t i = 0; i < range; ++i) {
            if (chunkMap. find (i * k) == chunkMap. end ()) {
                res = false;
                break;
            }
            res &= (chunkMap[i * k]. count == aggSize);
        }

        this->asEnd = res;
        //SendRequestAll ();
        if (res)
            this->curIter++;
        //ns3::Simulator::Schedule(ns3::MilliSeconds(10), &SimpleMPI::AVG, this);
        AVG ();
    }

    bool 
    SimpleMPI::GetAsEnd () {
        NS_LOG_FUNCTION (this);
        return this->asEnd;
    }

    void 
    SimpleMPI::SetAsEnd (bool v) {
        NS_LOG_FUNCTION (this);
        this->asEnd = v;
    }

    void
    SimpleMPI::ClearChunkMap () {
        NS_LOG_FUNCTION (this);
        for (auto &tmp : this->chunkMap) {
            tmp. second. count = 0;
            tmp. second. data = std::vector<uint64_t>(k, 0);
        }
    }

    void 
    SimpleMPI::WriteChunk2Vec (std::vector<uint64_t> &vec, uint16_t size) {
        NS_LOG_FUNCTION (this);

        uint8_t range = size / k + (size % k != 0);
        for (uint8_t i = 0; i < range; ++i) {
            //NS_LOG_INFO ("[SimpleMPI:WriteChunk2Vec] chun " << i * k << "size " << chunkMap[i * k]. data. size ()
                            //<< " chunk first number " << chunkMap[i * k]. data[0]);
            for (uint8_t j = 0; j < chunkMap[i * k]. data. size (); ++j) {
                vec[i * k + j] = chunkMap[i * k]. data[j];
            }
        }

    }

    void 
    SimpleMPI::SaveResult () {
        NS_LOG_FUNCTION (this);
        //save result
        for (uint16_t i = 0; i < uint16_t(avg. size ()); ++i) {
            outFile << avg[i];
            // after saving, clear it
            avg[i] = 0;
            // Add space except for the last element
            if (i != avg. size () - 1) {
                outFile << " ";
            }

        }
        outFile << std::endl; // End the line
    }

    void 
    SimpleMPI::SetOutFile (const std::string fileName) {
        NS_LOG_FUNCTION (this);
        outFile.open(fileName, std::ios::app);
        if (!outFile.is_open()) {
            NS_LOG_INFO ("Failed to open file" << fileName);
        }
    }

}; /*namespace ns3*/