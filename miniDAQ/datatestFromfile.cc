#include <cstring>
#include <stdint.h>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <sstream>
#include <inttypes.h>

#include "../Utils/Utilities.h"
#include "../Utils/Data.h"
#include "../Utils/Event.h"
#include "../Utils/Timer.h"
#include "../Utils/argvparser.h"
#include "../Utils/ConsoleColor.h"
#include "../System/SystemController.h"

#include "../RootWeb/include/publisher.h"

#include "TROOT.h"
#include "TH1.h"
#include "TFile.h"
#include "TString.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

using namespace CommandLineProcessing;

void tokenize( const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters )
{
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of( delimiters, 0 );

	// Find first "non-delimiter".
	std::string::size_type pos = str.find_first_of( delimiters, lastPos );

	while ( std::string::npos != pos || std::string::npos != lastPos )
	{
		// Found a token, add it to the vector.
		tokens.push_back( str.substr( lastPos, pos - lastPos ) );

		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of( delimiters, pos );

		// Find next "non-delimiter"
		pos = str.find_first_of( delimiters, lastPos );
	}
}

void readDataFile( const std::string infile, std::vector<uint32_t>& list )
{
	uint32_t word;
	std::ifstream fin( infile, std::ios::in | std::ios::binary );
	if ( fin.is_open() )
	{
		while ( !fin.eof() )
		{
			char buffer[4];
			fin.read( buffer, 4 );
			uint32_t word;
			memcpy( &word, buffer, 4 );
			list.push_back( word );
		}
		fin.close();
	}
	else
		std::cerr << "Failed to open " << infile << "!" << std::endl;
}

void fillDQMhisto( const std::vector<Event*>& elist, const std::string& outFileName )
{
	std::map<std::string, TH1I*> herrbitmap;
	std::map<std::string, TH1I*> hnstubsmap;
	std::map<std::string, std::vector<TH1I*>> hchdatamap;

	TH1I* hl1A = new TH1I( "l1A", "L1A Counter", 10000, 0, 10000 );
	TH1I* htdc = new TH1I( "tdc", "TDC counter", 256, -0.5, 255.5 );
	for ( const auto& ev : elist )
	{
		htdc->Fill( ev->GetTDC() );
		hl1A->Fill( ev->GetEventCount() );
		const EventMap& evmap = ev->GetEventMap();
		for ( auto const& it : evmap )
		{
			uint32_t feId = it.first;
			for ( auto const& jt : it.second )
			{
				uint32_t cbcId = jt.first;

				// create histogram name tags
				std::stringstream ss;
				ss << "fed" << feId << "cbc" << cbcId;
				std::string key = ss.str();

				// Error bit histogram
				if ( herrbitmap.find( key ) == herrbitmap.end() )
					herrbitmap[key] = new TH1I( TString( "errbit" + key ), "Error bit", 3, -0.5, 2.5 );
				herrbitmap[key]->Fill( static_cast<int>( ev->Error( feId, cbcId ) ) );

				// #Stubs info
				int nstubs = std::stoi( ev->StubBitString( feId, cbcId ), nullptr, 10 );
				if ( hnstubsmap.find( key ) == hnstubsmap.end() )
					hnstubsmap[key] = new TH1I( TString( "nstubs" + key ), "Number of stubs", 21, -0.5, 20.5 );
				hnstubsmap[key]->Fill( nstubs );

				// channel data
				const std::vector<bool>& dataVec = ev->DataBitVector( feId, cbcId );
				if ( hchdatamap.find( key ) == hchdatamap.end() )
				{
					hchdatamap[key].push_back( new TH1I( TString( "chOccupancy_even" + key ), "Even channel occupancy", 127, 0.5, 127.5 ) );
					hchdatamap[key].push_back( new TH1I( TString( "chOccupancy_odd" + key ), "Odd channel occupancy", 127, 0.5, 127.5 ) );
				}
				for ( unsigned int ch = 0; ch < dataVec.size(); ch++ )
				{
					if ( dataVec[ch] )
					{
						if ( ch % 2 == 0 )
							hchdatamap[key][0]->Fill( ch / 2 + 1 );
						else
							hchdatamap[key][1]->Fill( ch / 2 + 1 );
					}
				}
			}
		}
	}
	TFile* fout = TFile::Open( outFileName.c_str(), "RECREATE" );
	for ( auto& he : herrbitmap )
	{
		fout->mkdir( he.first.c_str() );
		fout->cd( he.first.c_str() );
		he.second->Write();
	}
	for ( auto& he : hnstubsmap )
	{
		fout->cd( he.first.c_str() );
		he.second->Write();
	}


	for ( auto& hVec : hchdatamap )
	{
		fout->cd( hVec.first.c_str() );
		hVec.second.at( 0 )->Write();
		hVec.second.at( 1 )->Write();
	}

	fout->cd();
	hl1A->Write();
	htdc->Write();
	fout->Close();
}

