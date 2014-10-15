/*

        FileName :                    GlibFWInterface.h
        Content :                     GlibFWInterface init/config of the Glib and its Cbc's
        Programmer :                  Lorenzo BIDEGAIN, Nicolas PIERRE
        Version :                     1.0
        Date of creation :            28/07/14
        Support :                     mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com

 */

#include <uhal/uhal.hpp>
#include <boost/date_time.hpp>
#include <boost/thread.hpp>
#include <time.h>
#include "GlibFWInterface.h"

#define DEV_FLAG         0

namespace Ph2_HwInterface
{

	GlibFWInterface::GlibFWInterface( const char* puHalConfigFileName, uint32_t pBoardId ) :
		BeBoardFWInterface( puHalConfigFileName, pBoardId )
	{

	}


	void GlibFWInterface::ConfigureBoard( BeBoard* pBoard )
	{

		//We may here switch in the future with the StackReg method of the RegManager
		//when the timeout thing will be implemented in a transparent and pretty way

		std::vector< std::pair<std::string, uint32_t> > cVecReg;
		std::pair<std::string, uint32_t> cPairReg;

		boost::posix_time::milliseconds cPause( 200 );

		//Primary Configuration
		cPairReg.first = PC_CONFIG_OK;
		cPairReg.second = 1;
		cVecReg.push_back( cPairReg );
		cPairReg.first = SRAM1_END_READOUT;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = SRAM2_END_READOUT;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = SRAM1_USR_LOGIC;
		cPairReg.second = 1;
		cVecReg.push_back( cPairReg );
		cPairReg.first = SRAM2_USR_LOGIC;
		cPairReg.second = 1;
		cVecReg.push_back( cPairReg );

		WriteStackReg( cVecReg );

		cVecReg.clear();

		boost::this_thread::sleep( cPause );


		/*
		        GlibRegMap : map<std::string,uint8_t> created from Glib class

		   Mandatory to go through a created cGlibRegMap.
		   If you want to put directly pGlib.getGlibRegMap(), you'll end up with
		   a seg fault error, as it is not putting all the map in mem but only
		   begin() and end().
		 */

		BeBoardRegMap cGlibRegMap = pBoard->getBeBoardRegMap();
		for ( BeBoardRegMap::iterator cIt = cGlibRegMap.begin(); cIt != cGlibRegMap.end(); ++cIt )
		{
			cPairReg.first = cIt->first;
			cPairReg.second = cIt->second;
			cVecReg.push_back( cPairReg );
		}

		WriteStackReg( cVecReg );

		cVecReg.clear();

		cPairReg.first = SPURIOUS_FRAME;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = FORCE_BG0_START;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = CBC_TRIGGER_1SHOT;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = BREAK_TRIGGER;
		cPairReg.second = 1;
		cVecReg.push_back( cPairReg );

		WriteStackReg( cVecReg );

		cVecReg.clear();


		cPairReg.first = PC_CONFIG_OK;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = SRAM1_END_READOUT;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = SRAM2_END_READOUT;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = SRAM1_USR_LOGIC;
		cPairReg.second = 1;
		cVecReg.push_back( cPairReg );
		cPairReg.first = SRAM2_USR_LOGIC;
		cPairReg.second = 1;
		cVecReg.push_back( cPairReg );

		WriteStackReg( cVecReg );

		cVecReg.clear();

		boost::this_thread::sleep( cPause );


		cPairReg.first = SPURIOUS_FRAME;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = FORCE_BG0_START;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = CBC_TRIGGER_1SHOT;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = BREAK_TRIGGER;
		cPairReg.second = 1;
		cVecReg.push_back( cPairReg );

		WriteStackReg( cVecReg );

		cVecReg.clear();

		boost::this_thread::sleep( cPause * 3 );

	}


