#include "Calibration.h"

void Calibration::Initialise ( bool pAllChan )
{
    // Initialize the TestGroups
    MakeTestGroups ( pAllChan );

    // now read the settings from the map
    auto cSetting = fSettingsMap.find ( "HoleMode" );
    fHoleMode = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 1;
    cSetting = fSettingsMap.find ( "TargetVcth" );
    fTargetVcth = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0x78;
    cSetting = fSettingsMap.find ( "TargetOffset" );
    fTargetOffset = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0x50;
    cSetting = fSettingsMap.find ( "Nevents" );
    fEventsPerPoint = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 10;
    cSetting = fSettingsMap.find ( "TestPulseAmplitude" );
    fTestPulseAmplitude = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0;
    cSetting = fSettingsMap.find ( "VerificationLoop" );
    fCheckLoop = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 1;

    if ( fTestPulseAmplitude == 0 ) fTestPulse = 0;
    else fTestPulse = 1;


    // Canvases
    //fVplusCanvas = new TCanvas ( "VPlus", "VPlus", 515, 0, 500, 500 );
    fOffsetCanvas = new TCanvas ( "Offset", "Offset", 10, 0, 500, 500 );
    fOccupancyCanvas = new TCanvas ( "Occupancy", "Occupancy", 10, 525, 500, 500 );

    // count FEs & CBCs
    uint32_t cCbcCount = 0;
    uint32_t cCbcIdMax = 0;
    uint32_t cFeCount = 0;

    for ( auto cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();

        for ( auto cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();
            cFeCount++;

            //figure out the chip type
            fType = cFe->getChipType();

            for ( auto cCbc : cFe->fCbcVector )
            {
                uint32_t cCbcId = cCbc->getCbcId();
                cCbcCount++;

                if ( cCbcId > cCbcIdMax ) cCbcIdMax = cCbcId;

                // populate the channel vector
                std::vector<Channel> cChanVec;

                for ( uint8_t cChan = 0; cChan < NCHANNELS; cChan++ )
                    cChanVec.push_back ( Channel ( cBoardId, cFeId, cCbcId, cChan ) );

                fCbcChannelMap[cCbc] = cChanVec;

                fVplusMap[cCbc] = 0;

                TString cName = Form ( "h_VplusValues_Fe%dCbc%d", cFeId, cCbcId );
                TObject* cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                TProfile* cHist = new TProfile ( cName, Form ( "Vplus Values for Test Groups FE%d CBC%d ; Test Group; Vplus", cFeId, cCbcId ), 9, -1.5, 7.5 );
                cHist->SetMarkerStyle ( 20 );
                // cHist->SetLineWidth( 2 );
                bookHistogram ( cCbc, "Vplus", cHist );

                cName = Form ( "h_Offsets_Fe%dCbc%d", cFeId, cCbcId );
                cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                TH1F* cOffsetHist = new TH1F ( cName, Form ( "Offsets FE%d CBC%d ; Channel; Offset", cFeId, cCbcId ), 254, -.5, 253.5 );
                uint8_t cOffset = ( fHoleMode ) ? 0x00 : 0xFF;

                for ( int iBin = 0; iBin < NCHANNELS; iBin++ )
                    cOffsetHist->SetBinContent ( iBin, cOffset );

                bookHistogram ( cCbc, "Offsets", cOffsetHist );

                cName = Form ( "h_Occupancy_Fe%dCbc%d", cFeId, cCbcId );
                cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                TH1F* cOccHist = new TH1F ( cName, Form ( "Occupancy FE%d CBC%d ; Channel; Occupancy", cFeId, cCbcId ), 254, -.5, 254.5 );
                bookHistogram ( cCbc, "Occupancy", cOccHist );

                //only needed for CBC3 calibration algorithm
                if (fType == ChipType::CBC3)
                {
                    cName = Form ( "h_UntunedPedestal_Fe%dCBC%d", cFeId, cCbcId );
                    cObj = gROOT->FindObject (cName);

                    if (cObj) delete cObj;

                    TH1F* cPedeHist = new TH1F ( cName, cName, 2048, -0.5, 1023.5 );
                    bookHistogram ( cCbc, "Untuned_Pedestal", cPedeHist );

                }
            }
        }

        fNCbc = cCbcCount;
        fNFe = cFeCount;
    }

    uint32_t cPads = ( cCbcIdMax > cCbcCount ) ? cCbcIdMax : cCbcCount;

    //fVplusCanvas->DivideSquare ( cPads );
    fOffsetCanvas->DivideSquare ( cPads );
    fOccupancyCanvas->DivideSquare ( cPads );


    LOG (INFO) << "Created Object Maps and parsed settings:" ;

    if (fType != ChipType::CBC3)
    {
        LOG (INFO) << "	Hole Mode = " << fHoleMode ;
        LOG (INFO) << "	TargetOffset = " << int ( fTargetOffset ) ;
        LOG (INFO) << "	TargetVcth = " << int ( fTargetVcth ) ;
    }
    else
    {
        LOG (INFO) << "Hole mode does not make sense for CBC3, setting to 0!";
        LOG (INFO) << "TargetOffset does not make sense for CBC3, setting to 0!";
        LOG (INFO) << "TargetVcth does not make sense for CBC3, should be determined algorithmically by scanning - still keeping value in case scan is skipped!";
        fHoleMode = 0;
        fTargetOffset = 0xFF;
    }

    LOG (INFO) << "	Nevents = " << fEventsPerPoint ;
    LOG (INFO) << "	TestPulseAmplitude = " << int ( fTestPulseAmplitude ) ;
}

