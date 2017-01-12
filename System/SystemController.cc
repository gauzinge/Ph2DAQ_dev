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

namespace Ph2_System {

    SystemController::SystemController()
        : fFileHandler (nullptr),
          fWriteHandlerEnabled (false),
          fData (nullptr)
    {
    }

    SystemController::~SystemController()
    {
    }
    void SystemController::Destroy()
    {
        if (fFileHandler)
        {
            if (fFileHandler->file_open() ) fFileHandler->closeFile();

            delete fFileHandler;
        }

        delete fBeBoardInterface;
        delete fCbcInterface;
        fBeBoardFWMap.clear();
        fSettingsMap.clear();

        for ( auto& el : fBoardVector )
            delete el;

        fBoardVector.clear();

        if (fData) delete fData;
    }

    void SystemController::addFileHandler ( const std::string& pFilename, char pOption )
    {
        //if the opion is read, create a handler object and use it to read the
        //file in the method below!
        if (pOption == 'r')
            fFileHandler = new FileHandler ( pFilename, pOption );
        //if the option is w, remember the filename and construct a new
        //fileHandler for every Interface
        else if (pOption == 'w')
        {
            fRawFileName = pFilename;
            fWriteHandlerEnabled = true;
        }
    }

    void SystemController::readFile ( std::vector<uint32_t>& pVec, uint32_t pNWords32 )
    {
        if (pNWords32 == 0) pVec = fFileHandler->readFile( );
        else pVec = fFileHandler->readFileChunks (pNWords32);
    }


    void SystemController::InitializeHw ( const std::string& pFilename, std::ostream& os )
    {
        this->fParser.parseHW (pFilename, fBeBoardFWMap, fBoardVector, os );

        fBeBoardInterface = new BeBoardInterface ( fBeBoardFWMap );
        fCbcInterface = new CbcInterface ( fBeBoardFWMap );

        if (fWriteHandlerEnabled)
            this->initializeFileHandler();
    }

    void SystemController::InitializeSettings ( const std::string& pFilename, std::ostream& os )
    {
        this->fParser.parseSettings (pFilename, fSettingsMap, os );
    }

    void SystemController::ConfigureHw ( bool bIgnoreI2c )
    {
        LOG (INFO) << BOLDBLUE << "Configuring HW parsed from .xml file: " << RESET;

        bool cHoleMode, cCheck;

        if ( !fSettingsMap.empty() )
        {
            SettingsMap::iterator cSetting = fSettingsMap.find ( "HoleMode" );

            if ( cSetting != fSettingsMap.end() )
                cHoleMode = cSetting->second;

            cCheck = true;
        }
        else cCheck = false;

        for (auto& cBoard : fBoardVector)
        {
            fBeBoardInterface->ConfigureBoard ( cBoard );
            fBeBoardInterface->CbcHardReset ( cBoard );

            if ( cCheck && cBoard->getBoardType() == BoardType::GLIB)
            {
                fBeBoardInterface->WriteBoardReg ( cBoard, "pc_commands2.negative_logic_CBC", ( ( cHoleMode ) ? 0 : 1 ) );
                LOG (INFO) << GREEN << "Overriding GLIB register values for signal polarity with value from settings node!" << RESET;
            }

            LOG (INFO) << GREEN << "Successfully configured Board " << int ( cBoard->getBeId() ) << RESET;

            for (auto& cFe : cBoard->fModuleVector)
            {
                for (auto& cCbc : cFe->fCbcVector)
                {
                    if ( !bIgnoreI2c )
                    {
                        fCbcInterface->ConfigureCbc ( cCbc );
                        LOG (INFO) << GREEN <<  "Successfully configured Cbc " << int ( cCbc->getCbcId() ) << RESET;
                    }
                }
            }

            //CbcFastReset as per recommendation of Mark Raymond
            fBeBoardInterface->CbcFastReset ( cBoard );
        }
    }