	void GlibFWInterface::SelectFEId()
	{
		if ( uint32_t( ReadReg( HYBRID_TYPE ) ) == 8 )
		{
			fCbcStubLat  = ( uint32_t( ReadReg( FMC1_PRESENT ) ) ? CBC_STUB_LATENCY_FE1 : CBC_STUB_LATENCY_FE2 );
			fCbcI2CCmdAck = ( uint32_t( ReadReg( FMC1_PRESENT ) ) ? CBC_I2C_CMD_ACK_FE1 : CBC_I2C_CMD_ACK_FE2 );
			fCbcI2CCmdRq = ( uint32_t( ReadReg( FMC1_PRESENT ) ) ? CBC_I2C_CMD_RQ_FE1 : CBC_I2C_CMD_RQ_FE2 );
			fCbcHardReset = ( uint32_t( ReadReg( FMC1_PRESENT ) ) ? CBC_HARD_RESET_FE1 : CBC_HARD_RESET_FE2 );
			fCbcFastReset = ( uint32_t( ReadReg( FMC1_PRESENT ) ) ? CBC_FAST_RESET_FE1 : CBC_FAST_RESET_FE2 );
		}
		else
		{
			fCbcStubLat  = CBC_STUB_LATENCY;
			fCbcI2CCmdAck =  CBC_I2C_CMD_ACK;
			fCbcI2CCmdRq = CBC_I2C_CMD_RQ;
			fCbcHardReset = CBC_HARD_RESET;
			fCbcFastReset = CBC_FAST_RESET;
		}
	}

	void GlibFWInterface::Start()
	{

#ifdef __CBCDAQ_DEV__
		long mtime = getTimeTook( fStartVeto, 1 );
		std::cout << "Time took for the trigger veto to trigger enable: " << std::dec << mtime << " ms." << std::endl;
#endif

		std::vector< std::pair<std::string, uint32_t> > cVecReg;
		std::pair<std::string, uint32_t> cPairReg;

		//Starting the DAQ

		cPairReg.first = BREAK_TRIGGER;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = PC_CONFIG_OK;
		cPairReg.second = 1;
		cVecReg.push_back( cPairReg );
		cPairReg.first = FORCE_BG0_START;
		cPairReg.second = 1;
		cVecReg.push_back( cPairReg );

		WriteStackReg( cVecReg );

		cVecReg.clear();

	}

	void GlibFWInterface::Stop( uint32_t pNthAcq )
	{

		std::vector< std::pair<std::string, uint32_t> > cVecReg;
		std::pair<std::string, uint32_t> cPairReg;

		uhal::ValWord<uint32_t> cVal;

		//Select SRAM
		SelectDaqSRAM( pNthAcq );

		//Stop the DAQ
		cPairReg.first = BREAK_TRIGGER;
		cPairReg.second = 1;
		cVecReg.push_back( cPairReg );
		cPairReg.first = PC_CONFIG_OK;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = FORCE_BG0_START;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );

		WriteStackReg( cVecReg );
		cVecReg.clear();

		boost::posix_time::milliseconds cWait( 100 );

		//Wait for the selected SRAM to be full then empty it
		do
		{
			cVal = ReadReg( fStrFull );

			if ( cVal == 1 )
				boost::this_thread::sleep( cWait );

		}
		while ( cVal == 1 );

