#include "HybridTester.h"

// fill the Histograms, count the hits and increment Vcth

struct HistogramFiller  : public HwDescriptionVisitor
{
	TH1F* fBotHist;
	TH1F* fTopHist;
	const Event* fEvent;

	HistogramFiller( TH1F* pBotHist, TH1F* pTopHist, const Event* pEvent ): fBotHist( pBotHist ), fTopHist( pTopHist ), fEvent( pEvent ) {}

	void visit( Cbc& pCbc ) {
		std::vector<bool> cDataBitVector = fEvent->DataBitVector( pCbc.getFeId(), pCbc.getCbcId() );
		for ( uint32_t cId = 0; cId < NSENSOR; cId++ ) {
			if ( cDataBitVector.at( cId ) ) {
				uint32_t globalChannel = ( pCbc.getCbcId() * 254 ) + cId;
				//              std::cout << "Channel " << globalChannel << " VCth " << ( int )pCbc.getReg( "VCth" ) << std::endl;
				// find out why histograms are not filling!
				if ( globalChannel % 2 == 0 )
					fBotHist->Fill( globalChannel / 2 );
				else
					fTopHist->Fill( ( globalChannel - 1 ) / 2 );

			}
		}
	}
};


struct CbcHitCounter  : public HwDescriptionVisitor
{
	const Event* fEvent;
	uint32_t fHitcounter = 0;

	CbcHitCounter( const Event* pEvent ): fEvent( pEvent ) {}

	void visit( Cbc& pCbc ) {
		for ( uint32_t cId = 0; cId < NSENSOR; cId++ ) {
			if ( fEvent->DataBit( pCbc.getFeId(), pCbc.getCbcId(), cId ) ) fHitcounter++;
		}
	}
};


void HybridTester::InitializeHists()
{
	gStyle->SetOptStat( 000000 );
	gStyle->SetTitleOffset( 1.3, "Y" );
	//  special Visito class to count objects
	Counter cCbcCounter;
	accept( cCbcCounter );
	fNCbc = cCbcCounter.getNCbc();

	fDataCanvas = new TCanvas( "Data", "Different hits counters", 1200, 800 );
	fDataCanvas->Divide( 2 );

	TString cFrontName( "fHistTop" );
	fHistTop = ( TH1F* )( gROOT->FindObject( cFrontName ) );
	if ( fHistTop ) delete fHistTop;

	fHistTop = new TH1F( cFrontName, "Front Pad Channels; Pad Number; Occupancy [%]", ( fNCbc / 2 * 254 ) , -0.5, ( fNCbc / 2 * 254 ) + .5 );

	TString cBackName( "fHistBottom" );
	fHistBottom = ( TH1F* )( gROOT->FindObject( cBackName ) );
	if ( fHistBottom ) delete fHistBottom;

	fHistBottom = new TH1F( cBackName, "Back Pad Channels; Pad Number; Occupancy [%]", ( fNCbc / 2 * 254 ) , -0.5, ( fNCbc / 2 * 254 ) + .5 );


}



