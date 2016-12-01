/*

        FileName :                     Event.cc
        Content :                      Event handling from DAQ
        Programmer :                   Nicolas PIERRE
        Version :                      1.0
        Date of creation :             10/07/14
        Support :                      mail to : nicolas.pierre@icloud.com

 */

#include "../Utils/Cbc3Event.h"

using namespace Ph2_HwDescription;


namespace Ph2_HwInterface {

    // Event implementation
    Cbc3Event::Cbc3Event ( const BeBoard* pBoard,  uint32_t pNbCbc, const std::vector<uint32_t>& list )
    {
        SetEvent ( pBoard, pNbCbc, list );
    }


    //Cbc3Event::Cbc3Event ( const Event& pEvent ) :
    //fBunch ( pEvent.fBunch ),
    //fOrbit ( pEvent.fOrbit ),
    //fLumi ( pEvent.fLumi ),
    //fEventCount ( pEvent.fEventCount ),
    //fEventCountCBC ( pEvent.fEventCountCBC ),
    //fTDC ( pEvent.fTDC ),
    //fEventSize (pEvent.fEventSize),
    //fEventDataMap ( pEvent.fEventDataMap )
    //{

    //}


    int Cbc3Event::SetEvent ( const BeBoard* pBoard, uint32_t pNbCbc, const std::vector<uint32_t>& list )
    {
        fEventSize = pNbCbc *  CBC_EVENT_SIZE_32  + EVENT_HEADER_TDC_SIZE_32;

        //now decode the header info
        fBunch = 0;
        fOrbit = 0;
        fLumi = 0;
        fEventCount = 0x1FFFFFFF &  list.at (2);
        //TODO: get the value from 1 CBC here?
        fEventCountCBC = 0;
        fTDC = 0xE0000000 & list.at (2);

        fBeId = 0x0FE00000 & list.at (0);
        fBeFWType = 0x001C0000 & list.at (0);
        fCBCDataType = 0x00030000 & list.at (0);
        fNCbc = 0x0000F000 & list.at (0);
        fEventDataSize = 0x00000FFF & list.at (0);
        fBeStatus = list.at (1);


        //now decode FEEvents
        uint8_t cBeId = pBoard->getBeId();

        if (cBeId != fBeId) LOG (INFO) << "Warning, BeId from Event header and from Memory do not match! - check your configuration!";

        uint32_t cNFe = static_cast<uint32_t> ( pBoard->getNFe() );

        for ( uint8_t cFeId = 0; cFeId < cNFe; cFeId++ )
        {
            // the number of Cbcs is taken from the Event header and compared to the parameter passed above!
            uint32_t cNCbc = fNCbc;

            if (fNCbc != pNbCbc)
            {
                LOG (ERROR) << "Error: the number of CBCs from the Event header do not match what is computed in SW! aborting!";
                exit (1);
            }

            //now loop the CBCs and encode the IDs in key
            for ( uint8_t cCbcId = 0; cCbcId < cNCbc; cCbcId++ )
            {
                //Sanity Check with CBC header
                //first, get the BeID, FeId, CbcId and CBCDataSize from the data frame CBC header and check against the Software
                uint8_t cBeId_Data = list.at (EVENT_HEADER_SIZE_32_CBC3 + cFeId * CBC_EVENT_SIZE_32_CBC3 * fNCbc + cCbcId * CBC_EVENT_SIZE_32_CBC3) &  0x07F00000;
                uint8_t cFeId_Data = list.at (EVENT_HEADER_SIZE_32_CBC3 + cFeId * CBC_EVENT_SIZE_32_CBC3 * fNCbc + cCbcId * CBC_EVENT_SIZE_32_CBC3) &  0x000E0000;
                uint8_t cCbcId_Data = list.at (EVENT_HEADER_SIZE_32_CBC3 + cFeId * CBC_EVENT_SIZE_32_CBC3 * fNCbc + cCbcId * CBC_EVENT_SIZE_32_CBC3) & 0x0001F000;
                uint16_t cCbcDataSize = list.at (EVENT_HEADER_SIZE_32_CBC3 + cFeId * CBC_EVENT_SIZE_32_CBC3 * fNCbc + cCbcId * CBC_EVENT_SIZE_32_CBC3) &  0x00000FFF;

                if (fBeId != cBeId_Data) LOG (INFO) << "Warning, EeId in Data frame does not match what is expected! - check your configuration!";

                if (cFeId != cFeId_Data) LOG (INFO) << "Warning, FeId in Data frame does not match what is expected! - check your configuration!";

                if (cCbcId != cCbcId_Data) LOG (INFO) << "Warning, CbcId in Data frame does not match what is expected! - check your configuration!";

                if (cCbcDataSize != CBC_EVENT_SIZE_32_CBC3 * 32) LOG (INFO) << "Warning, CbcDataSize in Data frame does not match what is expected! - check your configuration!";

                uint16_t cKey = encodeId (cFeId, cCbcId);

                //I still store the CBC header in the fEventDataMap
                uint32_t begin = EVENT_HEADER_SIZE_32_CBC3 + cFeId * CBC_EVENT_SIZE_32_CBC3 * fNCbc + cCbcId * CBC_EVENT_SIZE_32_CBC3;
                uint32_t end = begin + CBC_EVENT_SIZE_32_CBC3;

                std::vector<uint32_t> cCbcData (std::next (std::begin (list), begin), std::next (std::begin (list), end) );

                fEventDataMap[cKey] = cCbcData;
            }
        }
    }


