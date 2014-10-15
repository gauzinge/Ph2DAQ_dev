#include <cstring>
#include "../HWDescription/Cbc.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
#include "../tools/HybridTester.h"
#include <TApplication.h>

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

void syntax( int argc )
{
	if ( argc > 2 ) std::cerr << RED << "ERROR: Syntax: hybridtest HWDescriptionFile" << std::endl;
	else return;
}

int main( int argc, char* argv[] )
{
	syntax( argc );

	std::string cHWFile = argv[1];
	// if ( argc > 1 && !strcmp( argv[1], "8CBC" ) ) cHWFile = "settings/HWDescription_8CBC.xml";
	// else cHWFile = "settings/HWDescription_2CBC.xml";

	TApplication cApp( "Root Application", &argc, argv );
	TQObject::Connect( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );

	HybridTester cHybridTester;
	cHybridTester.InitializeHw( cHWFile );
	cHybridTester.InitializeHists();
	cHybridTester.InitializeSettings( cHWFile );
	cHybridTester.CreateResultDirectory( "Results/HybridTest" );
	cHybridTester.InitResultFile( "HybridTest" );
	cHybridTester.ConfigureHw();

	// Here comes our Part:
	cHybridTester.TestRegisters();
	cHybridTester.ScanThreshold();
	cHybridTester.Measure();
	cHybridTester.SaveResults();


	cApp.Run();

	return 0;

}