void HybridTester::ScanThreshold()
{
	if ( fSettingsMap.find( "ScanThreshold" )->second != 0 )
	{
		std::cout << "Scanning noise Occupancy to find threshold for test with external source ... " << std::endl;

		// Necessary variables
		uint32_t cEventsperVcth = 10;
		bool cNonZero = false;
		bool cAllOne = false;
		uint32_t cAllOneCounter = 0;
		bool cHoleMode = fSettingsMap.find( "HoleMode" )->second;
		uint8_t cVcth, cDoubleVcth;
		( cHoleMode ) ? cVcth = 0xFF : cVcth = 0x00;
		int cStep = ( cHoleMode ) ? -10 : 10;

		// Root objects
		TCanvas* cSCurveCanvas = new TCanvas( "cSCurveCanvas", "Noise Occupancy as function of VCth" );
		TH1F* cSCurve = new TH1F( "cSCurve", "Noise Occupancy; VCth; Counts", 255, 0, 255 );
		cSCurve->SetMarkerStyle( 8 );
		cSCurveCanvas->cd();
		TF1* cFit = new TF1( "cFit", MyErf, 0, 255, 2 );

		// Adaptive VCth loop
		while ( 0x00 <= cVcth && cVcth <= 0xFF )
		{
			if ( cAllOne ) break;
			if ( cVcth == cDoubleVcth )
			{
				cVcth +=  cStep;
				continue;
			}

			// Set current Vcth value on all Cbc's
			CbcRegWriter cWriter( fCbcInterface, "VCth", cVcth );
			accept( cWriter );

			uint32_t cN = 0;
			uint32_t cNthAcq = 0;
			uint32_t cHitCounter = 0;

			// maybe restrict to pBoard? instead of looping?
			for ( auto& cShelve : fShelveVector )
			{
				if ( cAllOne ) break;
				for ( BeBoard& pBoard : cShelve->fBoardVector )
				{
					while ( cN <  cEventsperVcth )
					{
						Run( &pBoard, cNthAcq );

						const Event* cEvent = fBeBoardInterface->GetNextEvent( &pBoard );

						// Loop over Events from this Acquisition
						while ( cEvent )
						{
							if ( cN == cEventsperVcth )
								break;

							CbcHitCounter cHitcounter( cEvent );
							pBoard.accept( cHitcounter );
							cHitCounter += cHitcounter.fHitcounter;
							cN++;

							if ( cN < cEventsperVcth )
								cEvent = fBeBoardInterface->GetNextEvent( &pBoard );
							else break;
						}
						cNthAcq++;
					} // done with this acquisition

					cSCurve->SetBinContent( cVcth, cHitCounter );
					cSCurve->Draw( "P" );
					cSCurveCanvas->Update();
					// check if the hitcounter is all ones

					if ( cNonZero == false && cHitCounter != 0 )
					{
						cDoubleVcth = cVcth;
						cNonZero = true;
						cVcth -= 2 * cStep;
						cStep /= 10;
						continue;
					}
					if ( cHitCounter > 0.95 * cEventsperVcth * fNCbc * NSENSOR ) cAllOneCounter++;
					if ( cAllOneCounter >= 10 ) cAllOne = true;
					if ( cAllOne ) break;
					cVcth += cStep;
				}
			}
		} // end of VCth loop

		// Fit & Plot
		cSCurve->Scale( 1 / double_t( cEventsperVcth * fNCbc * NSENSOR ) );
		// cSCurveCanvas->cd();
		cSCurve->Draw( "P" );

		double cFirstNon0( 0 );
		double cFirst1( 0 );

		// Not Hole Mode
		if ( !cHoleMode )
		{
			for ( Int_t cBin = 1; cBin <= cSCurve->GetNbinsX(); cBin++ )
			{
				double cContent = cSCurve->GetBinContent( cBin );
				if ( !cFirstNon0 )
				{
					if ( cContent ) cFirstNon0 = cSCurve->GetBinCenter( cBin );
				}
				else if ( cContent == 1 )
				{
					cFirst1 = cSCurve->GetBinCenter( cBin );
					break;
				}
			}
		}
		// Hole mode
		else
		{
			for ( Int_t cBin = cSCurve->GetNbinsX(); cBin >= 1; cBin-- )
			{
				double cContent = cSCurve->GetBinContent( cBin );
				if ( !cFirstNon0 )
				{
					if ( cContent ) cFirstNon0 = cSCurve->GetBinCenter( cBin );
				}
				else if ( cContent == 1 )
				{
					cFirst1 = cSCurve->GetBinCenter( cBin );
					break;
				}
			}
		}

		// Get rough midpoint & width
		double cMid = ( cFirst1 + cFirstNon0 ) * 0.5;
		double cWidth = ( cFirst1 - cFirstNon0 ) * 0.5;


		cFit->SetParameter( 0, cMid );
		cFit->SetParameter( 1, cWidth );

		cSCurve->Fit( cFit, "RNQ+" );
		cFit->Draw( "same" );

		// Save
		cSCurve->Write( cSCurve->GetName(), TObject::kOverwrite );
		cFit->Write( cFit->GetName(), TObject::kOverwrite );
		cSCurveCanvas->Write( cSCurveCanvas->GetName(), TObject::kOverwrite );
		std::string cPdfName = fDirectoryName + "/NoiseOccupancy.pdf";
		cSCurveCanvas->SaveAs( cPdfName.c_str() );

		// Set new VCth
		double_t pedestal = cFit->GetParameter( 0 );
		double_t noise = cFit->GetParameter( 1 );

		int cSigmas = fSettingsMap.find( "Threshold_NSigmas" )->second;
		uint8_t cThreshold = ceil( pedestal + cSigmas * fabs( noise ) );

		std::cout << "Identified a noise Occupancy of 50% at VCth " << int( pedestal ) << " -- increasing by " << cSigmas <<  " sigmas (" << fabs( noise ) << ") to " << int( cThreshold ) << " for Hybrid test!" << std::endl;

		TLine* cLine = new TLine( cThreshold, 0, cThreshold, 1 );
		cLine->SetLineWidth( 3 );
		cLine->SetLineColor( 2 );
		cLine->Draw( "same" );
		cSCurveCanvas->Update();

		CbcRegWriter cWriter( fCbcInterface, "VCth", cThreshold );
		accept( cWriter );

		// Wait for user to acknowledge and turn on external Source!
		std::cout << "Identified the threshold for 0 noise occupancy - Start external Signal source!" << std::endl;
		mypause();
	}
}