    std::string Cbc3Event::HexString() const
    {
        //std::stringbuf tmp;
        //std::ostream os ( &tmp );

        //os << std::hex;

        //for ( uint32_t i = 0; i < fEventSize; i++ )
        //os << std::uppercase << std::setw ( 2 ) << std::setfill ( '0' ) << ( fEventData.at (i) & 0xFF000000 ) << " " << ( fEventData.at (i) & 0x00FF0000 ) << " " << ( fEventData.at (i) & 0x0000FF00 ) << " " << ( fEventData.at (i) & 0x000000FF );

        ////os << std::endl;

        //return tmp.str();
    }

    std::string Cbc3Event::DataHexString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        std::stringbuf tmp;
        std::ostream os ( &tmp );
        std::ios oldState (nullptr);
        oldState.copyfmt (os);
        os << std::hex << std::setfill ('0');

        //get the CBC event for pFeId and pCbcId into vector<32bit> cbcData
        std::vector< uint32_t > cbcData;
        GetCbcEvent (pFeId, pCbcId, cbcData);
        //for the first 32-bit word, use only the 26 LSBs
        //this unfortunately means that i have two leading bits that are always 0
        os << std::setw (6) << (cbcData.at (10) & 0x03FFFFFF);

        //this is the body of 7 words that are full of data
        for ( uint32_t i = 9; i > 2; i++ )
            os << std::setw (8) << cbcData.at (i);

        //the last word with only 4 bits
        os << std::setw (2) << ( (cbcData.at (2) & 0xF0000000) >> 28);
        os.copyfmt (oldState);