		WriteReg( fStrReadout, 0 );
		fNTotalAcq++;
	}


	void GlibFWInterface::Pause()
	{

		WriteReg( BREAK_TRIGGER, 1 );

#ifdef __CBCDAQ_DEV__
		std::cout << "Pause engaged" << std::endl;
#endif
	}


	void GlibFWInterface::Resume()
	{

		WriteReg( BREAK_TRIGGER, 0 );

#ifdef __CBCDAQ_DEV__
		std::cout << "Pause disengaged" << std::endl;
#endif
	}

	void GlibFWInterface::ReadData( BeBoard* pBoard, unsigned int pNthAcq, bool pBreakTrigger )
	{

#ifdef __CBCDAQ_DEV__
		std::cout << "ReadDataInSRAM" << std::endl;

		struct timeval cStartReadDataInSRAM, cStartBlockRead;
		struct timeval start;
		long mtime;

		gettimeofday( &cStartReadDataInSRAM, 0 );
		gettimeofday( &start, 0 );
#endif

		//Readout settings
		boost::posix_time::milliseconds cWait( 1 );

		uhal::ValWord<uint32_t> cVal;

		// Since the Number of  Packets is a FW register, it should be read from the Settings Table, add 1 to avoid upsetting the EventCounter
		uint32_t cNPackets = pBoard->getReg( "user_wb_ttc_fmc_regs.pc_commands.CBC_DATA_PACKET_NUMBER" ) + 1 ;

		//use a counting visitor to find out the number of CBCs
		struct CbcCounter : public HwDescriptionVisitor
		{
			uint32_t fNCbc = 0;

			void visit( Cbc& pCbc ) {
				fNCbc++;
			}
			uint32_t getNCbc() {
				if ( fNCbc == 2 )
					// since the 2 CBC FW outputs data for 4 CBCs (beamtest heritage, might have to change in the future)
					return 2 * fNCbc;
				else return fNCbc;
			}
		};

		CbcCounter cCounter;
		pBoard->accept( cCounter );
		// compute the block size according to the number of CBC's on this board
		// this will have to change with a more generic FW
		uint32_t cBlockSize = cNPackets * ( cCounter.getNCbc() * CBC_EVENT_SIZE_32 + EVENT_HEADER_TDC_SIZE_32 ); // in 32 bit words

		//Wait for start acknowledge
		do
		{
			cVal = ReadReg( CMD_START_VALID );

			if ( cVal == 0 )
				boost::this_thread::sleep( cWait );

		}
		while ( cVal == 0 );

#ifdef __CBCDAQ_DEV__
		mtime = getTimeTook( start, 1 );
		std::cout << "GlibController::ReadData()  Time took for the CMD_START_VALID flag to be set: " <<                        std::dec << mtime << " ms." << std::endl;
#endif

		//FIFO goes to write_data state
		//Select SRAM
		SelectDaqSRAM( pNthAcq );

#ifdef __CBCDAQ_DEV__
		gettimeofday( &start, 0 );
#endif

		//Wait for the SRAM full condition.
		cVal = ReadReg( fStrFull );

		do
		{
			boost::this_thread::sleep( cWait );

			cVal = ReadReg( fStrFull );

		}
		while ( cVal == 0 );

#ifdef __CBCDAQ_DEV__
		mtime = getTimeTook( start, 1 );
		std::cout << "Time took for the data to be ready : " << std::dec << mtime << " ms." << std::endl;
#endif

		//break trigger
		if ( pBreakTrigger )
			WriteReg( BREAK_TRIGGER, 1 );


#ifdef __CBCDAQ_DEV__
		gettimeofday( &fStartVeto, 0 );
#endif

		//Set read mode to SRAM
		WriteReg( fStrSramUserLogic, 0 );

#ifdef __CBCDAQ_DEV__
		gettimeofday( &cStartBlockRead, 0 );
#endif

		//Read SRAM
		uhal::ValVector<uint32_t> cData = ReadBlockReg( fStrSram, cBlockSize );

		// To avoid the IPBUS bug
		// need to convert uHal::ValVector to vector<uint32_t> so we can replace the 256th word
		std::vector<uint32_t> cDataAlt = cData.value();
		if ( cBlockSize > 255 )
		{
			std::string fSram_256 = fStrSram + "_256";
			uhal::ValWord<uint32_t> cWord = ReadReg( fSram_256 );
			cDataAlt[255] = cWord.value();
		}

#ifdef __CBCDAQ_DEV__
		mtime = getTimeTook( cStartBlockRead, 1 );
		std::cout << "Time took for block read: " << std::dec << mtime << " ms." << std::endl;
#endif

		WriteReg( fStrSramUserLogic, 1 );
		WriteReg( fStrReadout, 1 );

#ifdef __CBCDAQ_DEV__
		gettimeofday( &start, 0 );
#endif

		//Wait for the non SRAM full condition starts,
		do
		{
			cVal = ReadReg( fStrFull );

			if ( cVal == 1 )
				boost::this_thread::sleep( cWait );

		}
		while ( cVal == 1 );

		//Wait for the non SRAM full condition ends.
#ifdef __CBCDAQ_DEV__
		mtime = getTimeTook( start, 0 );
		std::cout << "Time took to the full flag to be 0 : " << std::dec << mtime << " us." << std::endl;
#endif

		WriteReg( fStrReadout, 0 );

#ifdef __CBCDAQ_DEV__
		mtime = getTimeTook( cStartReadDataInSRAM, 1 );
		std::cout << "Time took for ReadDataInSRAM: " << std::dec << mtime << " ms." << std::endl;
#endif
		// just creates a new Data object, setting the pointers and getting the correct sizes happens in Set()
		if ( fData ) delete fData;
		fData = new Data();

		// set the vector<uint32_t> as event buffer and let him know how many packets it contains
		fData->Set( &cDataAlt , cNPackets );

	}

	void GlibFWInterface::SelectDaqSRAM( uint32_t pNthAcq )
	{
		fStrSram  = ( ( pNthAcq % 2 + 1 ) == 1 ? SRAM1 : SRAM2 );
		fStrSramUserLogic = ( ( pNthAcq % 2 + 1 ) == 1 ? SRAM1_USR_LOGIC : SRAM2_USR_LOGIC );
		fStrFull = ( ( pNthAcq % 2 + 1 ) == 1 ? SRAM1_FULL : SRAM2_FULL );
		fStrReadout = ( ( pNthAcq % 2 + 1 ) == 1 ? SRAM1_END_READOUT : SRAM2_END_READOUT );
	}


	const Event* GlibFWInterface::GetNextEvent( BeBoard* pBoard )
	{
		return fData->GetNextEvent( pBoard );
	}


	const char* GlibFWInterface::GetBuffer( uint32_t& pBufSize ) const
	{
		return fData->GetBuffer( pBufSize );
	}


	//Methods for Cbc's:

	void GlibFWInterface::SelectFeSRAM( uint32_t pFe )
	{
		pFe = 0;
		fStrSram = ( pFe ? SRAM2 : SRAM1 );
		fStrOtherSram = ( pFe ? SRAM1 : SRAM2 );
		fStrSramUserLogic = ( pFe ? SRAM2_USR_LOGIC : SRAM1_USR_LOGIC );
		fStrOtherSramUserLogic = ( pFe ? SRAM2_USR_LOGIC : SRAM1_USR_LOGIC );
	}

	bool GlibFWInterface::I2cCmdAckWait( uint32_t pAckVal, uint8_t pNcount )
	{
		unsigned int cWait( 100 );

		if ( pAckVal )
			cWait = pNcount * 500;

		usleep( cWait );

		uhal::ValWord<uint32_t> cVal;
		uint32_t cLoop = 0;

		do
		{
			cVal = ReadReg( CBC_I2C_CMD_ACK );

			if ( cVal != pAckVal )
			{
				//std::cout << "Waiting for the I2c command acknowledge to be " << pAckVal << " for " << uint32_t(pNcount) << " registers." << std::endl;
				usleep( cWait );
			}

		}
		while ( cVal != pAckVal && ++cLoop < MAX_NB_LOOP );

		if ( cLoop >= MAX_NB_LOOP )
		{
			std::cout << "Warning: time out in I2C acknowledge loop (" << pAckVal << ")" << std::endl;
			return false;
		}

		return true;
	}

	void GlibFWInterface::SendBlockCbcI2cRequest( std::vector<uint32_t>& pVecReq, bool pWrite )
	{

		WriteReg( fStrSramUserLogic, 1 );

		pVecReq.push_back( 0xFFFFFFFF );

		WriteReg( fStrSramUserLogic, 0 );

		WriteBlockReg( fStrSram, pVecReq );
		WriteReg( fStrOtherSram, 0xFFFFFFFF );

		WriteReg( fStrSramUserLogic, 1 );

		WriteReg( CBC_HARD_RESET, 0 );

		//r/w request
		WriteReg( CBC_I2C_CMD_RQ, pWrite ? 3 : 1 );

		pVecReq.pop_back();

		if ( I2cCmdAckWait( ( uint32_t )1, pVecReq.size() ) == 0 )
			throw Exception( Form( "%s: I2cCmdAckWait %d failed.", "CbcInterface", 1 ) );

		WriteReg( CBC_I2C_CMD_RQ, 0 );

		if ( I2cCmdAckWait( ( uint32_t )0, pVecReq.size() ) == 0 )
			throw Exception( Form( "%s: I2cCmdAckWait %d failed.", "CbcInterface", 0 ) );

	}

	void GlibFWInterface::ReadI2cBlockValuesInSRAM( std::vector<uint32_t>& pVecReq )
	{

		WriteReg( fStrSramUserLogic, 0 );

		uhal::ValVector<uint32_t> cData = ReadBlockReg( fStrSram, pVecReq.size() );

		WriteReg( fStrSramUserLogic, 1 );
		WriteReg( CBC_I2C_CMD_RQ, 0 );

		std::vector<uint32_t>::iterator it = pVecReq.begin();
		uhal::ValVector< uint32_t >::const_iterator itValue = cData.begin();

		while ( it != pVecReq.end() )
		{
			*it = *itValue;
			it++;
			itValue++;
		}
	}


	void GlibFWInterface::EnableI2c( bool pEnable )
	{
		uint32_t cValue = I2C_CTRL_ENABLE;

		if ( !pEnable )
			cValue = I2C_CTRL_DISABLE;

		WriteReg( I2C_SETTINGS, cValue );

		if ( pEnable )
			usleep( 100000 );
	}

	void GlibFWInterface::WriteCbcBlockReg( uint8_t pFeId, std::vector<uint32_t>& pVecReq )
	{

#ifdef __CBCDAQ_DEV__
		static long min( 0 ), sec( 0 );
		struct timeval start0, end;
		long seconds( 0 ), useconds( 0 );

		if ( DEV_FLAG )
			gettimeofday( &start0, 0 );
#endif

		SelectFeSRAM( pFeId );
		EnableI2c( 1 );

		try
		{
			SendBlockCbcI2cRequest( pVecReq, true );
		}

		catch ( Exception& except )
		{
			throw except;
		}

#ifdef __CBCDAQ_DEV__
		if ( DEV_FLAG )
		{
			gettimeofday( &end, 0 );
			seconds = end.tv_sec - start0.tv_sec;
			useconds = end.tv_usec - start0.tv_usec;
			min += ( seconds + useconds / 1000000 ) / 60;
			sec += ( seconds + useconds / 1000000 ) % 60;
			std::cout << "Time took for Cbc register write so far = " << min << " min " << sec << " sec." << std::endl;

		}
#endif

		EnableI2c( 0 );
	}

	void GlibFWInterface::ReadCbcBlockReg( uint8_t pFeId, std::vector<uint32_t>& pVecReq )
	{

#ifdef __CBCDAQ_DEV__
		static long min( 0 ), sec( 0 );
		struct timeval start0, end;
		long seconds( 0 ), useconds( 0 );

		if ( DEV_FLAG )
			gettimeofday( &start0, 0 );
#endif

		SelectFeSRAM( pFeId );
		EnableI2c( 1 );

		try
		{
			SendBlockCbcI2cRequest( pVecReq, false );
		}

		catch ( Exception& e )
		{
			throw e;
		}

#ifdef __CBCDAQ_DEV__
		if ( DEV_FLAG )
		{
			gettimeofday( &end, 0 );
			seconds = end.tv_sec - start0.tv_sec;
			useconds = end.tv_usec - start0.tv_usec;
			min += ( seconds + useconds / 1000000 ) / 60;
			sec += ( seconds + useconds / 1000000 ) % 60;
			std::cout << "Time took for Cbc register read so far = " << min << " min " << sec << " sec." << std::endl;
		}
#endif

		ReadI2cBlockValuesInSRAM( pVecReq );

		EnableI2c( 0 );

	}
}
