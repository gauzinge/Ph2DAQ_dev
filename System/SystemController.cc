/*

        FileName :                    SystemController.cc
        Content :                     Controller of the System, overall wrapper of the framework
        Programmer :                  Nicolas PIERRE
        Version :                     1.0
        Date of creation :            10/08/14
        Support :                     mail to : nicolas.pierre@cern.ch

 */

#include "SystemController.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

namespace Ph2_System
{

	SystemController::SystemController()
	{

	}

	SystemController::~SystemController()
	{
	}

	void SystemController::InitializeHw( const std::string& pFilename )
	{

		pugi::xml_document doc;
		uint32_t cShelveId, cBeId, cModuleId, cCbcId;
		uint32_t cNShelve = 0;
		int i, j;

		pugi::xml_parse_result result = doc.load_file( pFilename.c_str() );

		if ( !result )
		{
			std::cout << "ERROR :\n Unable to open the file : " << pFilename << std::endl;
			std::cout << "Error description : " << result.description() << std::endl;
			return;
		}

		std::cout << "\n\n\n";
		for ( i = 0; i < 80; i++ )
			std::cout << "*";
		std::cout << "\n";
		for ( j = 0; j < 40; j++ )
			std::cout << " ";
		std::cout << BOLDRED << "HW SUMMARY: " << RESET << std::endl;
		for ( i = 0; i < 80; i++ )
			std::cout << "*";
		std::cout << "\n";
		std::cout << "\n";

		for ( pugi::xml_node ns = doc.child( "HwDescription" ).child( "Shelve" ); ns; ns = ns.next_sibling() )
		{
			cShelveId = ns.attribute( "Id0" ).as_int();
			fShelveVector.push_back( new Shelve( cShelveId ) );

			std::cout << BOLDCYAN << ns.name() << "  " << ns.first_attribute().name() << " :" << ns.attribute( "Id" ).value() << RESET << std:: endl;


			for ( pugi::xml_node nb = ns.child( "BeBoard" ); nb; nb = nb.next_sibling() )
			{

				std::cout << BOLDCYAN << "|" << "----" << nb.name() << "  " << nb.first_attribute().name() << " :" << nb.attribute( "Id" ).value() << RESET << std:: endl;

				cBeId = nb.attribute( "Id" ).as_int();
				BeBoard cBeBoard( cShelveId, cBeId );

				for ( pugi::xml_node nr = nb.child( "Register" ); nr != nb.child( "Module" ); nr = nr.next_sibling() )
				{
					// std::cout<<BOLDCYAN<<"|"<<"  "<<"|"<<"_____"<<nr.name()<<"  "<<nr.first_attribute().name()<<" :"<<nr.attribute("name").value() <<RESET<<std:: endl;
					cBeBoard.setReg( std::string( nr.attribute( "name" ).value() ), atoi( nr.first_child().value() ) );
				}

				fShelveVector[cNShelve]->addBoard( cBeBoard );

				BeBoardFWInterface* cBeBoardFWInterface;

				if ( std::string( nb.attribute( "boardType" ).value() ).compare( std::string( "Glib" ) ) )
				{
					cBeBoardFWInterface = new GlibFWInterface( doc.child( "HwDescription" ).child( "Connections" ).attribute( "name" ).value(), cBeId );
					fBeBoardFWMap[cBeId] = cBeBoardFWInterface;
				}
				/*else
				        cBeBoardFWInterface = new OtherFWInterface();*/

				for ( pugi::xml_node nm = nb.child( "Module" ); nm; nm = nm.next_sibling() )
				{
					std::cout << BOLDCYAN << "|" << "	" << "|" << "----" << nm.name() << "  " << nm.first_attribute().name() << " :" << nm.attribute( "ModuleId" ).value() << RESET << std:: endl;

					cModuleId = nm.attribute( "ModuleId" ).as_int();

					Module cModule( cShelveId, cBeId, nm.attribute( "FMCId" ).as_int(), nm.attribute( "FeId" ).as_int(), cModuleId );
					fShelveVector[cNShelve]->getBoard( cBeId )->addModule( cModule );

					pugi::xml_node nprefix = nm.child( "CBC_Files" );
					std::string cFilePrefix = std::string( nprefix.attribute( "path" ).value() );
					if ( !cFilePrefix.empty() ) std::cout << GREEN << "|" << "	" << "|" << "	" << "|" << "----" << "CBC Files Path : " << cFilePrefix << RESET << std::endl;

					for ( pugi::xml_node nc = nm.child( "CBC" ); nc; nc = nc.next_sibling() )
					{
						std::cout << BOLDCYAN << "|" << "	" << "|" << "	" << "|" << "----" << nc.name() << "  " << nc.first_attribute().name() << " :" << nc.attribute( "Id" ).value() << RESET << std:: endl;


						std::string cFileName;
						if ( !cFilePrefix.empty() )
							cFileName = cFilePrefix + nc.attribute( "configfile" ).value();
						else cFileName = nc.attribute( "configfile" ).value();

						Cbc cCbc( cShelveId, cBeId, nm.attribute( "FMCId" ).as_int(), nm.attribute( "FeId" ).as_int(), nc.attribute( "Id" ).as_int(), cFileName );

						for ( pugi::xml_node ngr = nc.child( "Register" ); ngr; ngr = ngr.next_sibling() )
							cCbc.setReg( std::string( ngr.attribute( "name" ).value() ), atoi( ngr.first_child().value() ) );

						for ( pugi::xml_node ng = nm.child( "Global_CBC_Register" ); ng != nm.child( "CBC" ) && ng != nm.child( "CBC_Files" ) && ng != NULL; ng = ng.next_sibling() )
						{

							if ( ng != NULL )
							{
								std::string regname = std::string( ng.attribute( "name" ).value() );
								uint32_t regvalue = convertAnyInt( ng.first_child().value() ) ;
								cCbc.setReg( regname, uint8_t( regvalue ) ) ;

								std::cout << GREEN << "|" << "	" << "|" << "	" << "|" << "----" << ng.name() << "  " << ng.first_attribute().name() << " :" << regname << " =  0x" << std::hex << std::setw( 2 ) << std::setfill( '0' ) << regvalue << std::dec << RESET << std:: endl;
							}
						}
						fShelveVector[cNShelve]->getBoard( cBeId )->getModule( cModuleId )->addCbc( cCbc );
					}
				}

			}

			cNShelve++;
		}

		fBeBoardInterface = new BeBoardInterface( fBeBoardFWMap );
		fCbcInterface = new CbcInterface( fBeBoardFWMap );
		std::cout << "\n";
		std::cout << "\n";
		for ( i = 0; i < 80; i++ )
			std::cout << "*";
		std::cout << "\n";
		for ( j = 0; j < 40; j++ )
			std::cout << " ";
		std::cout << BOLDRED << "END OF HW SUMMARY: " << RESET << std::endl;
		for ( i = 0; i < 80; i++ )
			std::cout << "*";
		std::cout << "\n";
		std::cout << "\n";
	}