void Calibration::MakeTestGroups ( bool pAllChan )
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

        int cGId = -1;
        std::vector<uint8_t> tempchannelVec;

        for ( int idx = 0; idx < 254; idx++ )
            tempchannelVec.push_back ( idx );

        fTestGroupChannelMap[cGId] = tempchannelVec;
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


void Calibration::FindVplus()
{
    if (fType == ChipType::CBC2)
    {
        // first, set VCth to the target value for each CBC
        ThresholdVisitor cThresholdVisitor (fCbcInterface, fTargetVcth);
        this->accept (cThresholdVisitor);

        // now all offsets are either off (0x00 in holes mode, 0xFF in electrons mode)
        // next a group needs to be enabled - therefore now the group loop
        LOG (INFO) << BOLDBLUE << "Extracting Vplus ..." << RESET ;

        for ( auto& cTGroup : fTestGroupChannelMap )
        {
            if (cTGroup.first == -1)
            {
                // start with a fresh <Cbc, Vplus> map
                clearVPlusMap();

                // looping over the test groups, enable it
                LOG (INFO) << GREEN << "Enabling Test Group...." << cTGroup.first << RESET ;
                setOffset ( fTargetOffset, cTGroup.first, true ); // takes the group ID
                //updateHists ( "Offsets" );

                bitwiseVplus ( cTGroup.first );

                LOG (INFO) << RED << "Disabling Test Group...." << cTGroup.first << RESET  ;
                uint8_t cOffset = ( fHoleMode ) ? 0x00 : 0xFF;
                setOffset ( cOffset, cTGroup.first, true );

                // done looping all the bits - I should now have the Vplus value that corresponds to 50% occupancy at the desired VCth and Offset for this test group mapped against CBC
                for ( auto& cCbc : fVplusMap )
                {
                    TProfile* cTmpProfile = static_cast<TProfile*> ( getHist ( cCbc.first, "Vplus" ) );
                    cTmpProfile->Fill ( cTGroup.first, cCbc.second ); // fill Vplus value for each test group
                }

                //updateHists ( "Vplus" );
                //
            }
        }

        // done extracting reasonable Vplus values for all test groups, now find the mean
        // since I am lazy and do not want to iterate all boards, FEs etc, i Iterate fVplusMap
        for ( auto& cCbc : fVplusMap ) //this toggles bit i on Vplus for each
        {
            TProfile* cTmpProfile = static_cast<TProfile*> ( getHist ( cCbc.first, "Vplus" ) );
            cCbc.second = cTmpProfile->GetMean ( 2 );

            fCbcInterface->WriteCbcReg ( cCbc.first, "Vplus", cCbc.second );
            LOG (INFO) << BOLDGREEN <<  "Mean Vplus value for FE " << +cCbc.first->getFeId() << " CBC " << +cCbc.first->getCbcId() << " is " << BOLDRED << +cCbc.second << RESET ;
        }

    }
    else LOG (INFO) << "VPlus measurement does not make sense for CBC3, doing nothing!";
}

