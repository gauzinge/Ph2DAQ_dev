/*!

        \file                   Commissioning.h
        \brief                 class to do latency and threshold scans
        \author              Georg AUZINGER
        \version                1.0
        \date                   20/01/15
        Support :               mail to : georg.auzinger@cern.ch

 */

#ifndef COMMISSIONING_H__
#define COMMISSIONING_H__

#include "Tool.h"
#include "../Utils/Visitor.h"
#include "../Utils/Utilities.h"
#include "../Utils/CommonVisitors.h"


#include "TString.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TH2F.h"
#include "TGaxis.h"

using namespace Ph2_System;
using namespace Ph2_HwInterface;
using namespace Ph2_HwDescription;

typedef std::map<Cbc*, std::map<std::string, TObject*> >  CbcHistogramMap;
typedef std::map<Module*, std::map<std::string, TObject*> > ModuleHistogramMap;
// typedef std::map<Module*, TCanvas*> CanvasMap;

/*!
 * \class Commissioning
 * \brief Class to perform latency and threshold scans
 */

class Commissioning : public Tool
{

  public:
	void Initialize(uint32_t pStartLatency, uint32_t pLatencyRange);
	std::map<Module*, uint8_t> ScanLatency( uint8_t pStartLatency = 0, uint8_t pLatencyRange = 20 );
	std::map<Module*, uint8_t> ScanStubLatency( uint8_t pStartLatency = 0, uint8_t pLatencyRange = 20 );
	// void ScanLatencyThreshold();
	// void SaveResults();
	void SignalScan(int SignalScanLength);

  private:
	int countHitsLat( BeBoard* pBoard,  const std::vector<Event*> pEventVec, std::string pHistName, uint8_t pParameter, uint32_t pStartLatency );
	int countHits( Module* pFe,  const Event* pEvent, std::string pHistName, uint8_t pParameter );
	int countStubs( Module* pFe,  const Event* pEvent, std::string pHistName, uint8_t pParameter );
	void updateHists( std::string pHistName, bool pFinal );
	void parseSettings();

	//  Members
	uint32_t fNevents;
	uint32_t fInitialThreshold;
	uint32_t fHoleMode;
	uint32_t fNoiseToSignalVCTH;
	uint32_t fNCbc;
	uint32_t fSignalScanStep;

   	const uint32_t fTDCBins = 8;

    int convertLatencyPhase(uint32_t pStartLatency, uint32_t cLatency, uint32_t cPhase)
    {
         int result = int(cLatency) - int(pStartLatency);
         result *= fTDCBins;
         result += fTDCBins - 1 - cPhase;
         return result+1;
    }
};

#endif