    void SystemController::initializeFileHandler()
    {
        LOG (INFO) << BOLDBLUE << "Saving binary raw data to: " << fRawFileName << ".fedId" << RESET ;

        // here would be the ideal position to fill the file Header and call openFile when in read mode
        for (const auto& cBoard : fBoardVector)
        {
            std::string cBoardTypeString;
            BoardType cBoardType = cBoard->getBoardType();

            if (cBoardType == BoardType::GLIB) cBoardTypeString = "GLIB";
            else if (cBoardType == BoardType::CTA) cBoardTypeString = "CTA";
            else if (cBoardType == BoardType::ICGLIB) cBoardTypeString = "ICGLIB";
            else if (cBoardType == BoardType::ICFC7) cBoardTypeString = "ICFC7";
            else if (cBoardType == BoardType::CBC3FC7) cBoardTypeString = "CBC3FC7";


            uint32_t cBeId = cBoard->getBeId();
            uint32_t cNCbc = 0;

            for (const auto& cFe : cBoard->fModuleVector)
                cNCbc += cFe->getNCbc();

            //nCbcDataSize is set to 0 if not explicitly set
            uint32_t cNEventSize32 = (cBoard->getNCbcDataSize() == 0 ) ? EVENT_HEADER_TDC_SIZE_32 + cNCbc * CBC_EVENT_SIZE_32 : EVENT_HEADER_TDC_SIZE_32 + cBoard->getNCbcDataSize() * CBC_EVENT_SIZE_32;

            uint32_t cFWWord = fBeBoardInterface->getBoardInfo (cBoard);
            uint32_t cFWMajor = (cFWWord & 0xFFFF0000) >> 16;
            uint32_t cFWMinor = (cFWWord & 0x0000FFFF);

            //with the above info fill the header
            FileHeader cHeader (cBoardTypeString, cFWMajor, cFWMinor, cBeId, cNCbc, cNEventSize32);

            //construct a Handler
            std::stringstream cBeBoardString;
            cBeBoardString << "_fed" << std::setw (3) << std::setfill ('0') << cBeId;
            std::string cFilename = fRawFileName;

            if (fRawFileName.find (".raw") != std::string::npos)
                cFilename.insert (fRawFileName.find (".raw"), cBeBoardString.str() );

            FileHandler* cHandler = new FileHandler (cFilename, 'w', cHeader);

            //finally set the handler
            fBeBoardInterface->SetFileHandler (cBoard, cHandler);
        }
    }

    uint32_t SystemController::ReadData (BeBoard* pBoard)
    {
        //reset the data object
        if (fData) delete fData;

        fData = new Data();

        std::vector<uint32_t> cData;
        //read the data and get it by reference
        uint32_t cNPackets = fBeBoardInterface->ReadData (pBoard, false, cData);
        //pass data by reference to set and let it know what board we are dealing with
        fData->Set (pBoard, cData, cNPackets, fBeBoardInterface->getBoardType (pBoard) );
        //return the packet size
        return cNPackets;
    }

    void SystemController::ReadData()
    {
        for (auto cBoard : fBoardVector)
            this->ReadData (cBoard);

    }

    void SystemController::ReadNEvents (BeBoard* pBoard, uint32_t pNEvents)
    {
        //reset the data object
        if (fData) delete fData;


        fData = new Data();
        std::vector<uint32_t> cData;
        //read the data and get it by reference
        fBeBoardInterface->ReadNEvents (pBoard, pNEvents, cData);
        //pass data by reference to set and let it know what board we are dealing with
        fData->Set (pBoard, cData, pNEvents, fBeBoardInterface->getBoardType (pBoard) );
        //return the packet size
    }

    void SystemController::ReadNEvents (uint32_t pNEvents)
    {
        for (auto cBoard : fBoardVector)
            this->ReadNEvents (cBoard, pNEvents);
    }
}
