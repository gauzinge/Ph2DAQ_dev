#include "SCurve.h"
#include <cmath>


void SCurve::MakeTestGroups ( bool pAllChan )
{
    if ( !pAllChan )
    {
        for ( int cGId = 0; cGId < 8; cGId++ )
        {
            std::vector<uint8_t> tempchannelVec;

            for ( int idx = 0; idx < 16; idx++ )
            {
                int ctemp1 = idx * 16 + cGId * 2;
                int ctemp2 = ctemp1 + 1;

                if ( ctemp1 < 254 ) tempchannelVec.push_back ( ctemp1 );

                if ( ctemp2 < 254 )  tempchannelVec.push_back ( ctemp2 );

            }

            fTestGroupChannelMap[cGId] = tempchannelVec;

        }
    }
    else
    {
        int cGId = -1;
        std::vector<uint8_t> tempchannelVec;

        for ( int idx = 0; idx < 254; idx++ )
            tempchannelVec.push_back ( idx );

        fTestGroupChannelMap[cGId] = tempchannelVec;

    }
}

void SCurve::setOffset ( uint8_t pOffset, int  pGroup )
{
    // LOG(INFO) << "Setting offsets of Test Group " << pGroup << " to 0x" << std::hex << +pOffset << std::dec ;
    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            //uint32_t cFeId = cFe->getFeId();

            //for ( auto cCbc : cFe->fCbcVector )
            //{
            //uint32_t cCbcId = cCbc->getCbcId();

            RegisterVector cRegVec;   // vector of pairs for the write operation

            // loop the channels of the current group and toggle bit i in the global map
            for ( auto& cChannel : fTestGroupChannelMap[pGroup] )
            {
                TString cRegName = Form ( "Channel%03d", cChannel + 1 );
                cRegVec.push_back ( {cRegName.Data(), pOffset} );
            }

            fCbcInterface->WriteBroadcastMultReg ( cFe, cRegVec );
            //}
        }
    }
}


void SCurve::measureSCurves ( int  pTGrpId, uint16_t pStartValue )
{
    // Adaptive Loop to measure SCurves

    LOG (INFO) << BOLDGREEN << "Measuring SCurves sweeping VCth ... " << RESET;

    // Necessary variables
    bool cNonZero = false;
    bool cAllOne = false;
    uint32_t cAllOneCounter = 0;
    uint16_t cValue, cDoubleValue;
    int cStep;
    uint8_t cIterationCount = 0;

    //figure out what kind of chips we are dealing with here
    //not the nicest thing to do but assume it will work
    for (BeBoard* cBoard : fBoardVector)
    {
        for (Module* cModule : cBoard->fModuleVector)
            fType = cModule->getChipType();
    }

    uint16_t cMaxValue = (fType == ChipType::CBC2) ? 0xFF : 0x03FF;

    // the expression below mimics XOR
    if ( fHoleMode )
    {
        cValue = cMaxValue;
        cStep = -10;
    }
    else
    {
        cValue = pStartValue;
        cStep = 10;
    }

    //visitor to set threshold on CBC2 and CBC3
    ThresholdVisitor cVisitor (fCbcInterface, 0);

    // Adaptive VCth loop
    while ( 0x00 <= cValue && cValue <= cMaxValue )
    {
        if (cIterationCount > 0 && (cValue == cMaxValue || cValue == 0x00) )
        {
            LOG (INFO) << BOLDRED << "ERROR: something wrong with these SCurves"  << RESET ;
            break;
        }

        // DEBUG
        if ( cAllOne ) break;

        if ( cValue == cDoubleValue )
        {
            cValue +=  cStep;
            continue;
        }

        uint32_t cN = 1;
        uint32_t cNthAcq = 0;
        uint32_t cHitCounter = 0;

        // DEBUG
        if ( cAllOne ) break;


        for ( BeBoard* pBoard : fBoardVector )
        {
            for (Module* cFe : pBoard->fModuleVector)
            {
                cVisitor.setThreshold (cValue);
                cFe->accept (cVisitor);
            }

            ReadNEvents ( pBoard, fEventsPerPoint );

            const std::vector<Event*>& events = GetEvents ( pBoard );

            // Loop over Events from this Acquisition
            for ( auto& ev : events )
            {
                cHitCounter += fillSCurves ( pBoard, ev, cValue, pTGrpId ); //pass test group here
                cN++;
            }


            cNthAcq++;
            Counter cCounter;
            pBoard->accept ( cCounter );

            //LOG (INFO) << "DEBUG Vcth " << int ( cValue ) << " Hits " << cHitCounter << " and should be " <<  0.95 * fEventsPerPoint*   cCounter.getNCbc() * fTestGroupChannelMap[pTGrpId].size() ;

            // check if the hitcounter is all ones
            if ( cNonZero == false && cHitCounter != 0 )
            {
                cDoubleValue = cValue;
                cNonZero = true;

                if ( cValue == cMaxValue ) cValue = cMaxValue;
                else if ( cValue == 0 ) cValue = 0;
                else cValue -= cStep;

                cStep /= 10;
                LOG (INFO) << GREEN << "Found > 0 Hits!, Falling back to " << +cValue  <<  RESET ;
                continue;
            }

            // the above counter counted the CBC objects connected to pBoard
            if ( cHitCounter > 0.95 * fEventsPerPoint  * fNCbc * fTestGroupChannelMap[pTGrpId].size() ) cAllOneCounter++;

            if ( cAllOneCounter >= 10 )
            {
                cAllOne = true;
                LOG (INFO) << RED << "Found maximum occupancy 10 times, SCurves finished! " << RESET ;
            }

            if ( cAllOne ) break;

            cValue += cStep;
        }

        cIterationCount++;
    }
} // end of VCth loop