	void SystemController::InitializeSettings( const std::string& pFilename )
	{
		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file( pFilename.c_str() );

		if ( !result )
		{
			std::cout << "ERROR :\n Unable to open the file : " << pFilename << std::endl;
			std::cout << "Error description : " << result.description() << std::endl;
			return;
		}
		for ( pugi::xml_node nSettings = doc.child( "Settings" ); nSettings; nSettings = nSettings.next_sibling() )
		{
			for ( pugi::xml_node nSetting = nSettings.child( "Setting" ); nSetting; nSetting = nSetting.next_sibling() )
			{
				fSettingsMap[nSetting.attribute( "name" ).value()] = convertAnyInt( nSetting.first_child().value() );
				std:: cout << RED << "Setting" << RESET << " --" << BOLDCYAN << nSetting.attribute( "name" ).value() << RESET << ":" << BOLDYELLOW << strtoul( std::string( nSetting.first_child().value() ).c_str(), 0, 10 ) << RESET << std:: endl;
			}
		}
	}

	void SystemController::ConfigureHw()
	{
		class Configurator: public HwDescriptionVisitor
		{
		  private:
			Ph2_HwInterface::BeBoardInterface* fBeBoardInterface;
			Ph2_HwInterface::CbcInterface* fCbcInterface;
		  public:
			Configurator( Ph2_HwInterface::BeBoardInterface* pBeBoardInterface, Ph2_HwInterface::CbcInterface* pCbcInterface ): fBeBoardInterface( pBeBoardInterface ), fCbcInterface( pCbcInterface ) {}
			void visit( BeBoard& pBoard ) {
				fBeBoardInterface->ConfigureBoard( &pBoard );
				std::cout << GREEN << "Successfully configured Board " << int( pBoard.getBeId() ) << RESET << std::endl;
			}
			void visit( Cbc& pCbc ) {
				fCbcInterface->ConfigureCbc( &pCbc );
				std::cout << GREEN <<  "Successfully configured Cbc " << int( pCbc.getCbcId() ) << RESET << std::endl;

			}
		};

		Configurator cConfigurator( fBeBoardInterface, fCbcInterface );
		accept( cConfigurator );
	}

	void SystemController::CreateResultDirectory( std::string pDirname )
	{

		bool cHoleMode = fSettingsMap.find( "HoleMode" )->second;

		std::string cMode;
		if ( cHoleMode ) cMode = "_Hole";
		else cMode = "_Electron";

		pDirname = pDirname + cMode +  currentDateTime();
		std::cout << std::endl << "Creating directory: " << pDirname << std::endl << std::endl;
		std::string cCommand = "mkdir -p " + pDirname;

		system( cCommand.c_str() );

		fDirectoryName = pDirname;
	}

	void SystemController::InitResultFile( std::string pFilename )
	{

		if ( !fDirectoryName.empty() )
		{
			std::string cFilename = fDirectoryName + "/" + pFilename + ".root";
			fResultFile = TFile::Open( cFilename.c_str(), "RECREATE" );
		}
		else std::cout << RED << "ERROR: " << RESET << "No Result Directory initialized - not saving results!" << std::endl;
	}

	void SystemController::Run( BeBoard* pBeBoard, uint32_t pNthAcq )
	{
		fBeBoardInterface->Start( pBeBoard );
		fBeBoardInterface->ReadData( pBeBoard, pNthAcq, true );
		fBeBoardInterface->Stop( pBeBoard, pNthAcq );
	}

}