        return tmp.str();
    }

    bool Cbc3Event::Error ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const
    {
        i = 1 - i;
        return Bit ( pFeId, pCbcId, i + OFFSET_ERROR_CBC3 );
    }

    uint32_t Cbc3Event::Error ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            uint32_t cError = ( (cData->second.at (2) >> 8) & 0x00000003);
            return this->reverse_bits (cError);
        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return 0;
        }
    }

    uint32_t Cbc3Event::PipelineAddress ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            uint32_t cPipeAddress = ( (cData->second.at (2) >> 10) & 0x000001FF);
            return this->reverse_bits (cPipeAddress);
        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return 0;
        }
    }

    bool Cbc3Event::DataBit ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const
    {
        if ( i > NCHANNELS )
            return 0;

        //words 0 and 1 only contain header
        //this algorithm works
        uint32_t cWordP = 10 - ( (i + 6) / 32) ;
        uint32_t cBitP = (i + 6) % 32;

        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            if (cWordP >= cData->second.size() ) return false;

            return ( (cData->second.at (cWordP) >> (31 - cBitP) ) & 0x1);
        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return false;
        }

        //return Bit ( pFeId, pCbcId, i + OFFSET_CBCDATA );
    }

    std::string Cbc3Event::DataBitString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            std::ostringstream os;

            for ( uint32_t i = 0; i < NCHANNELS; ++i )
            {
                uint32_t cWordP = 10 - ( (i + 6) / 32);
                uint32_t cBitP = (i + 6) % 32;

                if ( cWordP >= cData->second.size() ) break;

                os << ( ( cData->second.at (cWordP) >> ( 31 - cBitP ) ) & 0x1 );
            }

            return os.str();

        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return "";
        }

        //return BitString ( pFeId, pCbcId, OFFSET_CBCDATA, WIDTH_CBCDATA );
    }


    std::vector<bool> Cbc3Event::DataBitVector ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        std::vector<bool> blist;
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            std::ostringstream os;

            for ( uint32_t i = 0; i < NCHANNELS; ++i )
            {
                uint32_t cWordP = 10 - ( (i + 6) / 32);
                uint32_t cBitP = (i + 6) % 32;

                if ( cWordP >= cData->second.size() ) break;

                blist.push_back ( ( cData->second.at (cWordP) >> ( 31 - cBitP ) ) & 0x1 );
            }
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return blist;
        //return BitVector ( pFeId, pCbcId, OFFSET_CBCDATA, WIDTH_CBCDATA );
    }

    std::vector<bool> Cbc3Event::DataBitVector ( uint8_t pFeId, uint8_t pCbcId, const std::vector<uint8_t>& channelList ) const
    {
        std::vector<bool> blist;

        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            for ( auto i :  channelList )
            {
                uint32_t cWordP = 10 - ( (i + 6) / 32);
                uint32_t cBitP = (i + 6) % 32;

                if ( cWordP >= cData->second.size() ) break;

                blist.push_back ( ( cData->second.at (cWordP) >> ( 31 - cBitP ) ) & 0x1 );
            }
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return blist;
    }


    //#if 0


    std::string Cbc3Event::GlibFlagString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        return "";
    }


    std::string Cbc3Event::StubBitString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        //TODO: this is super weird - don't do it like that!
        //EDIT: just never use this method
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            std::ostringstream os;

            for ( uint32_t i = 0; i < 36; ++i )
            {
                uint32_t cWordP = i / 32;
                uint32_t cBitP = i % 32;

                if ( cWordP >= cData->second.size() ) break;

                //os << ((cbcData[cByteP] & ( 1 << ( 7 - cBitP ) ))?"1":"0");
                uint32_t cWord = cData->second.at (cWordP);
                this->reverse_bits (cWord);
                os <<  (  (cWord >> ( 31 - cBitP ) ) & 0x1 );
            }

            return os.str();

        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return "";
        }

        //return BitString ( pFeId, pCbcId, OFFSET_CBCSTUBDATA, WIDTH_CBCSTUBDATA );
    }

    bool Cbc3Event::StubBit ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        //here just OR the stub positions
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            uint8_t pos1 = cData->second.at (1) & 0x000000FF;
            uint8_t pos2 = cData->second.at (1) & 0x0000FF00;
            uint8_t pos3 = cData->second.at (1) & 0x00FF0000;
            return (pos1 || pos2 || pos3);
        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return false;
        }
    }

    std::vector<Stub> Cbc3Event::StubVector (uint8_t pFeId, uint8_t pCbcId) const
    {
        std::vector<Stub> cStubVec;
        //here create stubs and return the vector
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            uint8_t pos1 = cData->second.at (1) & 0x000000FF;
            uint8_t pos2 = cData->second.at (1) & 0x0000FF00;
            uint8_t pos3 = cData->second.at (1) & 0x00FF0000;

            uint8_t bend1 = cData->second.at (1) & 0x0F000000;
            uint8_t bend2 = cData->second.at (1) & 0xF0000000;
            uint8_t bend3 = cData->second.at (2) & 0x0000000F;

            cStubVec.emplace_back (pos1, bend1) ;
            cStubVec.emplace_back (pos2, bend2) ;
            cStubVec.emplace_back (pos3, bend3) ;
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return cStubVec;
    }


    uint32_t Cbc3Event::GetNHits (uint8_t pFeId, uint8_t pCbcId) const
    {
        uint32_t cNHits = 0;
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);


        if (cData != std::end (fEventDataMap) )
        {
            cNHits += __builtin_popcount ( cData->second.at (2) & 0xF0000000);

            for (auto cWord = (cData->second.begin() + 3); cWord != cData->second.end() - 1;  cWord++)
                cNHits += __builtin_popcount (*cWord);

            cNHits += __builtin_popcount (cData->second.back() & 0x03FFFFFF);
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return cNHits;
    }


    std::vector<uint32_t> Cbc3Event::GetHits (uint8_t pFeId, uint8_t pCbcId) const
    {
        std::vector<uint32_t> cHits;
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            uint32_t cIndex = 0;

            for ( uint32_t i = 0; i < NCHANNELS; ++i )
            {
                uint32_t cWordP = 10 - ( (i + 6) / 32);
                uint32_t cBitP = (i + 6) % 32;

                if ( cWordP >= cData->second.size() ) break;

                if ( ( cData->second.at (cWordP) >> ( 31 - cBitP ) ) & 0x1) cHits.push_back (cIndex);

                cIndex++;
            }
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return cHits;
    }

    void Cbc3Event::printCbcHeader (std::ostream& os, uint8_t pFeId, uint8_t pCbcId) const
    {
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            uint8_t cBeId = cData->second.at (0) & 0x07F00000;
            uint8_t cFeId = cData->second.at (0) & 0x000E0000;
            uint8_t cCbcId = cData->second.at (0) & 0x0001F000;
            uint16_t cCbcDataSize = cData->second.at (0) & 0x00000FFF;
            os << GREEN << "CBC Header:" << std::endl;
            os << "BeId: " << +cBeId << " FeId: " << +cFeId << " CbcId: " << +cCbcId << " DataSize: " << cCbcDataSize << RESET << std::endl;
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

    }

    void Cbc3Event::print ( std::ostream& os) const
    {
        os << BOLDGREEN << "EventType: CBC3" << RESET << std::endl;
        os << BOLDBLUE <<  "L1A Counter: " << this->GetEventCount() << RESET << std::endl;
        os << "          Be Id: " << this->GetBeId() << std::endl;
        os << "          Be FW: " << this->GetFWType() << std::endl;
        os << "      Be Status: " << this->GetBeStatus() << std::endl;
        os << "  Cbc Data type: " << this->GetCbcDataType() << std::endl;
        os << "          N Cbc: " << this->GetNCbc() << std::endl;
        os << "Event Data size: " << this->GetEventDataSize() << std::endl;
        //os << "  CBC Counter: " << this->GetEventCountCBC() << RESET << std::endl;
        //os << "Bunch Counter: " << this->GetBunch() << std::endl;
        //os << "Orbit Counter: " << this->GetOrbit() << std::endl;
        //os << " Lumi Section: " << this->GetLumi() << std::endl;
        os << BOLDRED << "    TDC Counter: " << this->GetTDC() << RESET << std::endl;

        const int FIRST_LINE_WIDTH = 22;
        const int LINE_WIDTH = 32;
        const int LAST_LINE_WIDTH = 8;

        os << GREEN << "CBC Header:" << std::endl;

        for (auto const& cKey : this->fEventDataMap)
        {
            uint8_t cFeId;
            uint8_t cCbcId;
            this->decodeId (cKey.first, cFeId, cCbcId);

            //here display the Cbc Header manually
            this->printCbcHeader (os, cFeId, cCbcId);

            // here print a list of stubs
            uint8_t cCounter = 1;

            os << BOLDCYAN << "List of Stubs: " << RESET << std::endl;

            for (auto& cStub : this->StubVector (cFeId, cCbcId) )
            {
                os << CYAN << "Stub: " << +cCounter << " Position: " << cStub.getPosition() << " Bend: " << cStub.getBend() << RESET << std::endl;
                cCounter++;
            }

            // here list other bits in the stub stream

            std::string data ( this->DataBitString ( cFeId, cCbcId ) );
            os << GREEN << "FEId = " << +cFeId << " CBCId = " << +cCbcId << RESET << " len(data) = " << data.size() << std::endl;
            os << YELLOW << "PipelineAddress: " << this->PipelineAddress (cFeId, cCbcId) << RESET << std::endl;
            os << RED << "Error: " << static_cast<std::bitset<2>> ( this->Error ( cFeId, cCbcId ) ) << RESET << std::endl;
            os << CYAN << "Total number of hits: " << this->GetNHits ( cFeId, cCbcId ) << RESET << std::endl;
            os << BLUE << "List of hits: " << RESET << std::endl;
            std::vector<uint32_t> cHits = this->GetHits (cFeId, cCbcId);

            if (cHits.size() == 254) os << "All channels firing!" << std::endl;
            else
            {
                int cCounter = 0;

                for (auto& cHit : cHits )
                {
                    os << std::setw (3) << cHit << " ";
                    cCounter++;

                    if (cCounter == 10)
                    {
                        os << std::endl;
                        cCounter = 0;
                    }

                }

                os << RESET << std::endl;
            }

            os << "Ch. Data:      ";

            for (int i = 0; i < FIRST_LINE_WIDTH; i += 2)
                os << data.substr ( i, 2 ) << " ";

            os << std::endl;

            for ( int i = 0; i < 7; ++i )
            {
                for (int j = 0; j < LINE_WIDTH; j += 2)
                    //os << data.substr ( FIRST_LINE_WIDTH + LINE_WIDTH * i, LINE_WIDTH ) << std::endl;
                    os << data.substr ( FIRST_LINE_WIDTH + LINE_WIDTH * i + j, 2 ) << " ";

                os << std::endl;
            }

            for (int i = 0; i < LAST_LINE_WIDTH; i += 2)
                os << data.substr ( FIRST_LINE_WIDTH + LINE_WIDTH * 7 + i , 2 ) << " ";

            os << std::endl;

            os << BLUE << "Stubs: " << this->StubBitString ( cFeId, cCbcId ).c_str() << RESET << std::endl;
        }

        os << std::endl;
    }

    std::vector<Cluster> Cbc3Event::getClusters ( uint8_t pFeId, uint8_t pCbcId) const
    {
        std::vector<Cluster> result;

        // Use the bool vector method (SLOW!) TODO: improve this
        std::vector<bool> stripBits = DataBitVector ( pFeId, pCbcId );

        // Cluster finding
        Cluster aCluster;

        for (int iSensor = 0; iSensor < 2; ++iSensor)
        {
            aCluster.fSensor = iSensor;
            bool inCluster = false;

            for (int iStrip = iSensor; iStrip < stripBits.size(); iStrip += 2)
            {
                if (stripBits.at (iStrip) )
                {
                    // The strip is on
                    if (!inCluster)
                    {
                        // New cluster
                        aCluster.fFirstStrip = iStrip / 2;
                        aCluster.fClusterWidth = 1;
                        inCluster = true;
                    }
                    else
                    {
                        // Increase cluster
                        aCluster.fClusterWidth++;
                    }
                }
                else
                {
                    // The strip is off
                    if (inCluster)
                    {
                        inCluster = false;
                        result.push_back (aCluster);
                    }
                }
            }

            // Fix clusters at the end of the sensor
            if (inCluster) result.push_back (aCluster);
        }

        return result;
    }
}