void SCurve::measureSCurvesOffset ( int  pTGrpId )
{
    // Adaptive Loop to measure SCurves

    LOG (INFO) << BOLDGREEN << "Measuring SCurves sweeping Channel Offsets ... " << RESET ;

    // Necessary variables
    bool cNonZero = false;
    bool cAllOne = false;
    uint32_t cAllOneCounter = 0;
    uint8_t cValue, cStartValue, cDoubleValue;
    int cStep;

    // the expression below mimics XOR
    if ( !fHoleMode )
    {
        cStartValue = cValue = 0xFF;
        cStep = -10;
    }
    else
    {
        cStartValue = cValue = 0x00;
        cStep = 10;
    }

    // Adaptive VCth loop
    while ( 0x00 <= cValue && cValue <= 0xFF )
    {
        // DEBUG
        if ( cAllOne ) break;

        if ( cValue == cDoubleValue )
        {
            cValue +=  cStep;
            continue;
        }

        setOffset ( cValue, pTGrpId ); //need to pass on the testgroup

        uint32_t cN = 1;
        uint32_t cNthAcq = 0;
        uint32_t cHitCounter = 0;

        // DEBUG
        if ( cAllOne ) break;

        for ( BeBoard* pBoard : fBoardVector )
        {
            //Counter cCounter;
            //pBoard->accept ( cCounter );

            ReadNEvents ( pBoard, fEventsPerPoint );
            const std::vector<Event*>& events = GetEvents ( pBoard );

            // Loop over Events from this Acquisition
            for ( auto& ev : events )
            {
                cHitCounter += fillSCurves ( pBoard, ev, cValue, pTGrpId ); //pass test group here
                cN++;
            }

            cNthAcq++;

            // LOG(INFO) << "DEBUG Vcth " << int( cValue ) << " Hits " << cHitCounter << " and should be " <<  0.95 * fEventsPerPoint*   cCounter.getNCbc() * fTestGroupChannelMap[pTGrpId].size() ;

            // check if the hitcounter is all ones
            if ( cNonZero == false && cHitCounter != 0 )
            {
                cDoubleValue = cValue;
                cNonZero = true;

                if ( cValue == 255 ) cValue = 255;
                else if ( cValue == 0 ) cValue = 0;
                else cValue -= 1.5 * cStep;

                cStep /= 10;
                LOG (INFO) << GREEN << "Found > 0 Hits!, Falling back to " << +cValue  <<  RESET ;
                continue;
            }

            // the above counter counted the CBC objects connected to pBoard
            if ( cHitCounter > 0.95 * fEventsPerPoint  * fNCbc * fTestGroupChannelMap[pTGrpId].size() ) cAllOneCounter++;

            if ( cAllOneCounter >= 10 )
            {
                cAllOne = true;
                LOG (INFO) << RED << "Found maximum occupancy 10 times, SCurves finished! " << RESET ;
            }

            if ( cAllOne ) break;

            cValue += cStep;
        }
    } // end of VCth loop

    setOffset ( cStartValue, pTGrpId );
}