void Calibration::bitwiseVplus ( int pTGroup )
{
    // now go over the VPlus bits for each CBC, start with the MSB, flip it to one and measure the occupancy
    for ( int iBit = 7; iBit >= 0; iBit-- )
    {
        for ( auto& cCbc : fVplusMap ) //this toggles bit i on Vplus for each
        {
            toggleRegBit ( cCbc.second, iBit );
            fCbcInterface->WriteCbcReg ( cCbc.first, "Vplus", cCbc.second );
            // LOG(INFO) << "IBIT " << +iBit << " DEBUG Setting Vplus for CBC " << +cCbc.first->getCbcId() << " to " << +cCbc.second << " (= 0b" << std::bitset<8>( cCbc.second ) << ")" ;
        }

        // now each CBC has the MSB Vplus Bit written
        // now take data
        measureOccupancy ( fEventsPerPoint, pTGroup );
        //updateHists ( "Occupancy" );

        // done taking data, now find the occupancy per CBC
        for ( auto& cCbc : fVplusMap )
        {
            // if the occupancy is larger than 0.5 I need to flip the bit back to 0, else leave it
            float cOccupancy = findCbcOccupancy ( cCbc.first, pTGroup, fEventsPerPoint );

            //LOG(INFO) << "VPlus " << +cCbc.second << " = 0b" << std::bitset<8>( cCbc.second ) << " on CBC " << +cCbc.first->getCbcId() << " Occupancy : " << cOccupancy ;

            if ( fHoleMode && cOccupancy > 0.56 )
            {
                toggleRegBit ( cCbc.second, iBit ); //here could also use setRegBit to set to 0 explicitly
                fCbcInterface->WriteCbcReg ( cCbc.first, "Vplus", cCbc.second );
            }
            else if ( !fHoleMode && cOccupancy < 0.45 )
            {
                toggleRegBit ( cCbc.second, iBit ); //here could also use setRegBit to set to 0 explicitly
                fCbcInterface->WriteCbcReg ( cCbc.first, "Vplus", cCbc.second );
            }


            // clear the occupancy histogram for the next bit
            clearOccupancyHists ( cCbc.first );
        }

    }

    if ( fCheckLoop )
    {
        measureOccupancy ( fEventsPerPoint, pTGroup );

        for ( auto& cCbc : fVplusMap )
        {
            float cOccupancy = findCbcOccupancy ( cCbc.first, pTGroup, fEventsPerPoint );
            LOG (INFO) << BOLDBLUE << "Found Occupancy of " << BOLDRED << cOccupancy << BOLDBLUE << " for CBC " << +cCbc.first->getCbcId() << " , test Group " << pTGroup << " using VPlus " << BOLDRED << +cCbc.second << BOLDBLUE << " (= 0x" << std::hex << +cCbc.second << std::dec << "; 0b" << std::bitset<8> ( cCbc.second ) << ")" << RESET ;
            clearOccupancyHists ( cCbc.first );
        }
    }
}