void dumpEvents( const std::vector<Event*>& elist )
{
	for ( int i = 0; i < elist.size(); i++ )
	{
		std::cout << "Event index: " << i << std::endl;
		std::cout << *elist[i] << std::endl;
	}
}

int main( int argc, char* argv[] )
{
	ArgvParser cmd;

	// init
	cmd.setIntroductoryDescription( "CMS Ph2_ACF  miniDQM application" );
	// error codes
	cmd.addErrorCode( 0, "Success" );
	cmd.addErrorCode( 1, "Error" );
	// options
	cmd.setHelpOption( "h", "help", "Print this help page" );

	cmd.defineOption( "file", "Binary Data File", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "file", "f" );

	cmd.defineOption( "output", "Output Directory for DQM plots & page. Default value: Results", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "output", "o" );

	cmd.defineOption( "dqm", "Build DQM webpage. Default = false", ArgvParser::NoOptionAttribute /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "dqm", "d" );

	cmd.defineOption( "swap", "Swap endianness in Data::set. Default = true (Ph2_ACF); should be false for GlibStreamer Data", ArgvParser::NoOptionAttribute /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "swap", "s" );

	int result = cmd.parse( argc, argv );
	if ( result != ArgvParser::NoParserError )
	{
		std::cout << cmd.parseErrorDescription( result );
		exit( 1 );
	}

	// now query the parsing results
	std::string rawFilename = ( cmd.foundOption( "file" ) ) ? cmd.optionValue( "file" ) : "";
	if ( rawFilename.empty() )
	{
		std::cerr << "Error, no binary file provided. Quitting" << std::endl;
		exit( 1 );
	}

	bool cSwap = ( cmd.foundOption( "swap" ) ) ? false : true;

	bool cDQMPage = ( cmd.foundOption( "dqm" ) ) ? true : false;

	std::string cDirBasePath;

	if ( cDQMPage )
	{
		if ( cmd.foundOption( "output" ) )
		{
			cDirBasePath = cmd.optionValue( "output" );
			cDirBasePath += "/";

		}
		else cDirBasePath = "Results/";
	}
	else std::cout << "Not creating DQM files!" << std::endl;

	std::vector<uint32_t> dataVec;
	readDataFile( rawFilename, dataVec );

	SystemController cSystemController;
	std::string cHWFile = getenv( "BASE_DIR" );
	cHWFile += "/settings/HWDescription_2CBC.xml";
	cSystemController.parseHWxml( cHWFile );
	BeBoard* pBoard = cSystemController.fShelveVector.at( 0 )->fBoardVector.at( 0 );

	Data d;
	int nEvents = dataVec.size() / 42;
	d.Set( pBoard, dataVec, nEvents, cSwap );
	const std::vector<Event*>& elist = d.GetEvents( pBoard );
	//dumpEvents(elist);

	// Create the DQM plots and generate the root file
	// first of all strip the folder name
	std::vector<std::string> tokens;
	tokenize( rawFilename, tokens, "/" );
	std::string fname = tokens.back();

	// now form the output Root filename
	tokens.clear();
	tokenize( fname, tokens, "." );
	std::string runLabel = tokens[0];
	std::string dqmFilename =  runLabel + "_dqm.root";

	if ( cDQMPage )
	{
		fillDQMhisto( elist, dqmFilename );
		// cDirBasePath += runLabel;
		RootWeb::makeDQMmonitor( dqmFilename, cDirBasePath, runLabel );
		std::cout << "Saving root file to " << dqmFilename << " and webpage to " << cDirBasePath << std::endl;
	}
	else dumpEvents( elist );
	return 0;
}
