/*

   FileName :                     mcp.cpp
   Content :                      MCP Test Interface program, test the software
   Programmer :                   Nicolas PIERRE, Lorenzo Bidegain
   Version :                      0.3
   Date of creation :             24/06/14
   Support :                      mail to : nico.pierre@icloud.com, lorenzo.bidegain@cern.ch

 */

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

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

int main( int argc, char* argv[] )
{
	int i;

	BeBoard* cBeBoard;
	Module* cModule;
	Cbc* cCbc;
	uint32_t cBoardId, cModuleId, cCbcId, cShelveId, cBeId, cFMCId, cFeId;
	std::string cFilePath;
	std::string cRegNode;
	std::string cValue;
	uint32_t cValueHex;
	uint32_t cNthAcq, cVcthMin, cVcthMax, cVcthStep, cNevents;
	bool cBreakTrigger;
	int cMissedBoard = 0;
	int cMissedModule = 0;
	int cMissedCbc = 0;

	TApplication cApp( "Root Application", &argc, argv );

	BeBoardFWInterface* cBeBoardFWInterface;

	SystemController cSystemController;

	std::cout << "\n\n\n\n" << std::endl;
	std::cout << "****************************************************************" << std::endl;
	std::cout << "****************************************************************" << std::endl;
	std::cout << "***          MCP Test Program for Ph2 Base Software          ***" << std::endl;
	std::cout << "***                N. Pierre, L. Bidegain v0.3               ***" << std::endl;
	std::cout << "****************************************************************" << std::endl;
	std::cout << "****************************************************************" << std::endl;

	do
	{

		std::cout << "\n\n\n\n" << std::endl;
		std::cout << "****************************************************" << std::endl;
		std::cout << "***                  Main Menu                   ***" << std::endl;
		std::cout << "****************************************************\n" << std::endl;
		std::cout << "\n\n\n\nWhat do you want to do ?\n" << std::endl;
		std::cout << "1: Load your xml description of the Hardware" << std::endl;
		std::cout << "2: Configure" << std::endl;
		std::cout << "3: Glib Manipulation" << std::endl;
		std::cout << "4: Cbc Manipulation" << std::endl;
		std::cout << "5: Acquisition-ish" << std::endl;
		std::cout << "6: Configuration Recap" << std::endl;
		std::cout << "0: Quit\n" << std::endl;

		std::cin >> i;

		switch ( i )
		{

		case 1:
			std::cout << "\n\n\n\n" << std::endl;
			std::cout << "****************************************************" << std::endl;
			std::cout << "***         Load your xml Hw Description         ***" << std::endl;
			std::cout << "****************************************************\n" << std::endl;

			std::cout << "1: 2 CBC XML file" << std::endl;
			std::cout << "2: 8 CBC XML file\n" << std::endl;

			std::cin >> i;

			switch ( i )
			{
			case 1:
				cSystemController.InitializeHw( XML_DESCRIPTION_FILE_2CBC );
				cSystemController.ConfigureHw();

				std::cout << "*** 2CBC Xml file loaded ***" << std::endl;
				break;


			case 2:
				cSystemController.InitializeHw( XML_DESCRIPTION_FILE_8CBC );
				cSystemController.ConfigureHw();

				std::cout << "*** 8CBC Xml file loaded ***" << std::endl;
				break;
			}
			break;

		/*

		    Legacy stuff for adding/removing Cbcs on the fly.
		    If someone find it useful someday, here it is.

		 */
		/*case ?:

		    std::cout << "\n\n\n\n"<< std::endl;
		    std::cout << "****************************************************" << std::endl;
		    std::cout << "***            Add a Module or a Cbc             ***" << std::endl;
		    std::cout << "****************************************************\n" << std::endl;

		    std::cout << "1: Add Board" << std::endl;
		    std::cout << "2: Add Module" << std::endl;
		    std::cout << "3: Add Cbc" << std::endl;
		    std::cout << "4: Remove Board" << std::endl;
		    std::cout << "5: Remove Module" << std::endl;
		    std::cout << "6: Remove Cbc\n" << std::endl;

		    std::cin >> i;

		    switch(i)
		    {

		        case 1:
		            std::cout << "*** Add Board ***" << std::endl;
		            std::cout << "--> Which ShelveId ?" << std::endl;
		            std::cin >> cShelveId;
		            std::cout << "--> Which BoardId ?" << std::endl;
		            std::cin >> cBoardId;

		            for(uint32_t cSId=0; cSId<cSystemController.fShelveVector.size(); cSId++)
		            {
		                if(cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId)
		                {
		                    if(cSystemController.fBeBoardFWMap.find(cBoardId) == cSystemController.fBeBoardFWMap.end())
		                    {
		                        cBeBoardFWInterface = new BeBoardFWInterface(UHAL_CONNECTION_FILE,cBoardId);
		                        if(cBeBoardFWInterface->getBoardType() == "GLIB")
		                        {
		                            delete cBeBoardFWInterface;
		                            cSystemController.fBeBoardFWMap[cBoardId] = new GlibFWInterface(UHAL_CONNECTION_FILE,cBoardId);
		                            cBeBoard = new BeBoard(cShelveId,cBoardId);
		                            cSystemController.fShelveVector[cSId]->addBoard(*cBeBoard);
		                            std::cout << "*** Board Added ***" << std::endl;
		                        }
		                        else
		                        {
		                            delete cBeBoardFWInterface;
		                            std::cout << "ERROR : Unknown type of board !" << std::endl;
		                        }
		                    }
		                    else
		                    {
		                        std::cout << "ERROR : This board already exists !" << std::endl;
		                        myflush( std::cin );
		                        mypause();
		                    }
		                }
		                else if(cSId == cSystemController.fShelveVector.size())
		                {
		                    std::cout << "ERROR : This Shelve does not exist !" << std::endl;
		                    myflush( std::cin );
		                    mypause();
		                }
		            }
		        break;


		        case 2:
		            std::cout << "*** Add Module ***" << std::endl;
		            std::cout << "--> Which ShelveId ?" << std::endl;
		            std::cin >> cShelveId;
		            std::cout << "--> Which BoardId ?" << std::endl;
		            std::cin >> cBoardId;

		            for(uint32_t cSId=0; cSId<cSystemController.fShelveVector.size(); cSId++)
		            {
		                if(cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId)
		                {
		                    if(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getBeId() == NULL)
		                    {
		                        std::cout << "ERROR : This board does not exist !" << std::endl;
		                        myflush( std::cin );
		                        mypause();
		                    }
		                    else
		                    {
		                        std::cout << "--> Which ModuleId ?" << std::endl;
		                        std::cin >> cModuleId;

		                        if(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId) == NULL)
		                        {
		                            std::cout << "--> Which FMCId ?" << std::endl;
		                            std::cin >> cFMCId;
		                            std::cout << "--> Which FeId ?" << std::endl;
		                            std::cin >> cFeId;

		                            cModule = new Module(cSystemController.fShelveVector[cSId]->getShelveId(),cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getBeId(),cFMCId,cFeId,cModuleId);

		                            cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->addModule(*cModule);
		                            std::cout << "*** Module Added ***" << std::endl;
		                        }
		                        else
		                        {
		                            std::cout << "ERROR : This module already exists !" << std::endl;
		                            myflush( std::cin );
		                            mypause();
		                        }
		                    }
		                }
		                else if(cSId == cSystemController.fShelveVector.size())
		                {
		                    std::cout << "ERROR : This Shelve does not exist !" << std::endl;
		                    myflush( std::cin );
		                    mypause();
		                }
		            }
		        break;


		        case 3:
		            std::cout << "*** Add Cbc ***" << std::endl;
		            std::cout << "1: Add Default Cbc" << std::endl;
		            std::cout << "2: Add Personalised Cbc" << std::endl;
		            std::cout << "3: Add a 2Cbc Hybrid Structure" << std::endl;
		            std::cout << "4: Add a 8Cbc Hybrid Structure\n" << std::endl;

		            std::cin >> i;

		            switch(i)
		            {

		                case 1:
		                    std::cout << "*** Add Default Cbc ***" << std::endl;
		                    std::cout << "--> Which ShelveId ?" << std::endl;
		                    std::cin >> cShelveId;
		                    std::cout << "--> Which BoardId ?" << std::endl;
		                    std::cin >> cBoardId;

		                    for(uint32_t cSId=0; cSId<cSystemController.fShelveVector.size(); cSId++)
		                    {

		                        if(cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId)
		                        {
		                            if(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getBeId() == NULL)
		                            {
		                                std::cout << "ERROR : This board does not exist !" << std::endl;
		                                myflush( std::cin );
		                                mypause();
		                            }
		                            else
		                            {
		                                std::cout << "--> Which ModuleId ?" << std::endl;
		                                std::cin >> cModuleId;
		                                if (cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId) == NULL)
		                                {
		                                    std::cout << "*** ERROR !!                                      ***" << std::endl;
		                                    std::cout << "*** This module does not exist !                  ***" << std::endl;
		                                    std::cout << "*** This is not the module you are looking for... ***" << std::endl;
		                                    myflush( std::cin );
		                                    mypause();
		                                }
		                                else
		                                {
		                                    std::cout << "--> Which CbcId ?" << std::endl;
		                                    std::cin >> cCbcId;
		                                    if(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getCbc(cCbcId) == NULL)
		                                    {
		                                        cCbc = new Cbc(0,0,0,0,cCbcId,DEFAULT_FILE);
		                                        cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->addCbc(*cCbc);
		                                        delete cCbc;
		                                        std::cout << "*** Cbc Added ***" << std::endl;
		                                    }
		                                    else
		                                    {
		                                        std::cout << "ERROR : This cbc already exists !" << std::endl;
		                                        myflush( std::cin );
		                                        mypause();
		                                    }
		                                }
		                            }
		                        }
		                        else if(cSId == cSystemController.fShelveVector.size())
		                        {
		                            std::cout << "ERROR : This Shelve does not exist !" << std::endl;
		                            myflush( std::cin );
		                            mypause();
		                        }
		                    }
		                break;


		                case 2:
		                    std::cout << "*** Add Personalised Cbc ***" << std::endl;
		                    std::cout << "--> Which ShelveId ?" << std::endl;
		                    std::cin >> cShelveId;
		                    std::cout << "--> Which BoardId ?" << std::endl;
		                    std::cin >> cBoardId;
		                    for(uint32_t cSId=0; cSId<cSystemController.fShelveVector.size(); cSId++)
		                    {

		                        if(cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId)
		                        {
		                            if(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getBeId() == NULL)
		                            {
		                                std::cout << "ERROR : This board does not exist !" << std::endl;
		                                myflush( std::cin );
		                                mypause();
		                            }
		                            else
		                            {
		                                std::cout << "--> Which ModuleId ?" << std::endl;
		                                std::cin >> cModuleId;
		                                if (cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId) == NULL)
		                                {
		                                    std::cout << "*** ERROR !!                                      ***" << std::endl;
		                                    std::cout << "*** This module does not exist !                  ***" << std::endl;
		                                    std::cout << "*** This is not the module you are looking for... ***" << std::endl;
		                                    myflush( std::cin );
		                                    mypause();
		                                }
		                                else
		                                {
		                                    std::cout << "--> Which CbcId ?" << std::endl;
		                                    std::cin >> cCbcId;

		                                    if(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getCbc(cCbcId) == NULL)
		                                    {
		                                        std::cout << "ERROR : This board does not exist !" << std::endl;
		                                        myflush( std::cin );
		                                        mypause();
		                                        std::cout << "--> Choose your Config File" << std::endl;
		                                        std::cout << "1: FE0CBC0HOLE" << std::endl;
		                                        std::cout << "2: FE0CBC1" << std::endl;
		                                        std::cout << "3: FE0CBC1HOLE" << std::endl;
		                                        std::cout << "4: Your own Config File" << std::endl;
		                                        std::cin >> i;

		                                        switch(i)
		                                        {

		                                            case 1:
		                                                {
		                                                    Cbc cCbcPers(cShelveId,cBeId,cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getFMCId(),cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getFeId(),cCbcId,FE0CBC0HOLE);
		                                                    cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->addCbc(cCbcPers);
		                                                    cSystemController.fCbcInterface->ConfigureCbc(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getCbc(cCbcId));
		                                                    std::cout << "*** Cbc Added ***" << std::endl;
		                                                }
		                                            break;


		                                            case 2:
		                                                {
		                                                    Cbc cCbcPers(cShelveId,cBeId,cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getFMCId(),cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getFeId(),cCbcId,FE0CBC1);
		                                                    cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->addCbc(cCbcPers);
		                                                    cSystemController.fCbcInterface->ConfigureCbc(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getCbc(cCbcId));
		                                                    std::cout << "*** Cbc Added ***" << std::endl;
		                                                }
		                                            break;


		                                            case 3:
		                                                {
		                                                    Cbc cCbcPers(cShelveId,cBeId,cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getFMCId(),cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getFeId(),cCbcId,FE0CBC1HOLE);
		                                                    cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->addCbc(cCbcPers);
		                                                    cSystemController.fCbcInterface->ConfigureCbc(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getCbc(cCbcId));
		                                                    std::cout << "*** Cbc Added ***" << std::endl;
		                                                }
		                                            break;


		                                            case 4:
		                                                {
		                                                    std::cout << "--> Enter your File Path " << std::endl;
		                                                    std::cout << "(Absolute path please) " << std::endl;
		                                                    std::cin >> cFilePath;
		                                                    Cbc cCbcPers(cShelveId,cBeId,cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getFMCId(),cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getFeId(),cCbcId,cFilePath);
		                                                    cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->addCbc(cCbcPers);
		                                                    cSystemController.fCbcInterface->ConfigureCbc(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getCbc(cCbcId));
		                                                    std::cout << "*** Cbc Added ***" << std::endl;
		                                                }
		                                            break;


		                                            default:
		                                                std::cout << "*** This is not the option you are looking for... ***" << std::endl;
		                                                myflush( std::cin );
		                                                mypause();
		                                            break;

		                                        }
		                                    }
		                                    else
		                                    {
		                                        std::cout << "ERROR : This cbc already exists !" << std::endl;
		                                    }
		                                }
		                            }
		                        }
		                        else if(cSId == cSystemController.fShelveVector.size())
		                        {
		                            std::cout << "ERROR : This Shelve does not exist !" << std::endl;
		                            myflush( std::cin );
		                            mypause();
		                        }
		                    }
		                break;


		                case 3:
		                    std::cout << "*** Add a 2Cbc Hybrid Structure ***" << std::endl;
		                    std::cout << "--> Which ShelveId ?" << std::endl;
		                    std::cin >> cShelveId;
		                    std::cout << "--> Which BoardId ?" << std::endl;
		                    std::cin >> cBoardId;

		                    for(uint32_t cSId=0; cSId<cSystemController.fShelveVector.size(); cSId++)
		                    {

		                        if(cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId)
		                        {
		                            if(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getBeId() == NULL)
		                            {
		                                std::cout << "ERROR : This board does not exist !" << std::endl;
		                                myflush( std::cin );
		                                mypause();
		                            }
		                            else
		                            {

		                                std::cout << "--> Which ModuleId ?" << std::endl;
		                                std::cin >> cModuleId;
		                                if (cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId) == NULL)
		                                {
		                                    std::cout << "*** ERROR !!                                      ***" << std::endl;
		                                    std::cout << "*** This module does not exist !                  ***" << std::endl;
		                                    std::cout << "*** This is not the module you are looking for... ***" << std::endl;
		                                    myflush( std::cin );
		                                    mypause();
		                                }
		                                else
		                                {
		                                    for(uint8_t i=0; i<2; i++)
		                                    {
		                                        if(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getCbc(i) == NULL)
		                                        {
		                                            cCbc = new Cbc(0,0,0,0,i,(boost::format("settings/FE0CBC%d.txt") %(uint32_t(i))).str());
		                                            cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->addCbc(*cCbc);
		                                            cSystemController.fCbcInterface->ConfigureCbc(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getCbc(i));
		                                            delete cCbc;
		                                        }
		                                    }

		                                    std::cout << "*** 2Cbc Hybrid Structure Added ***" << std::endl;
		                                }
		                            }
		                        }
		                        else if(cSId == cSystemController.fShelveVector.size())
		                        {
		                            std::cout << "ERROR : This Shelve does not exist !" << std::endl;
		                            myflush( std::cin );
		                            mypause();
		                        }
		                    }
		                break;



		                case 4:
		                    std::cout << "*** Add a 8Cbc Hybrid Structure ***" << std::endl;
		                    std::cout << "--> Which ShelveId ?" << std::endl;
		                    std::cin >> cShelveId;
		                    std::cout << "--> Which BoardId ?" << std::endl;
		                    std::cin >> cBoardId;

		                    for(uint32_t cSId=0; cSId<cSystemController.fShelveVector.size(); cSId++)
		                    {

		                        if(cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId)
		                        {
		                            if(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getBeId() == NULL)
		                            {
		                                std::cout << "ERROR : This board does not exist !" << std::endl;
		                                myflush( std::cin );
		                                mypause();
		                            }
		                            else
		                            {

		                                std::cout << "--> Which ModuleId ?" << std::endl;
		                                std::cin >> cModuleId;
		                                if (cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId) == NULL)
		                                {
		                                    std::cout << "*** ERROR !!                                      ***" << std::endl;
		                                    std::cout << "*** This module does not exist !                  ***" << std::endl;
		                                    std::cout << "*** This is not the module you are looking for... ***" << std::endl;
		                                    myflush( std::cin );
		                                    mypause();
		                                }
		                                else
		                                {
		                                    for(uint8_t i=0; i<8; i++)
		                                    {
		                                        if(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getCbc(i) == NULL)
		                                        {
		                                            cCbc = new Cbc(0,0,0,0,i,(boost::format("settings/FE0CBC%d.txt") %(uint32_t(i))).str());
		                                            cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->addCbc(*cCbc);
		                                            cSystemController.fCbcInterface->ConfigureCbc(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getCbc(i));
		                                            delete cCbc;
		                                        }
		                                    }

		                                    std::cout << "*** 8Cbc Hybrid Structure Added ***" << std::endl;
		                                }
		                            }
		                        }
		                        else if(cSId == cSystemController.fShelveVector.size())
		                        {
		                            std::cout << "ERROR : This Shelve does not exist !" << std::endl;
		                            myflush( std::cin );
		                            mypause();
		                        }
		                    }
		                break;


		                default:
		                    std::cout << "*** This is not the option you are looking for... ***" << std::endl;
		                break;
		            }
		        break;


		        case 4:
		        {
		            std::cout << "*** Remove Board ***" << std::endl;
		            std::cout << "--> Which ShelveId ?" << std::endl;
		            std::cin >> cShelveId;
		            std::cout << "--> Which BoardId ?" << std::endl;
		            std::cin >> cBoardId;

		            ShelveVec::iterator cIt;

		            for(cIt=cSystemController.fShelveVector.begin(); cIt!=cSystemController.fShelveVector.end(); cIt++)
		            {
		                if((*cIt)->getShelveId() == cShelveId)
		                {
		                    if((*cIt)->getBoard(cBoardId) == NULL)
		                    {
		                        std::cout << "*** ERROR !!                                     ***" << std::endl;
		                        std::cout << "*** This board does not exist !                  ***" << std::endl;
		                        std::cout << "*** This is not the board you are looking for... ***" << std::endl;
		                    }
		                    else
		                    {
		                        cSystemController.fBeBoardFWMap.erase(cBoardId);

		                        if((*cIt)->removeBoard(cBoardId))
		                            std::cout << "*** Board removed ! ***" << std::endl;
		                        else
		                            std::cout << "*** ERROR in Board removal !! ***" << std::endl;
		                    }
		                }
		            }

		            if(cIt == cSystemController.fShelveVector.end())
		            {
		                std::cout << "ERROR : This Shelve does not exist !" << std::endl;
		                myflush( std::cin );
		                mypause();
		            }
		        }
		        break;


		        case 5:
		            std::cout << "*** Remove Module ***" << std::endl;
		            std::cout << "--> Which ShelveId ?" << std::endl;
		            std::cin >> cShelveId;
		            std::cout << "--> Which BoardId ?" << std::endl;
		            std::cin >> cBoardId;

		            for(uint32_t cSId=0; cSId<cSystemController.fShelveVector.size(); cSId++)
		            {

		                if(cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId)
		                {
		                    if(cSystemController.fShelveVector[cSId]->getBoard(cBoardId) == NULL)
		                    {
		                        std::cout << "ERROR : This board does not exist !" << std::endl;
		                        myflush( std::cin );
		                        mypause();
		                    }
		                    else
		                    {
		                        std::cout << "--> Which ModuleId ?" << std::endl;
		                        std::cin >> cModuleId;

		                        if (cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId) == NULL)
		                        {
		                            std::cout << "*** ERROR !!                                      ***" << std::endl;
		                            std::cout << "*** This module does not exist !                  ***" << std::endl;
		                            std::cout << "*** This is not the module you are looking for... ***" << std::endl;
		                        }
		                        else
		                        {
		                            if(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->removeModule(cModuleId))
		                                std::cout << "*** Module removed ! ***" << std::endl;
		                            else
		                                std::cout << "*** ERROR in Module removal !! ***" << std::endl;
		                        }
		                    }
		                }
		                else if(cSId == cSystemController.fShelveVector.size())
		                {
		                    std::cout << "ERROR : This shelve does not exist !" << std::endl;
		                    myflush( std::cin );
		                    mypause();
		                }
		            }
		        break;


		        case 6:
		            std::cout << "*** Remove Cbc ***" << std::endl;
		            std::cout << "--> Which ShelveId ?" << std::endl;
		            std::cin >> cShelveId;
		            std::cout << "--> Which BoardId ?" << std::endl;
		            std::cin >> cBoardId;

		            for(uint32_t cSId=0; cSId<cSystemController.fShelveVector.size(); cSId++)
		            {

		                if(cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId)
		                {
		                    if(cSystemController.fShelveVector[cSId]->getBoard(cBoardId) == NULL)
		                    {
		                        std::cout << "ERROR : This board does not exist !" << std::endl;
		                        myflush( std::cin );
		                        mypause();
		                    }
		                    else
		                    {
		                        std::cout << "--> Which ModuleId ?" << std::endl;
		                        std::cin >> cModuleId;

		                        if(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId) == NULL)
		                        {
		                            std::cout << "*** ERROR !!                                      ***" << std::endl;
		                            std::cout << "*** This module does not exist !                  ***" << std::endl;
		                            std::cout << "*** This is not the module you are looking for... ***" << std::endl;
		                            myflush( std::cin );
		                            mypause();
		                        }
		                        else
		                        {
		                            std::cout << "--> Which CbcId ?" << std::endl;
		                            std::cin >> cCbcId;
		                            if(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->getCbc(cCbcId) == NULL)
		                            {
		                                std::cout << "*** ERROR !!                                      ***" << std::endl;
		                                std::cout << "*** This Cbc does not exist !                     ***" << std::endl;
		                                std::cout << "*** This is not the Cbc you are looking for...    ***" << std::endl;
		                                myflush( std::cin );
		                                mypause();
		                            }
		                            else
		                            {
		                                if(cSystemController.fShelveVector[cSId]->getBoard(cBoardId)->getModule(cModuleId)->removeCbc(cCbcId))
		                                    std::cout << "*** Module removed ! ***" << std::endl;
		                                else
		                                    std::cout << "*** ERROR in Cbc removal !! ***" << std::endl;
		                            }
		                            std::cout << "*** Cbc Removed ***" << std::endl;
		                        }
		                    }
		                }
		                else if(cSId == cSystemController.fShelveVector.size())
		                {
		                    std::cout << "ERROR : This shelve does not exist !" << std::endl;
		                    myflush( std::cin );
		                    mypause();
		                }
		            }
		        break;


		        default:
		            std::cout << "*** This is not the option you are looking for... ***" << std::endl;
		            myflush( std::cin );
		            mypause();
		        break;

		    }

		   break;
		 */


		case 2:

			std::cout << "\n\n\n\n" << std::endl;
			std::cout << "****************************************************" << std::endl;
			std::cout << "***                  Configure                   ***" << std::endl;
			std::cout << "****************************************************\n" << std::endl;

			std::cout << "1: Configure Board" << std::endl;
			std::cout << "2: Configure Cbc" << std::endl;
			std::cout << "3: Configure all Cbcs in one Module" << std::endl;
			std::cout << "4: Configure all Cbcs in all Modules\n" << std::endl;

			std::cin >> i;

			switch ( i )
			{

			case 1:
				std::cout << "*** Configure Board ***" << std::endl;
				std::cout << "--> Which ShelveId ?" << std::endl;
				std::cin >> cShelveId;
				std::cout << "--> Which BoardId ?" << std::endl;
				std::cin >> cBoardId;

				for ( uint32_t cSId = 0; cSId < cSystemController.fShelveVector.size(); cSId++ )
				{

					if ( cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId )
					{
						if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ) == NULL )
						{
							std::cout << "ERROR : This board does not exist !" << std::endl;
							myflush( std::cin );
							mypause();
						}
						else
						{
							cSystemController.fBeBoardInterface->ConfigureBoard( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ) );
							std::cout << "*** Glib Configured ***" << std::endl;
						}
					}
					else if ( cSId == cSystemController.fShelveVector.size() )
					{
						std::cout << "ERROR : This shelve does not exist !" << std::endl;
						myflush( std::cin );
						mypause();
					}
				}
				break;


			case 2:
				std::cout << "*** Configure Cbc ***" << std::endl;
				std::cout << "--> Which ShelveId ?" << std::endl;
				std::cin >> cShelveId;
				std::cout << "--> Which BoardId ?" << std::endl;
				std::cin >> cBoardId;

				for ( uint32_t cSId = 0; cSId < cSystemController.fShelveVector.size(); cSId++ )
				{

					if ( cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId )
					{
						if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ) == NULL )
						{
							std::cout << "ERROR : This board does not exist !" << std::endl;
							myflush( std::cin );
							mypause();
						}
						else
						{
							std::cout << "--> Which ModuleId ?" << std::endl;
							std::cin >> cModuleId;
							if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId ) == NULL )
							{
								std::cout << "*** ERROR !!                                      ***" << std::endl;
								std::cout << "*** This module does not exist !                  ***" << std::endl;
								std::cout << "*** This is not the module you are looking for... ***" << std::endl;
								myflush( std::cin );
								mypause();
							}
							else
							{
								std::cout << "--> Which CbcId ?" << std::endl;
								std::cin >> cCbcId;
								if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId )->getCbc( cCbcId ) == NULL )
								{
									std::cout << "*** ERROR !!                                      ***" << std::endl;
									std::cout << "*** This Cbc does not exist !                     ***" << std::endl;
									std::cout << "*** This is not the Cbc you are looking for...    ***" << std::endl;
									myflush( std::cin );
									mypause();
								}
								else
									cSystemController.fCbcInterface->ConfigureCbc( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId )->getCbc( cCbcId ) );
								std::cout << "*** Cbc Configured ***" << std::endl;
							}
						}
					}
					else if ( cSId == cSystemController.fShelveVector.size() )
					{
						std::cout << "ERROR : This shelve does not exist !" << std::endl;
						myflush( std::cin );
						mypause();
					}
				}
				break;


			case 3:
				std::cout << "*** Configure all Cbcs in one Module***" << std::endl;
				std::cout << "--> Which ShelveId ?" << std::endl;
				std::cin >> cShelveId;
				std::cout << "--> Which BoardId ?" << std::endl;
				std::cin >> cBoardId;

				for ( uint32_t cSId = 0; cSId < cSystemController.fShelveVector.size(); cSId++ )
				{

					if ( cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId )
					{
						if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ) == NULL )
						{
							std::cout << "ERROR : This board does not exist !" << std::endl;
							myflush( std::cin );
							mypause();
						}
						else
						{
							std::cout << "--> Which ModuleId ?" << std::endl;
							std::cin >> cModuleId;

							if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId ) == NULL )
							{
								std::cout << "*** ERROR !!                                      ***" << std::endl;
								std::cout << "*** This module does not exist !                  ***" << std::endl;
								std::cout << "*** This is not the module you are looking for... ***" << std::endl;
								myflush( std::cin );
								mypause();
							}
							else
							{
								for ( uint8_t cNCbc = 0; cNCbc < cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId )->getNCbc(); cNCbc++ )
								{
									if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId )->getCbc( cNCbc + cMissedCbc ) == NULL )
									{
										cNCbc--;
										cMissedCbc++;
									}

									else
										cSystemController.fCbcInterface->ConfigureCbc( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId )->getCbc( cNCbc + cMissedCbc ) );
								}
								cMissedCbc = 0;
								std::cout << "*** All Cbcs in the Module Configured ***" << std::endl;
							}
						}
					}
					else if ( cSId == cSystemController.fShelveVector.size() )
					{
						std::cout << "ERROR : This shelve does not exist !" << std::endl;
						myflush( std::cin );
						mypause();
					}
				}
				break;


			case 4:
				std::cout << "*** Configure all Cbcs in all Modules ***" << std::endl;
				std::cout << "--> Which ShelveId ?" << std::endl;
				std::cin >> cShelveId;
				std::cout << "--> Which BoardId ?" << std::endl;
				std::cin >> cBoardId;

				for ( uint32_t cSId = 0; cSId < cSystemController.fShelveVector.size(); cSId++ )
				{

					if ( cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId )
					{

						if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ) == NULL )
						{
							std::cout << "ERROR : This board does not exist !" << std::endl;
							myflush( std::cin );
							mypause();
						}
						else
						{
							for ( uint8_t cNFe = 0; cNFe < cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getNFe(); cNFe++ )
							{
								if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cNFe + cMissedModule ) == NULL )
								{
									cNFe--;
									cMissedModule++;
								}

								else
								{
									for ( uint8_t cNCbc = 0; cNCbc < cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cNFe + cMissedModule )->getNCbc(); cNCbc++ )
									{
										if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cNFe + cMissedModule )->getCbc( cNCbc + cMissedCbc ) == NULL )
										{
											cNCbc--;
											cMissedCbc++;
										}

										else
											cSystemController.fCbcInterface->ConfigureCbc( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cNFe + cMissedModule )->getCbc( cNCbc + cMissedCbc ) );
									}
									cMissedCbc = 0;
								}
							}
							cMissedModule = 0;
							std::cout << "*** All Cbcs in all Modules Configured ***" << std::endl;
						}
					}
					else if ( cSId == cSystemController.fShelveVector.size() )
					{
						std::cout << "ERROR : This shelve does not exist !" << std::endl;
						myflush( std::cin );
						mypause();
					}
				}
				break;


			default:
				std::cout << "*** This is not the option you are looking for... ***" << std::endl;
				myflush( std::cin );
				mypause();
				break;

			}

			break;


		case 3:

			std::cout << "\n\n\n\n" << std::endl;
			std::cout << "****************************************************" << std::endl;
			std::cout << "***              Board Manipulation              ***" << std::endl;
			std::cout << "****************************************************\n" << std::endl;

			std::cout << "1: Update both ways" << std::endl;
			std::cout << "2: Update one way" << std::endl;
			std::cout << "3: Get infos\n" << std::endl;

			std::cin >> i;

			switch ( i )
			{

			case 1:
				std::cout << "*** Update both ways ***" << std::endl;
				std::cout << "--> Which ShelveId ?" << std::endl;
				std::cin >> cShelveId;
				std::cout << "--> Which BoardId ?" << std::endl;
				std::cin >> cBoardId;

				for ( uint32_t cSId = 0; cSId < cSystemController.fShelveVector.size(); cSId++ )
				{

					if ( cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId )
					{

						if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ) == NULL )
						{
							std::cout << "ERROR : This board does not exist !" << std::endl;
							myflush( std::cin );
							mypause();
							std::cout << "--> Which Register ?" << std::endl;
						}
						else
						{
							std::cin >> cRegNode;
							std::cout << "--> Which Value ? (0x__)" << std::endl;
							std::cin >> cValue;
							cValueHex = strtoul( cValue.c_str(), 0, 16 );
							if ( cValueHex > 255 )
							{
								std::cout << "*** ERROR !!                                      ***" << std::endl;
								std::cout << "*** This value exceed the maximum value (OxFF) !  ***" << std::endl;
								myflush( std::cin );
								mypause();
							}
							else
							{
								cSystemController.fBeBoardInterface->WriteBoardReg( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ), cRegNode, cValueHex );
								std::cout << "*** Updated ***" << std::endl;
							}
						}
					}
					else if ( cSId == cSystemController.fShelveVector.size() )
					{
						std::cout << "ERROR : This shelve does not exist !" << std::endl;
						myflush( std::cin );
						mypause();
					}
				}
				break;


			case 2:
				std::cout << "*** Update both ways ***" << std::endl;
				std::cout << "--> Which ShelveId ?" << std::endl;
				std::cin >> cShelveId;
				std::cout << "--> Which BoardId ?" << std::endl;
				std::cin >> cBoardId;

				for ( uint32_t cSId = 0; cSId < cSystemController.fShelveVector.size(); cSId++ )
				{

					if ( cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId )
					{

						if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ) == NULL )
						{
							std::cout << "ERROR : This board does not exist !" << std::endl;
							myflush( std::cin );
							mypause();
						}
						else
						{
							std::cout << "*** Update one way ***" << std::endl;
							std::cout << "--> Which Register ?" << std::endl;
							std::cin >> cRegNode;
							cSystemController.fBeBoardInterface->ReadBoardReg( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ), cRegNode );
							std::cout << "*** Updated ***" << std::endl;
						}
					}
					else if ( cSId == cSystemController.fShelveVector.size() )
					{
						std::cout << "ERROR : This shelve does not exist !" << std::endl;
						myflush( std::cin );
						mypause();
					}
				}
				break;


			case 3:
				std::cout << "*** Get Infos ***" << std::endl;
				std::cout << "--> Which ShelveId ?" << std::endl;
				std::cin >> cShelveId;
				std::cout << "--> Which BoardId ?" << std::endl;
				std::cin >> cBoardId;

				for ( uint32_t cSId = 0; cSId < cSystemController.fShelveVector.size(); cSId++ )
				{

					if ( cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId )
					{

						if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ) == NULL )
						{
							std::cout << "ERROR : This board does not exist !" << std::endl;
							myflush( std::cin );
							mypause();
						}
						else
						{
							cSystemController.fBeBoardInterface->getBoardInfo( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ) );
							std::cout << "*** Infos Get ***" << std::endl;
						}
					}
					else if ( cSId == cSystemController.fShelveVector.size() )
					{
						std::cout << "ERROR : This shelve does not exist !" << std::endl;
						myflush( std::cin );
						mypause();
					}
				}
				break;


			default:
				std::cout << "*** This is not the option you are looking for... ***" << std::endl;
				myflush( std::cin );
				mypause();
				break;

			}

			break;


		case 4:

			std::cout << "\n\n\n\n" << std::endl;
			std::cout << "****************************************************" << std::endl;
			std::cout << "***               Cbc Manipulation               ***" << std::endl;
			std::cout << "****************************************************\n" << std::endl;

			std::cout << "1: Write Cbc" << std::endl;
			std::cout << "2: Update Cbc" << std::endl;
			std::cout << "3: Write all Cbc" << std::endl;
			std::cout << "4: Update all Cbc" << std::endl;
			std::cout << "5: Hard Reset a Cbc" << std::endl;
			std::cout << "6: Fast Reset Cbc\n" << std::endl;

			std::cin >> i;

			switch ( i )
			{

			case 1:
				std::cout << "*** Write Cbc ***" << std::endl;
				std::cout << "--> Which ShelveId ?" << std::endl;
				std::cin >> cShelveId;
				std::cout << "--> Which BoardId ?" << std::endl;
				std::cin >> cBoardId;

				for ( uint32_t cSId = 0; cSId < cSystemController.fShelveVector.size(); cSId++ )
				{

					if ( cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId )
					{

						if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ) == NULL )
						{
							std::cout << "ERROR : This board does not exist !" << std::endl;
							myflush( std::cin );
							mypause();
						}
						else
						{
							std::cout << "--> Which ModuleId ?" << std::endl;
							std::cin >> cModuleId;
							if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId ) == NULL )
							{
								std::cout << "*** ERROR !!                                      ***" << std::endl;
								std::cout << "*** This module does not exist !                  ***" << std::endl;
								std::cout << "*** This is not the module you are looking for... ***" << std::endl;
								myflush( std::cin );
								mypause();
							}
							else
							{
								std::cout << "--> Which CbcId ?" << std::endl;
								std::cin >> cCbcId;
								if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId )->getCbc( cCbcId ) == NULL )
								{
									std::cout << "*** ERROR !!                                      ***" << std::endl;
									std::cout << "*** This Cbc does not exist !                     ***" << std::endl;
									std::cout << "*** This is not the Cbc you are looking for...    ***" << std::endl;
									myflush( std::cin );
									mypause();
								}
								else
								{
									std::cout << "--> Which Register ?" << std::endl;
									std::cin >> cRegNode;
									std::cout << "--> Which Value ? (0x__)" << std::endl;
									std::cin >> cValue;
									cValueHex = strtoul( cValue.c_str(), 0, 16 );
									if ( cValueHex > 255 )
									{
										std::cout << "*** ERROR !!                                      ***" << std::endl;
										std::cout << "*** This value exceed the maximum value (OxFF) !  ***" << std::endl;
										myflush( std::cin );
										mypause();
									}
									else
									{
										cSystemController.fCbcInterface->WriteCbcReg( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId )->getCbc( cCbcId ), cRegNode, cValueHex );
										std::cout << "*** Cbc Written ***" << std::endl;
									}
								}
							}
						}
					}
					else if ( cSId == cSystemController.fShelveVector.size() )
					{
						std::cout << "ERROR : This shelve does not exist !" << std::endl;
						myflush( std::cin );
						mypause();
					}
				}
				break;


			case 2:
				std::cout << "*** Update Cbc ***" << std::endl;
				std::cout << "--> Which ShelveId ?" << std::endl;
				std::cin >> cShelveId;
				std::cout << "--> Which BoardId ?" << std::endl;
				std::cin >> cBoardId;

				for ( uint32_t cSId = 0; cSId < cSystemController.fShelveVector.size(); cSId++ )
				{

					if ( cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId )
					{

						if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ) == NULL )
						{
							std::cout << "ERROR : This board does not exist !" << std::endl;
							myflush( std::cin );
							mypause();
						}
						else
						{
							std::cout << "--> Which ModuleId ?" << std::endl;
							std::cin >> cModuleId;
							if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId ) == NULL )
							{
								std::cout << "*** ERROR !!                                      ***" << std::endl;
								std::cout << "*** This module does not exist !                  ***" << std::endl;
								std::cout << "*** This is not the module you are looking for... ***" << std::endl;
								myflush( std::cin );
								mypause();
							}
							else
							{
								std::cout << "--> Which CbcId ?" << std::endl;
								std::cin >> cCbcId;
								if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId )->getCbc( cCbcId ) == NULL )
								{
									std::cout << "*** ERROR !!                                      ***" << std::endl;
									std::cout << "*** This Cbc does not exist !                     ***" << std::endl;
									std::cout << "*** This is not the Cbc you are looking for...    ***" << std::endl;
									myflush( std::cin );
									mypause();
								}
								else
								{
									std::cout << "--> Which Register ?" << std::endl;
									std::cin >> cRegNode;
									cSystemController.fCbcInterface->ReadCbcReg( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId )->getCbc( cCbcId ), cRegNode );
									std::cout << "*** Updated ***" << std::endl;
								}
							}
						}
					}
					else if ( cSId == cSystemController.fShelveVector.size() )
					{
						std::cout << "ERROR : This shelve does not exist !" << std::endl;
						myflush( std::cin );
						mypause();
					}
				}
				break;


			case 3:
				std::cout << "*** Write all Cbc ***" << std::endl;
				std::cout << "--> Which ShelveId ?" << std::endl;
				std::cin >> cShelveId;
				std::cout << "--> Which BoardId ?" << std::endl;
				std::cin >> cBoardId;

				for ( uint32_t cSId = 0; cSId < cSystemController.fShelveVector.size(); cSId++ )
				{

					if ( cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId )
					{

						if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ) == NULL )
						{
							std::cout << "ERROR : This board does not exist !" << std::endl;
							myflush( std::cin );
							mypause();
						}
						else
						{
							std::cout << "--> Which ModuleId ?" << std::endl;
							std::cin >> cModuleId;
							if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId ) == NULL )
							{
								std::cout << "*** ERROR !!                                      ***" << std::endl;
								std::cout << "*** This module does not exist !                  ***" << std::endl;
								std::cout << "*** This is not the module you are looking for... ***" << std::endl;
								myflush( std::cin );
								mypause();
							}
							else
							{
								std::cout << "--> Which Register ?" << std::endl;
								std::cin >> cRegNode;
								std::cout << "--> Which Value ? (0x__)" << std::endl;
								std::cin >> cValue;
								cValueHex = strtoul( cValue.c_str(), 0, 16 );
								if ( cValueHex > 255 )
								{
									std::cout << "*** ERROR !!                                      ***" << std::endl;
									std::cout << "*** This value exceed the maximum value (OxFF) !  ***" << std::endl;
									myflush( std::cin );
									mypause();
								}
								else
								{
									cSystemController.fCbcInterface->WriteBroadcast( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId ), cRegNode, cValueHex );
									std::cout << "*** All Cbc Written ***" << std::endl;
								}
							}
						}
					}
					else if ( cSId == cSystemController.fShelveVector.size() )
					{
						std::cout << "ERROR : This shelve does not exist !" << std::endl;
						myflush( std::cin );
						mypause();
					}
				}
				break;


			case 4:
				std::cout << "*** Read all Cbc ***" << std::endl;
				std::cout << "--> Which ShelveId ?" << std::endl;
				std::cin >> cShelveId;
				std::cout << "--> Which BoardId ?" << std::endl;
				std::cin >> cBoardId;

				for ( uint32_t cSId = 0; cSId < cSystemController.fShelveVector.size(); cSId++ )
				{

					if ( cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId )
					{

						if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ) == NULL )
						{
							std::cout << "ERROR : This board does not exist !" << std::endl;
							myflush( std::cin );
							mypause();
						}
						else
						{
							std::cout << "--> Which ModuleId ?" << std::endl;
							std::cin >> cModuleId;
							if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId ) == NULL )
							{
								std::cout << "*** ERROR !!                                      ***" << std::endl;
								std::cout << "*** This module does not exist !                  ***" << std::endl;
								std::cout << "*** This is not the module you are looking for... ***" << std::endl;
								myflush( std::cin );
								mypause();
							}
							else
							{
								cSystemController.fCbcInterface->ReadAllCbc( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId ) );
								std::cout << "*** All Cbc Updated ***" << std::endl;
							}
						}
					}
					else if ( cSId == cSystemController.fShelveVector.size() )
					{
						std::cout << "ERROR : This shelve does not exist !" << std::endl;
						myflush( std::cin );
						mypause();
					}
				}
				break;


			case 5:
				std::cout << "*** Cbc Hard Reset ***" << std::endl;
				std::cout << "--> Which ShelveId ?" << std::endl;
				std::cin >> cShelveId;
				std::cout << "--> Which BoardId ?" << std::endl;
				std::cin >> cBoardId;

				for ( uint32_t cSId = 0; cSId < cSystemController.fShelveVector.size(); cSId++ )
				{

					if ( cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId )
					{

						if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ) == NULL )
						{
							std::cout << "ERROR : This board does not exist !" << std::endl;
							myflush( std::cin );
							mypause();
						}
						else
						{
							std::cout << "--> Which ModuleId ?" << std::endl;
							std::cin >> cModuleId;
							if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId ) == NULL )
							{
								std::cout << "*** ERROR !!                                      ***" << std::endl;
								std::cout << "*** This module does not exist !                  ***" << std::endl;
								std::cout << "*** This is not the module you are looking for... ***" << std::endl;
								myflush( std::cin );
								mypause();
							}
							else
							{
								std::cout << "--> Which CbcId ?" << std::endl;
								std::cin >> cCbcId;
								if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId )->getCbc( cCbcId ) == NULL )
								{
									std::cout << "*** ERROR !!                                      ***" << std::endl;
									std::cout << "*** This Cbc does not exist !                     ***" << std::endl;
									std::cout << "*** This is not the Cbc you are looking for...    ***" << std::endl;
									myflush( std::cin );
									mypause();
								}
								else
								{
									cSystemController.fCbcInterface->CbcHardReset( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId )->getCbc( cCbcId ) );
									std::cout << "*** Cbc Hard Reset ***" << std::endl;
								}
							}
						}
					}
					else if ( cSId == cSystemController.fShelveVector.size() )
					{
						std::cout << "ERROR : This shelve does not exist !" << std::endl;
						myflush( std::cin );
						mypause();
					}
				}
				break;


			case 6:
				std::cout << "*** Cbc Fast Reset ***" << std::endl;
				std::cout << "--> Which ShelveId ?" << std::endl;
				std::cin >> cShelveId;
				std::cout << "--> Which BoardId ?" << std::endl;
				std::cin >> cBoardId;

				for ( uint32_t cSId = 0; cSId < cSystemController.fShelveVector.size(); cSId++ )
				{

					if ( cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId )
					{

						if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ) == NULL )
						{
							std::cout << "ERROR : This board does not exist !" << std::endl;
							myflush( std::cin );
							mypause();
						}
						else
						{
							std::cout << "--> Which ModuleId ?" << std::endl;
							std::cin >> cModuleId;
							if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId ) == NULL )
							{
								std::cout << "*** ERROR !!                                      ***" << std::endl;
								std::cout << "*** This module does not exist !                  ***" << std::endl;
								std::cout << "*** This is not the module you are looking for... ***" << std::endl;
								myflush( std::cin );
								mypause();
							}
							else
							{
								std::cout << "--> Which CbcId ?" << std::endl;
								std::cin >> cCbcId;
								if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId )->getCbc( cCbcId ) == NULL )
								{
									std::cout << "*** ERROR !!                                      ***" << std::endl;
									std::cout << "*** This Cbc does not exist !                     ***" << std::endl;
									std::cout << "*** This is not the Cbc you are looking for...    ***" << std::endl;
									myflush( std::cin );
									mypause();
								}
								else
								{
									cSystemController.fCbcInterface->CbcFastReset( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cModuleId )->getCbc( cCbcId ) );
									std::cout << "*** Cbc Fast Reset ***" << std::endl;
								}
							}
						}
					}
					else if ( cSId == cSystemController.fShelveVector.size() )
					{
						std::cout << "ERROR : This shelve does not exist !" << std::endl;
						myflush( std::cin );
						mypause();
					}
				}
				break;


			default:
				std::cout << "*** This is not the option you are looking for... ***" << std::endl;
				myflush( std::cin );
				mypause();
				break;

			}

			break;


		case 5:

			std::cout << "\n\n\n\n" << std::endl;
			std::cout << "****************************************************" << std::endl;
			std::cout << "***               Acquisition-ish                ***" << std::endl;
			std::cout << "****************************************************\n" << std::endl;

			std::cout << "1: Read Data\n" << std::endl;

			std::cin >> i;

			switch ( i )
			{

			case 1:
			{
				std::cout << "*** Run Acquisition ***" << std::endl;
				std::cout << "--> Which ShelveId ?" << std::endl;
				std::cin >> cShelveId;
				std::cout << "--> Which BoardId ?" << std::endl;
				std::cin >> cBoardId;

				for ( uint32_t cSId = 0; cSId < cSystemController.fShelveVector.size(); cSId++ )
				{

					if ( cSystemController.fShelveVector[cSId]->getShelveId() == cShelveId )
					{

						if ( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ) == NULL )
						{
							std::cout << "ERROR : This board does not exist !" << std::endl;
							myflush( std::cin );
							mypause();
						}
						else
						{
							std::cout << "--> Which VcthMin ?" << std::endl;
							std::cin >> cVcthMin;
							std::cout << "--> Which VcthMax ?" << std::endl;
							std::cin >> cVcthMax;
							std::cout << "--> Which VcthStep ?" << std::endl;
							std::cin >> cVcthStep;
							std::cout << "--> How many events ?" << std::endl;
							std::cin >> cNevents;

							uint32_t cNthAcq = 0;
							TCanvas* cCanvas;

							if ( uint32_t( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( 0 )->getNCbc() ) == 2 )
							{
								cCanvas = new TCanvas( "Data Acq", "Different hits counters", 1000, 800 );
								cCanvas->Divide( 2, 1 );
							}
							else
							{
								cCanvas = new TCanvas( "Data Acq", "Different hits counters", 1200, 700 );
								cCanvas->Divide( 4, 2 );
							}

							std::vector<TH1F*> cHistVec;
							gStyle->SetOptStat( kFALSE );

							for ( uint8_t cNCbc = 0; cNCbc < cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( 0 )->getNCbc(); cNCbc++ )
								cHistVec.push_back( new TH1F( Form( "Histo_Hits_CBC%d", cNCbc ), Form( "Occupancy_CBC%d", cNCbc ), 255, -0.5, 254.5 ) );

							usleep( 100 );

							for ( uint32_t cVCth = cVcthMin; cVCth < cVcthMax; cVCth += cVcthStep )
							{
								for ( uint32_t cNCbc = 0; cNCbc < cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( 0 )->getNCbc(); cNCbc++ )
									cSystemController.fCbcInterface->WriteCbcReg( cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( 0 )->getCbc( cNCbc ), "VCth", cVCth );

								uint32_t cN = 0;

								for ( uint8_t cNCbc = 0; cNCbc < cSystemController.fShelveVector[0]->getBoard( cBoardId )->getModule( 0 )->getNCbc(); cNCbc++ )
								{
									delete cHistVec[cNCbc];
									cHistVec[cNCbc] = new TH1F( Form( "Histo_Hits_CBC%d", cNCbc ), Form( "Occupancy_CBC%d", cNCbc ), 255, -0.5, 254.5 );
								}

								while ( !( cNevents != 0 && cN == cNevents ) )
								{

									cSystemController.Run( cSystemController.fShelveVector[0]->getBoard( cBoardId ), cNthAcq );

									const Event* cEvent = cSystemController.fBeBoardInterface->GetNextEvent( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ) );

									while ( cEvent )
									{

										if ( cNevents != 0 && cN == cNevents )
											break;

										for ( uint8_t cNFe = 0; cNFe < cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getNFe(); cNFe++ )
										{

											for ( uint8_t cNCbc = 0; cNCbc < cSystemController.fShelveVector[cSId]->getBoard( cBoardId )->getModule( cNFe )->getNCbc(); cNCbc++ )
											{
												uint32_t cNHits = 0;
												std::vector<bool> cDataBitVector = cEvent->DataBitVector( cNFe, cNCbc );

												for ( uint32_t cDBVec = 0; cDBVec < cDataBitVector.size(); cDBVec++ )
												{
													if ( cDataBitVector[cDBVec] )
														cNHits++;
												}
												cHistVec[cNCbc]->Fill( cNHits );
											}
										}

										cEvent = cSystemController.fBeBoardInterface->GetNextEvent( cSystemController.fShelveVector[cSId]->getBoard( cBoardId ) );

										cN++;
									}

									for ( uint8_t m = 0; m < cHistVec.size(); m++ )
									{
										cCanvas->cd( uint32_t( m + 1 ) );
										cHistVec[m]->Draw();
									}

									cCanvas->Update();

									if ( cN == cNevents )
										break;

								}

								cCanvas->Print( ( ( boost::format( "output/Histogram_Vcth_%d.pdf" ) % ( cVCth ) ).str() ).c_str() );

								//delete cCanvas;
							}

							std::cout << "*** Acquisition Run ***" << std::endl;
						}
						break;
					}
					else if ( cSId == cSystemController.fShelveVector.size() )
					{
						std::cout << "ERROR : This shelve does not exist !" << std::endl;
						myflush( std::cin );
						mypause();
					}
				}
			}
			break;


			default:
				std::cout << "*** This is not the option you are looking for... ***" << std::endl;
				myflush( std::cin );
				mypause();
				break;

			}

			break;


		case 6:
		{

			std::cout << "\n\n" << std::endl;
			std::cout << "****************************************************" << std::endl;
			std::cout << "***             Configuration Recap'             ***" << std::endl;
			std::cout << "****************************************************\n" << std::endl;

			std::cout << "1: Full Recap'" << std::endl;
			std::cout << "2: Fast Recap'\n" << std::endl;

			std::cin >> i;

			switch ( i )
			{

			case 1:
			{
				std::cout << "\n\n" << std::endl;
				std::cout << "****************************************************" << std::endl;
				std::cout << "***           Configuration Full Recap'          ***" << std::endl;
				std::cout << "****************************************************\n" << std::endl;

				for ( BeBoardFWMap::iterator cIt = cSystemController.fBeBoardFWMap.begin(); cIt != cSystemController.fBeBoardFWMap.end(); ++cIt )
				{

					for ( uint32_t cSId = 0; cSId < cSystemController.fShelveVector.size(); cSId++ )
					{

						if ( cSystemController.fShelveVector[cSId]->getBoard( cIt->first ) == NULL )
						{
							std::cout << "ERROR : This board does not exist !" << std::endl;
							myflush( std::cin );
							mypause();
						}
						else
						{
							std::cout << "****************************************************" << std::endl;
							std::cout << "***               Board Configuration             ***" << std::endl;
							std::cout << "****************************************************\n" << std::endl;

							std::cout << " ---------------------- " << std::endl;
							std::cout << "| Number of Module : " << uint32_t( cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getNFe() ) << std::endl;
							std::cout << "| Be Id : " << uint32_t( cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getBeId() ) << std::endl;
							std::cout << "| Shelve Id : " << uint32_t( cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getShelveId() ) << std::endl;
							std::cout << " ---------------------- \n\n" << std::endl;

							std::cout << " *** Board Register Map *** " << std::endl;
							std::cout << " -------------------------------------------------------------- " << std::endl;

							BeBoardRegMap cGlibRegMap = cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getBeBoardRegMap();
							for ( BeBoardRegMap::iterator cJt = cGlibRegMap.begin(); cJt != cGlibRegMap.end(); ++cJt )
								std::cout << "| " << cJt->first << " : " << uint32_t( cJt->second ) << std::endl;

							std::cout << " -------------------------------------------------------------- \n\n" << std::endl;

							myflush( std::cin );
							mypause();

							std::cout << "****************************************************" << std::endl;
							std::cout << "***      Module and Cbc Configuration            ***" << std::endl;
							std::cout << "****************************************************" << std::endl;

							for ( uint8_t cNFe = 0; cNFe < cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getNFe(); cNFe++ )
							{

								if ( cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getModule( cNFe + cMissedModule ) == NULL )
								{
									cNFe--;
									cMissedModule++;
								}

								else
								{

									std::cout << " \n\n*** Module *** " << std::endl;
									std::cout << " --------------------- " << std::endl;
									std::cout << "| Module Id : " << uint32_t( cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getModule( cNFe + cMissedModule )->getModuleId() ) << std::endl;
									std::cout << "| Number of Cbc : " << uint32_t( cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getModule( cNFe + cMissedModule )->getNCbc() ) << std::endl;
									std::cout << " --------------------- \n" << std::endl;

									cSystemController.fCbcInterface->ReadAllCbc( cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getModule( cModuleId ) );

									for ( uint8_t cNCbc = 0; cNCbc < cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getModule( cNFe + cMissedModule )->getNCbc(); cNCbc++ )
									{
										if ( cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getModule( cNFe + cMissedModule )->getCbc( cNCbc + cMissedCbc ) == NULL )
										{
											cNCbc--;
											cMissedCbc++;
										}

										else
										{

											std::cout << "     *** Cbc contained *** " << std::endl;
											std::cout << "     ------------------------- " << std::endl;
											std::cout << "    | Cbc Id : " << uint32_t( cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getModule( cNFe + cMissedModule )->getCbc( cNCbc + cMissedCbc )->getCbcId() ) << std::endl;
											std::cout << "     ------------------------- " << std::endl;

											CbcRegMap cCbcRegMap = cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getModule( cNFe + cMissedModule )->getCbc( cNCbc + cMissedCbc )->getRegMap();

											for ( CbcRegMap::iterator cJt = cCbcRegMap.begin(); cJt != cCbcRegMap.end(); ++cJt )
												std::cout << "    | " << cJt->first << " : 0x" << std::hex << uint32_t( ( cJt->second ).fValue ) << std::endl;
											std::cout << "     ------------------------- " << std::endl;

											mypause();
										}
									}
								}

								cMissedCbc = 0;
							}

							cMissedModule = 0;
						}
					}
				}
			}
			break;

			case 2:
			{
				std::cout << "\n\n\n\n" << std::endl;
				std::cout << "****************************************************" << std::endl;
				std::cout << "***          Configuration Fast Recap'           ***" << std::endl;
				std::cout << "****************************************************\n" << std::endl;

				for ( BeBoardFWMap::iterator cIt = cSystemController.fBeBoardFWMap.begin(); cIt != cSystemController.fBeBoardFWMap.end(); ++cIt )
				{

					for ( uint32_t cSId = 0; cSId < cSystemController.fShelveVector.size(); cSId++ )
					{

						if ( cSystemController.fShelveVector[cSId]->getBoard( cIt->first ) == NULL )
						{
							std::cout << "ERROR : This board does not exist !" << std::endl;
							myflush( std::cin );
							mypause();
						}
						else
						{
							std::cout << "****************************************************" << std::endl;
							std::cout << "***               Board Configuration            ***" << std::endl;
							std::cout << "****************************************************\n" << std::endl;

							std::cout << " ---------------------- " << std::endl;
							std::cout << "| Number of Module : " << uint32_t( cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getNFe() ) << " |" << std::endl;
							std::cout << "| Be Id : " << uint32_t( cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getBeId() ) << "            |" << std::endl;
							std::cout << "| Shelve Id : " << uint32_t( cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getShelveId() ) << "        |" << std::endl;
							std::cout << " ---------------------- \n\n" << std::endl;

							myflush( std::cin );
							mypause();

							std::cout << "****************************************************" << std::endl;
							std::cout << "***        Module and Cbc Configuration          ***" << std::endl;
							std::cout << "****************************************************" << std::endl;

							for ( uint8_t cNFe = 0; cNFe < cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getNFe(); cNFe++ )
							{

								if ( cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getModule( cNFe + cMissedModule ) == NULL )
								{
									cNFe--;
									cMissedModule++;
								}

								else
								{

									std::cout << " \n\n*** Module *** " << std::endl;
									std::cout << " --------------------- " << std::endl;
									std::cout << "| Module Id : " << uint32_t( cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getModule( cNFe + cMissedModule )->getModuleId() ) << std::endl;
									std::cout << "| Number of Cbc : " << uint32_t( cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getModule( cNFe + cMissedModule )->getNCbc() ) << std::endl;
									std::cout << " --------------------- \n" << std::endl;

									cSystemController.fCbcInterface->ReadAllCbc( cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getModule( cModuleId ) );

									for ( uint8_t cNCbc = 0; cNCbc < cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getModule( cNFe + cMissedModule )->getNCbc(); cNCbc++ )
									{
										if ( cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getModule( cNFe + cMissedModule )->getCbc( cNCbc + cMissedCbc ) == NULL )
										{
											cNCbc--;
											cMissedCbc++;
										}

										else
										{
											std::cout << "     *** Cbc contained *** " << std::endl;
											std::cout << "     ------------------------- " << std::endl;
											std::cout << "    | Cbc Id : " << uint32_t( cSystemController.fShelveVector[cSId]->getBoard( cIt->first )->getModule( cNFe + cMissedModule )->getCbc( cNCbc + cMissedCbc )->getCbcId() ) << std::endl;
											std::cout << "     ------------------------- " << std::endl;
										}
									}

									mypause();
								}

								cMissedCbc = 0;

							}

							cMissedModule = 0;
						}
					}
				}
			}
			break;

			default:
				std::cout << "*** This is not the option you are looking for... ***" << std::endl;
				myflush( std::cin );
				mypause();
				break;

			}

			break;


			case 0:
				std::cout << "Quit..." << std::endl;
				break;


			default:
				std::cout << "*** This is not the option you are looking for... ***" << std::endl;
				myflush( std::cin );
				mypause();
				break;

			}
		}
	}
	while ( i != 0 );

	for ( uint32_t cSId = 0; cSId < cSystemController.fShelveVector.size(); cSId++ )
	{

		for ( uint32_t cBId; cBId < cSystemController.fShelveVector[cSId]->getNBoard(); cBId++ )
		{
			if ( cSystemController.fShelveVector[cSId]->getBoard( cBId + cMissedBoard ) == NULL )
			{
				cBId--;
				cMissedBoard++;
			}

			else
			{
				for ( uint8_t cNFe = 0; cNFe < cSystemController.fShelveVector[cSId]->getBoard( cBId + cMissedBoard )->getNFe(); cNFe++ )
				{

					if ( cSystemController.fShelveVector[cSId]->getBoard( cBId + cMissedBoard )->getModule( cNFe + cMissedModule ) == NULL )
					{
						cNFe--;
						cMissedModule++;
					}

					else
					{
						for ( uint8_t cNCbc = 0; cNCbc < cSystemController.fShelveVector[cSId]->getBoard( cBId + cMissedBoard )->getModule( cNFe + cMissedModule )->getNCbc(); cNCbc++ )
						{
							if ( cSystemController.fShelveVector[cSId]->getBoard( cBId + cMissedBoard )->getModule( cNFe + cMissedModule )->getCbc( cNCbc + cMissedCbc ) == NULL )
							{
								cNCbc--;
								cMissedCbc++;
							}

							else
								cSystemController.fShelveVector[cSId]->getBoard( cBId + cMissedBoard )->getModule( cNFe + cMissedModule )->getCbc( cNCbc + cMissedCbc )->saveRegMap( ( boost::format( "output/output_%d_%d_%d.txt" ) % ( uint32_t( cSystemController.fShelveVector[cSId]->getBoard( cBId + cMissedBoard )->getBeId() ) ) % ( uint32_t( cNFe + cMissedModule ) ) % ( uint32_t( cNCbc + cMissedCbc ) ) ).str() );
						}

						cMissedCbc = 0;
					}
				}
			}

		}
	}

	//Root cApp
	cApp.Run();

	std::cout << "\n\nEnd of the MCP program...\n\n" << std::endl;

	return 0;
}