float Calibration::FindTargetVcth()
{
    if (fType == ChipType::CBC3)
    {
        TCanvas* cCanvas = new TCanvas ( "Untuned Pedestals", "Untuned Pedestal", 425, 10, 500, 500 );
        cCanvas->DivideSquare (fNCbc);

        LOG (INFO) << BOLDGREEN << "Measuring Untuned SCurve midpoints..." << RESET;
        uint8_t cOffset = 0xFF;
        LOG (INFO) << "Disabling all TestGroups by setting all channel offsets to " << std::hex << "0x" << +cOffset << std::dec;
        //disable all test groups
        auto cGroup = fTestGroupChannelMap.find (-1);

        if (cGroup != std::end (fTestGroupChannelMap) )
            setOffset (cOffset, cGroup->first, false);

        else LOG (ERROR) << "Test Group not found!";

        updateHists ( "Offsets" );

        //loop the test groups, enable a group, measure an SCurve
        for (auto& cTGrp : fTestGroupChannelMap)
        {
            if (cTGrp.first != -1)
            {
                setOffset ( 0x80, cTGrp.first, true ); // takes the group ID
                updateHists ( "Offsets" );
                initializeSCurves ("Offset", 0x80, cTGrp.first);
                measureSCurves (cTGrp.first, 450);
                setOffset (cOffset, cTGrp.first, true);
                updateHists ( "Offsets" );
            }
        }

        //process the SCurves and fill in the pedestal Histogram
        float cMeanPedestal = 0;
        int iCbc = 1;

        for ( auto& cCbc : fCbcChannelMap )
        {

            TH1F* cPedeHist  = dynamic_cast<TH1F*> ( getHist ( cCbc.first, "Untuned_Pedestal" ) );

            //std::vector<uint8_t> cTestGrpChannelVec = fTestGroupChannelMap[pTGrp];

            for (uint8_t cChanId = 0; cChanId < NCHANNELS; cChanId++)
                //for ( auto& cChanId : cTestGrpChannelVec )
            {
                //for ( auto& cChan : cCbc.second )
                Channel cChan = cCbc.second.at ( cChanId );

                // Fit or Differentiate
                //if ( fFitted ) cChan.fitHist ( fEventsPerPoint, fHoleMode, pValue, pParameter, fResultFile );
                cChan.differentiateHist ( fEventsPerPoint, fHoleMode, 0, "Untuned", fResultFile );


                // instead of the code below, use a histogram to histogram the noise
                if ( cChan.getNoise() == 0 || cChan.getNoise() > 1023 ) LOG (INFO) << RED << "Error, SCurve Fit for Fe " << int ( cCbc.first->getFeId() ) << " Cbc " << int ( cCbc.first->getCbcId() ) << " Channel " << int ( cChan.fChannelId ) << " did not work correctly! Noise " << cChan.getNoise() << RESET ;

                cPedeHist->Fill ( cChan.getPedestal() );
            }

            cMeanPedestal += cPedeHist->GetMean();
            LOG (INFO) << BOLDRED << "Average untuned pedestal for Fe " << +cCbc.first->getFeId() << " Cbc " << +cCbc.first->getCbcId() << " : " << cPedeHist->GetMean() << " Vcth units!" << RESET;

            cCanvas->cd (iCbc);
            cPedeHist->Draw ("APL");
            iCbc++;
        }

        fTargetVcth = static_cast<uint16_t> (cMeanPedestal / fNCbc);
        LOG (INFO) << BOLDCYAN << "Setting target Vcth to " << fTargetVcth << RESET;
        return cMeanPedestal / fNCbc;

    }
    else
    {
        LOG (INFO) << "This measurement does not make sense for CBC2, doing nothing!";
        return 0;
    }
}


void Calibration::FindOffsets()
{
    // do a binary search for the correct offset value


    // just to be sure, configure the correct VCth and VPlus values
    ThresholdVisitor cThresholdVisitor (fCbcInterface, fTargetVcth);
    this->accept (cThresholdVisitor);
    // ok, done, all the offsets are at the starting value, VCth & Vplus are written

    // now loop over test groups
    for ( auto& cTGroup : fTestGroupChannelMap )
    {
        if (cTGroup.first != -1)
        {
            LOG (INFO) << GREEN << "Enabling Test Group...." << cTGroup.first << RESET ;

            bitwiseOffset ( cTGroup.first );

            if ( fCheckLoop )
            {
                // now all the bits are toggled or not, I still want to verify that the occupancy is ok
                int cMultiple = 3;
                LOG (INFO) << "Verifying Occupancy with final offsets by taking " << fEventsPerPoint* cMultiple << " Triggers!" ;
                measureOccupancy ( fEventsPerPoint  * cMultiple, cTGroup.first );
                // now find the occupancy for each channel and update the TProfile
            }

            updateHists ( "Occupancy" );
            uint8_t cOffset = ( fHoleMode ) ? 0x00 : 0xFF;
            setOffset ( cOffset, cTGroup.first );
            LOG (INFO) << RED << "Disabling Test Group...." << cTGroup.first << RESET  ;
        }
    }

    setRegValues();
}


void Calibration::bitwiseOffset ( int pTGroup )
{
    // loop over the bits
    for ( int iBit = 7; iBit >= 0; iBit-- )
    {
        LOG (INFO) << "Searching for the correct offsets by flipping bit " << iBit ;
        // now, for all the channels in the group and for each cbc, toggle the MSB of the offset from the map
        toggleOffset ( pTGroup, iBit, true );


        // now the offset for the current group is changed
        // now take data
        measureOccupancy ( fEventsPerPoint, pTGroup );


        // now call toggleOffset again with pBegin = false; this method checks the occupancy and flips a bit back if necessary
        toggleOffset ( pTGroup, iBit, false );
        //updateHists ( "Offsets" );
    }

    updateHists ( "Occupancy" );
    updateHists ( "Offsets" );
}