void HybridTester::TestRegisters()
{

	// This method has to be followed by a configure call, otherwise the CBCs will be in an undefined state
	struct RegTester : public HwDescriptionVisitor
	{
		CbcInterface* fInterface;
		std::map<uint32_t, std::set<std::string>> fBadRegisters;
		RegTester( CbcInterface* pInterface ): fInterface( pInterface ) {
			std::set<std::string> tempset;
			fBadRegisters[0] = tempset;
			fBadRegisters[1] = tempset;
		}

		void visit( Cbc& pCbc ) {
			uint8_t cFirstBitPattern = 0xAA;
			uint8_t cSecondBitPattern = 0x55;

			CbcRegMap cMap = pCbc.getRegMap();
			for ( const auto& cReg : cMap ) {
				if ( !fInterface->WriteCbcReg( &pCbc, cReg.first, cFirstBitPattern, true ) ) fBadRegisters[pCbc.getCbcId()] .insert( cReg.first );
				if ( !fInterface->WriteCbcReg( &pCbc, cReg.first, cSecondBitPattern, true ) ) fBadRegisters[pCbc.getCbcId()] .insert( cReg.first );
			}
		}

		void dumpResult() {
			for ( const auto& cCbc : fBadRegisters ) {
				std::cout << "Bad Registers on Cbc " << cCbc.first << " : " << std::endl;
				for ( const auto& cReg : cCbc.second ) std::cout << cReg << std::endl;
			}
		}
	};

	// This should probably be done in the top level application but there I do not have access to the settings map
	if ( fSettingsMap.find( "TestRegs" )->second != 0 )
	{

		std::cout << "Testing Cbc Registers one-by-one with complimentary bit-patterns (0xAA, 0x55) ..." << std::endl;
		RegTester cRegTester( fCbcInterface );
		accept( cRegTester );
		cRegTester.dumpResult();
		std::cout << "Done testing registers, re-configuring to calibrated state!" << std::endl;
		ConfigureHw();
	}
}

void HybridTester::Measure()
{
	std::cout << "Mesuring Efficiency per Strip ... " << std::endl;
	uint32_t cTotalEvents = fSettingsMap.find( "Nevents" )->second;

	CbcRegReader cReader( fCbcInterface, "VCth" );
	accept( cReader );

	for ( auto& cShelve : fShelveVector )
	{
		for ( BeBoard& pBoard : cShelve->fBoardVector )
		{
			uint32_t cN = 0;
			uint32_t cNthAcq = 0;

			while ( cN <  cTotalEvents )
			{
				std::cout << "Event # " << cN << " # of Acquistion " << cNthAcq << std::endl;
				Run( &pBoard, cNthAcq );

				const Event* cEvent = fBeBoardInterface->GetNextEvent( &pBoard );
				// Loop over Events from this Acquisition
				while ( cEvent )
				{
					std::cout << "Inside Event Loop Event # " << cN << " # of Acquistion " << cNthAcq << std::endl;

					if ( cN == cTotalEvents )
						break;

					HistogramFiller cFiller( fHistBottom, fHistTop, cEvent );
					pBoard.accept( cFiller );

					if ( cN % 100 == 0 )
						UpdateHists();

					cN++;

					if ( cN < cTotalEvents )
						cEvent = fBeBoardInterface->GetNextEvent( &pBoard );
					else break;
				}
				cNthAcq++;
			} // End of Analyze Events of last Acquistion loop
		}
	}
	fHistTop->Scale( 100 / double_t( cTotalEvents ) );
	fHistTop->GetYaxis()->SetRangeUser( 0, 100 );
	fHistBottom->Scale( 100 / double_t( cTotalEvents ) );
	fHistBottom->GetYaxis()->SetRangeUser( 0, 100 );
	UpdateHists();
}

void HybridTester::SaveResults()
{
	fHistTop->Write( fHistTop->GetName(), TObject::kOverwrite );
	fHistBottom->Write( fHistBottom->GetName(), TObject::kOverwrite );
	fDataCanvas->Write( fDataCanvas->GetName(), TObject::kOverwrite );

	fResultFile->Write();
	fResultFile->Close();


	std::cout << "Resultfile written correctly!" << std::endl;

	std::string cPdfName = fDirectoryName + "/HybridTestResults.pdf";
	fDataCanvas->SaveAs( cPdfName.c_str() );

}


