#include "../HWDescription/Cbc.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
#include "../HWInterface/Utilities.h"
#include "../System/SystemController.h"
#include <boost/format.hpp>
#include <TH1F.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TApplication.h>
#include <TROOT.h>

#include <sys/time.h>
#include <ctime>
#include "../HWInterface/Visitor.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;


uint64_t get_time()
{
	/* Linux */
	struct timeval tv;

	gettimeofday( &tv, NULL );

	uint64_t ret = tv.tv_usec;
	/* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
	ret /= 1000;

	/* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
	ret += ( tv.tv_sec * 1000 );

	return ret;
}

int main( int argc, char* argv[] )
{

	SystemController cSystemController;

	cSystemController.InitializeHw( XML_DESCRIPTION_FILE_2CBC );
	cSystemController.ConfigureHw();

	uint64_t t0 = get_time();

	cSystemController.fCbcInterface->ConfigureCbc( cSystemController.fShelveVector[0]->getBoard( 0 )->getModule( 0 )->getCbc( 0 ), true );
	cSystemController.fCbcInterface->ConfigureCbc( cSystemController.fShelveVector[0]->getBoard( 0 )->getModule( 0 )->getCbc( 1 ), true );

	uint64_t t1 = get_time();

	std::cout << "Time for configuring all CBC: " << t1 - t0 << " milliseconds!" << std::endl;

	struct CbcWriter : public HwDescriptionVisitor
	{
		CbcInterface* fInterface;
		CbcWriter( CbcInterface* pInterface ): fInterface( pInterface ) {
		}

		void visit( Cbc& pCbc ) {
			fInterface->ConfigureCbc( &pCbc , true );
			std::cout << "I visited CBC " << int( pCbc.getCbcId() ) << " at " << &pCbc << " using interface " << fInterface <<  std::endl;
		}
	};

	t0 = get_time();

	// cSystemController.fCbcInterface->WriteCbcReg( cSystemController.fShelveVector[0]->getBoard( 0 )->getModule( 0 )->getCbc( 0 ), "VCth", 0x78, true );
	// cSystemController.fCbcInterface->WriteCbcReg(cSystemController.fShelveVector[0]->getBoard(0)->getModule(0)->getCbc(1),"VCth",0x78, false);
	std::cout << cSystemController.fCbcInterface << " INTERFACE " <<  std::endl;
	std::cout << cSystemController.fShelveVector[0]->getBoard( 0 )->getModule( 0 )->getCbc( 0 ) << " CBC 0" << std::endl;
	std::cout << cSystemController.fShelveVector[0]->getBoard( 0 )->getModule( 0 )->getCbc( 1 ) << " CBC 1" << std::endl;

	CbcWriter pWriter( cSystemController.fCbcInterface );
	cSystemController.accept( pWriter );

	t1 = get_time();

	std::cout << "Time for configuring all CBCs using visitor: " << t1 - t0 << " milliseconds!" << std::endl;

	std::vector< std::pair< std::string, uint8_t > > cRegVec;

	uint8_t nChannels = 255;
	for ( uint8_t cChannel = 1; cChannel < nChannels; cChannel++ )
	{

		TString cRegName = Form( "Channel%03d", cChannel );
		uint8_t cRegValue = 0x50;

		std::pair<std::string, uint8_t> cRegPair = std::make_pair( cRegName.Data(), cRegValue );
		cRegVec.push_back( cRegPair );
	}

	t0 = get_time();


	cSystemController.fCbcInterface->WriteCbcMultReg( cSystemController.fShelveVector[0]->getBoard( 0 )->getModule( 0 )->getCbc( 0 ), cRegVec, true );
	// cSystemController.fCbcInterface->WriteCbcMultReg(cSystemController.fShelveVector[0]->getBoard(0)->getModule(0)->getCbc(1),cRegVec, false);

	t1 = get_time();

	std::cout << "Time for writing " << int( nChannels ) << " registers to 1 CBC: " << t1 - t0 << " milliseconds!" << std::endl;


	return 0;
}