void Calibration::measureOccupancy ( uint32_t pNEvents, int pTGroup )
{
    for ( BeBoard* pBoard : fBoardVector )
    {

        uint32_t cN = 0;
        uint32_t cNthAcq = 0;

        ReadNEvents (pBoard, pNEvents);
        std::vector<Event*> events = GetEvents ( pBoard );

        // if this is for channelwise offset tuning, iterate the events and fill the occupancy histogram

        for ( auto& cEvent : events )
        {
            for ( auto cFe : pBoard->fModuleVector )
            {
                for ( auto cCbc : cFe->fCbcVector )
                    fillOccupancyHist ( cCbc, pTGroup, cEvent );
            }

            cN++;
        }

        cNthAcq++;
    }
}


float Calibration::findCbcOccupancy ( Cbc* pCbc, int pTGroup, int pEventsPerPoint )
{
    TH1F* cOccHist = static_cast<TH1F*> ( getHist ( pCbc, "Occupancy" ) );
    float cOccupancy = cOccHist->GetEntries();
    // return the hitcount divided by the the number of channels and events
    return cOccupancy / ( static_cast<float> ( fTestGroupChannelMap[pTGroup].size() * pEventsPerPoint ) );
}

void Calibration::fillOccupancyHist ( Cbc* pCbc, int pTGroup, const Event* pEvent )
{
    // Find the Occupancy histogram for the current Cbc
    TH1F* cOccHist = static_cast<TH1F*> ( getHist ( pCbc, "Occupancy" ) );
    // iterate the channels in current group
    int cHits = 0;

    for ( auto& cChanId : fTestGroupChannelMap[pTGroup] )
    {
        // I am filling the occupancy profile for each CBC for the current test group
        if ( pEvent->DataBit ( pCbc->getFeId(), pCbc->getCbcId(), cChanId ) )
        {
            cOccHist->Fill ( cChanId );
            cHits++;
        }
    }
}

void Calibration::clearOccupancyHists ( Cbc* pCbc )
{
    TH1F* cOccHist = static_cast<TH1F*> ( getHist ( pCbc, "Occupancy" ) );
    cOccHist->Reset ( "ICESM" );
}

void Calibration::clearVPlusMap()
{
    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            for ( auto cCbc : cFe->fCbcVector )
                fVplusMap[cCbc] = 0;
        }
    }
}

void Calibration::setOffset ( uint8_t pOffset, int  pGroup, bool pVPlus )
{
    LOG (INFO) << "Setting offsets of Test Group " << pGroup << " to 0x" << std::hex << +pOffset << std::dec ;

    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();

            for ( auto cCbc : cFe->fCbcVector )
            {
                uint32_t cCbcId = cCbc->getCbcId();

                // first, find the offset Histogram for this CBC
                TH1F* cOffsetHist = static_cast<TH1F*> ( getHist ( cCbc, "Offsets" ) );

                RegisterVector cRegVec;   // vector of pairs for the write operation

                // loop the channels of the current group and toggle bit i in the global map
                for ( auto& cChannel : fTestGroupChannelMap[pGroup] )
                {
                    TString cRegName = Form ( "Channel%03d", cChannel + 1 );
                    cRegVec.push_back ( {cRegName.Data(), pOffset} );

                    if ( pVPlus ) cOffsetHist->SetBinContent ( cChannel, pOffset );
                }

                fCbcInterface->WriteCbcMultReg ( cCbc, cRegVec );
            }
        }
    }
}

