
#ifndef HybridTester_h__
#define HybridTester_h__

#include "../HWDescription/Module.h"
#include "../HWDescription/Cbc.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../System/SystemController.h"
#include "../System/ConsoleColor.h"
#include "../HWInterface/Visitor.h"
#include "../HWInterface/Utilities.h"
#include "../HWInterface/CommonVisitors.h"



#include "../System/SystemController.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TH1F.h"
#include "TF1.h"
#include "TStyle.h"
#include "TLine.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

class HybridTester : public SystemController
{
  public:
	HybridTester() {}
	~HybridTester() {}
	void InitializeHists();
	void TestRegisters();
	void ScanThreshold();
	void Measure();
	void SaveResults();

  private:
	uint32_t fNCbc;
	TCanvas* fDataCanvas;
	TH1F* fHistTop;
	TH1F* fHistBottom;

	void UpdateHists() {
		fDataCanvas->cd( 1 );
		fHistTop->Draw();
		fDataCanvas->cd( 2 );
		fHistBottom->Draw();
		fDataCanvas->Update();
	}
};


#endif


