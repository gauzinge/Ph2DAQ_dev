#ifndef _CHANNEL_H__
#define _CHANNEL_H__

#include <map>
#include <vector>
#include "TH1F.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "THStack.h"
#include "TFile.h"
#include "TString.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "../System/ConsoleColor.h"
#include "../HWInterface/Utilities.h"



struct Channel
{

	Channel( uint8_t pBeId, uint8_t pFeId, uint8_t pCbcId, uint8_t pChannelId );
	~Channel();

	// members
	// Ids
	uint8_t fBeId;
	uint8_t fFeId;
	uint8_t fCbcId;
	uint8_t fChannelId;

	// Settings
	uint8_t fOffset;

	// Data
	TH1F* fScurve;
	TF1*  fFit;

	// Methods
	double getPedestal();
	double getNoise();
	uint8_t getOffset() {
		return fOffset;
	};
	void setOffset( uint8_t pOffset );

	void initializeHist( uint8_t pValue, TString pParameter );
	void fillHist( uint8_t pVcth );
	void fitHist( uint32_t pEventsperVcth, bool pHole, uint8_t pValue, TString pParameter, TFile* pResultfile );
	void resetHist();
};

struct TestGroup
{
	TestGroup( uint8_t pShelveId, uint8_t pBeId, uint8_t pFeId, uint8_t pCbcId, uint8_t pGroupId );

	uint8_t fShelveId;
	uint8_t fBeId;
	uint8_t fFeId;
	uint8_t fCbcId;
	uint8_t fGroupId;
};

struct TestGroupGraph
{
	TestGroupGraph();
	TestGroupGraph( uint8_t pBeId, uint8_t pFeId, uint8_t pCbcId, uint8_t pGroupId );
	void FillVplusVcthGraph( uint8_t& pVplus, double pPedestal, double pNoise );
	TGraphErrors* fVplusVcthGraph;

};

struct TestGroupComparer
{
	bool operator()( const TestGroup& g1, const TestGroup& g2 ) const {
		if ( g1.fShelveId == g2.fShelveId ) {
			if ( g1.fBeId == g2.fBeId ) {
				if ( g1.fFeId == g2.fFeId ) {
					if ( g1.fCbcId == g2.fCbcId )
						return g1.fGroupId < g2.fGroupId;
					else return g1.fCbcId < g2.fCbcId;
				}
				else return g1.fFeId < g2.fFeId;
			}
			else return g1.fBeId < g2.fBeId;
		}
		else g1.fShelveId < g2.fShelveId;
	}
};

typedef std::map< TestGroup, std::vector<Channel>, TestGroupComparer > TestGroupMap;
typedef std::map< TestGroup, TestGroupGraph, TestGroupComparer > TestGroupGraphMap;

#endif