void Calibration::toggleOffset ( uint8_t pGroup, uint8_t pBit, bool pBegin )
{
    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();

            for ( auto cCbc : cFe->fCbcVector )
            {
                uint32_t cCbcId = cCbc->getCbcId();

                // first, find the offset Histogram for this CBC
                TH1F* cOffsetHist = static_cast<TH1F*> ( getHist ( cCbc, "Offsets" ) );
                // find the TProfile for occupancy measurment of current channel
                TH1F* cOccHist = static_cast<TH1F*> ( getHist ( cCbc, "Occupancy" ) );
                // cOccHist->Scale( 1 / double( fEventsPerPoint ) );
                RegisterVector cRegVec;   // vector of pairs for the write operation

                // loop the channels of the current group and toggle bit i in the global map
                for ( auto& cChannel : fTestGroupChannelMap[pGroup] )
                {
                    TString cRegName = Form ( "Channel%03d", cChannel + 1 );

                    if ( pBegin )
                    {
                        // get the offset
                        uint8_t cOffset = cOffsetHist->GetBinContent ( cChannel );

                        // toggle Bit i
                        toggleRegBit ( cOffset, pBit );

                        // modify the histogram
                        cOffsetHist->SetBinContent ( cChannel, cOffset );

                        // push in a vector for CBC write transaction
                        cRegVec.push_back ( {cRegName.Data(), cOffset} );
                    }
                    else  //here it is interesting since now I will check if the occupancy is smaller or larger 50% and decide wether to toggle or not to toggle
                    {
                        // if the occupancy is larger than 50%, flip the bit back, if it is smaller, don't do anything
                        // get the offset
                        uint8_t cOffset = cOffsetHist->GetBinContent ( cChannel );

                        // get the occupancy
                        int iBin = cOccHist->GetXaxis()->FindBin ( cChannel );
                        float cOccupancy = cOccHist->GetBinContent ( iBin );
                        //LOG (DEBUG) << "Bit " << +pBit << " Offset " << +cOffset << " Occupancy " << +cOccupancy;

                        // only if the occupancy is too high I need to flip the bit back and write, if not, I can leave it
                        if ( cOccupancy > 0.57 * fEventsPerPoint )
                        {
                            toggleRegBit ( cOffset, pBit ); // toggle the bit back that was previously flipped
                            cOffsetHist->SetBinContent ( cChannel, cOffset );
                            cRegVec.push_back ( {cRegName.Data(), cOffset} );
                        }

                        // since I extracted the info from the occupancy profile for this bit (this iteration), i need to clear the corresponding bins
                        cOccHist->SetBinContent ( iBin, 0 );
                    }
                }

                fCbcInterface->WriteCbcMultReg ( cCbc, cRegVec );
            }
        }
    }
}

void Calibration::updateHists ( std::string pHistname )
{
    // loop the CBCs
    for ( const auto& cCbc : fCbcHistMap )
    {
        // loop the map of string vs TObject
        auto cHist = cCbc.second.find ( pHistname );

        if ( cHist != std::end ( cCbc.second ) )
        {
            // now cHist.second is the Histogram
            if ( pHistname == "Vplus" )
            {
                //fVplusCanvas->cd ( cCbc.first->getCbcId() + 1 );
                TProfile* cTmpProfile = static_cast<TProfile*> ( cHist->second );
                cTmpProfile->DrawCopy ( "H P0 E" );
                //fVplusCanvas->Update();
            }

            if ( pHistname == "Offsets" )
            {
                fOffsetCanvas->cd ( cCbc.first->getCbcId() + 1 );
                TH1F* cTmpHist = static_cast<TH1F*> ( cHist->second );
                cTmpHist->DrawCopy();
                fOffsetCanvas->Update();
            }

            if ( pHistname == "Occupancy" )
            {
                fOccupancyCanvas->cd ( cCbc.first->getCbcId() + 1 );
                TProfile* cTmpProfile = static_cast<TProfile*> ( cHist->second );
                cTmpProfile->DrawCopy();
                fOccupancyCanvas->Update();
            }
        }
        else LOG (INFO) << "Error, could not find Histogram with name " << pHistname ;
    }

#ifdef __HTTP__
    //fHttpServer->ProcessRequests();
#endif
}

void Calibration::setRegValues()
{
    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();

            for ( auto cCbc : cFe->fCbcVector )
            {
                uint32_t cCbcId = cCbc->getCbcId();

                // first, find the offset Histogram for this CBC
                TH1F* cOffsetHist = static_cast<TH1F*> ( getHist ( cCbc, "Offsets" ) );

                for ( int iChan = 0; iChan < NCHANNELS; iChan++ )
                {
                    uint8_t cOffset = cOffsetHist->GetBinContent ( iChan );
                    cCbc->setReg ( Form ( "Channel%03d", iChan + 1 ), cOffset );
                    //LOG(INFO) << GREEN << "Offset for CBC " << cCbcId << " Channel " << iChan << " : 0x" << std::hex << +cOffset << std::dec << RESET ;
                }

            }
        }
    }
}

void Calibration::writeGraphs()
{
    fResultFile->cd();
    // Save hist maps for CBCs

    //Tool::SaveResults();

    // save canvases too
    //fVplusCanvas->Write ( fVplusCanvas->GetName(), TObject::kOverwrite );
    fOffsetCanvas->Write ( fOffsetCanvas->GetName(), TObject::kOverwrite );
    fOccupancyCanvas->Write ( fOccupancyCanvas->GetName(), TObject::kOverwrite );
}