void SCurve::initializeSCurves ( TString pParameter, uint16_t pValue, int  pTGrpId )
{
    // Just call the initializeHist method of every channel and tell it what we are varying
    for ( auto& cCbc : fCbcChannelMap )
    {
        std::vector<uint8_t> cTestGrpChannelVec = fTestGroupChannelMap[pTGrpId];

        for ( auto& cChanId : cTestGrpChannelVec )
            ( cCbc.second.at ( cChanId ) ).initializeHist ( pValue, pParameter );
    }

    LOG (INFO) << "SCurve Histograms for " << pParameter << " =  " << int ( pValue ) << " initialized!" ;
}

uint32_t SCurve::fillSCurves ( BeBoard* pBoard,  const Event* pEvent, uint16_t pValue, int  pTGrpId, bool pDraw )
{
    // loop over all FEs on board, check if channels are hit and if so , fill pValue in the histogram of Channel
    uint32_t cHitCounter = 0;

    for ( auto cFe : pBoard->fModuleVector )
    {

        for ( auto cCbc : cFe->fCbcVector )
        {

            CbcChannelMap::iterator cChanVec = fCbcChannelMap.find ( cCbc );

            if ( cChanVec != fCbcChannelMap.end() )
            {
                const std::vector<uint8_t>& cTestGrpChannelVec = fTestGroupChannelMap[pTGrpId];

                for ( auto& cChanId : cTestGrpChannelVec )
                {
                    if ( pEvent->DataBit ( cFe->getFeId(), cCbc->getCbcId(), cChanVec->second.at ( cChanId ).fChannelId ) )
                    {
                        cChanVec->second.at ( cChanId ).fillHist ( pValue );
                        cHitCounter++;
                    }

                }
            }
            else LOG (INFO) << RED << "Error: could not find the channels for CBC " << int ( cCbc->getCbcId() ) << RESET ;
        }
    }

    return cHitCounter;
}


void SCurve::setSystemTestPulse ( uint8_t pTPAmplitude, uint8_t pTestGroup )
{
    std::vector<std::pair<std::string, uint8_t>> cRegVec;
    uint8_t cRegValue =  to_reg ( 0, pTestGroup );

    cRegVec.push_back ( std::make_pair ( "SelTestPulseDel&ChanGroup",  cRegValue ) );

    //set the value of test pulsepot registrer and MiscTestPulseCtrl&AnalogMux register
    if ( fHoleMode )
        cRegVec.push_back ( std::make_pair ( "MiscTestPulseCtrl&AnalogMux", 0xD1 ) );
    else
        cRegVec.push_back ( std::make_pair ( "MiscTestPulseCtrl&AnalogMux", 0x61 ) );

    cRegVec.push_back ( std::make_pair ( "TestPulsePot", pTPAmplitude ) );
    // cRegVec.push_back( std::make_pair( "Vplus",  fVplus ) );
    CbcMultiRegWriter cWriter ( fCbcInterface, cRegVec );
    this->accept ( cWriter );
    // CbcRegReader cReader( fCbcInterface, "MiscTestPulseCtrl&AnalogMux" );
    // this->accept( cReader );
    // cReader.setRegister( "TestPulsePot" );
    // this->accept( cReader );
}

void SCurve::setFWTestPulse()
{
    for (auto& cBoard : fBoardVector)
    {
        std::vector<std::pair<std::string, uint32_t> > cRegVec;
        BoardType cBoardType = cBoard->getBoardType();

        if (cBoardType == BoardType::GLIB || cBoardType == BoardType::CTA)
        {
            cRegVec.push_back ({"COMMISSIONNING_MODE_RQ", 1 });
            cRegVec.push_back ({"COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID", 1 });
        }
        else if (cBoardType == BoardType::ICGLIB || cBoardType == BoardType::ICFC7)
        {
            cRegVec.push_back ({"cbc_daq_ctrl.commissioning_cycle.mode_flags.enable", 1 });
            cRegVec.push_back ({"cbc_daq_ctrl.commissioning_cycle.mode_flags.test_pulse_enable", 1 });
            cRegVec.push_back ({"cbc_daq_ctrl.commissioning_cycle_ctrl", 0x1 });
        }

        fBeBoardInterface->WriteBoardMultReg (cBoard, cRegVec);
    }
}
