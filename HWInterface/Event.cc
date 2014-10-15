/*

        FileName :                     Event.cc
        Content :                      Event handling from DAQ
        Programmer :                   Nicolas PIERRE
        Version :                      1.0
        Date of creation :             10/07/14
        Support :                      mail to : nicolas.pierre@icloud.com

 */

#include "Event.h"

using namespace Ph2_HwDescription;


uint32_t swap_bytes( char* pOrg )
{
	uint32_t ui32 = pOrg[0] | ( pOrg[1] << 8 ) | ( pOrg[2] << 16 ) | ( pOrg[3] << 24 );
	return ( ( ui32 >> 24 ) & 0xFF ) |
		   ( ( ui32 >>  8 ) & 0x0000FF00 ) |
		   ( ( ui32 <<  8 ) & 0x00FF0000 ) |
		   ( ( ui32 << 24 ) & 0xFF000000 );
}

namespace Ph2_HwInterface
{

	//Event implementation
	Event::Event( uint32_t pNbCbc, char* pEventBuf )
	{
		SetSize( pNbCbc );
		SetEvent( pEventBuf );
	}


	Event::Event( BeBoard* pBoard, uint32_t pNbCbc, char* pEventBuf )
	{
		SetSize( pNbCbc );
		AddBoard( pBoard );
		SetEvent( pEventBuf );
	}

	Event::Event( Event& pEvent ) :
		fBuf( NULL ),
		fBunch( pEvent.fBunch ),
		fOrbit( pEvent.fOrbit ),
		fLumi( pEvent.fLumi ),
		fEventCount( pEvent.fEventCount ),
		fEventCountCBC( pEvent.fEventCountCBC ),
		fTDC( pEvent.fTDC ),
		fEventMap( pEvent.fEventMap )
	{

	}

	void Event::SetSize( uint32_t pNbCbc )
	{
		//  need to introduce a factor of 2 because the 2CBC FW is written for 4 CBCs actually
		int cFactor = ( pNbCbc == 2 ) ? 2 : 1;

		fEventSize = pNbCbc * cFactor * CBC_EVENT_SIZE_CHAR  + EVENT_HEADER_TDC_SIZE_CHAR;
		fOffsetTDC = EVENT_HEADER_SIZE_32 * cFactor + CBC_EVENT_SIZE_32 * pNbCbc * 2; //in 32 bit words

#ifdef __CBCDAQ_DEV__

		std::cout << "DEBUG EVENT SET SIZE: Event size(char) " << fEventSize << " nCBC =  " << cFactor* pNbCbc <<  "  this should be 168 with 4cbc " << "  and Offset TDC " << fOffsetTDC << std::endl;
#endif
	}

	void Event::AddBoard( BeBoard* pBoard )
	{
		uint32_t cNFe = uint32_t( pBoard->getNFe() );

		for ( uint32_t i = 0; i < cNFe; i++ )
		{
			uint32_t cNCbc = uint32_t( pBoard->getModule( i )->getNCbc() );
			FeEventMap cFeEventMap;

			for ( uint32_t j = 0; j < cNCbc; j++ )
				cFeEventMap[j] = 0;

			fEventMap[i] = cFeEventMap;
		}

	}


	int Event::SetEvent( char* pEvent )
	{
		int vsize = sizeof( uint32_t );

		fBuf = pEvent;

		fBunch = 0x00FFFFFF & swap_bytes( &pEvent[0 * vsize] );

		fOrbit = 0x00FFFFFF & swap_bytes( &pEvent[1 * vsize] );

		fLumi = 0x00FFFFFF & swap_bytes( &pEvent[2 * vsize] );

		fEventCount = 0x00FFFFFF & swap_bytes( &pEvent[3 * vsize] );

		fEventCountCBC = 0x00FFFFFF & swap_bytes( &pEvent[4 * vsize] );

		fTDC = 0x000000FF & swap_bytes( &pEvent[fOffsetTDC * vsize] );


		for ( EventMap::iterator cFeIt = fEventMap.begin(); cFeIt != fEventMap.end(); cFeIt++ )
		{
			uint8_t cFeId = uint8_t( cFeIt->first );
			uint8_t cNCbc = uint8_t( cFeIt->second.size() );

			for ( FeEventMap::iterator cCbcIt = cFeIt->second.begin(); cCbcIt != cFeIt->second.end(); cCbcIt++ )
			{
				uint8_t cCbcId = uint8_t( cCbcIt->first );
				// cCbcIt->second = &pEvent[OFFSET_FE_EVENT + FeId * fFeNChar + CbcId * CBC_NCHAR];
				cCbcIt->second = &pEvent[EVENT_HEADER_SIZE_CHAR + cFeId * CBC_EVENT_SIZE_CHAR * cNCbc + cCbcId * CBC_EVENT_SIZE_CHAR];

#ifdef __CBCDAQ_DEV__
				std::cout << "DEBUG FE " << int( cFeId ) << "  with " << int( cNCbc ) << " cbcs on CBC " << int( cCbcId ) << " and the offset in Chars is "  << EVENT_HEADER_SIZE_CHAR + cFeId* CBC_EVENT_SIZE_CHAR* cNCbc + cCbcId* CBC_EVENT_SIZE_CHAR << std::endl;
#endif

			}
		}

		return 0;
	}


	char* Event::GetCbcEvent( uint8_t& pFeId, uint8_t& pCbcId ) const
	{
		EventMap::const_iterator cIt = fEventMap.find( pFeId );

		if ( cIt == fEventMap.end() )
		{
			std::cout << "Event: FE " << pFeId << " is not found." << std::endl;
			return NULL;
		}

		FeEventMap::const_iterator cJt = cIt->second.find( pCbcId );

		if ( cJt == cIt->second.end() )
		{
			std::cout << "Event: CBC " << pCbcId << " is not found." << std::endl;
			return NULL;
		}

		return cJt->second;
	}


	std::string Event::HexString() const
	{
		std::stringbuf tmp;
		std::ostream os( &tmp );

		os << std::hex;

		for ( uint32_t i = 0; i < fEventSize; i++ )
			os << std::uppercase << std::setw( 2 ) << std::setfill( '0' ) << ( fBuf[i] & 0xFF );

		os << std::endl;

		return tmp.str();
	}


	bool Event::Bit( uint8_t pFeId, uint8_t pCbcId, uint32_t pPosition ) const
	{
		uint32_t cByteP = pPosition / 8;
		uint32_t cBitP = pPosition % 8;

		return GetCbcEvent( pFeId, pCbcId )[cByteP] & ( 1 << ( 7 - cBitP ) );
	}


	bool Event::Error( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const
	{
		return Bit( pFeId, pCbcId, i + OFFSET_ERROR );
	}

	uint32_t Event::Error( uint8_t pFeId, uint8_t pCbcId ) const
	{

		uint32_t val( 0 );
		uint32_t cWidth = WIDTH_ERROR;

		for ( uint32_t i = 0; i < cWidth; i++ )
		{
			val = val << 1;
			val |= Error( pFeId, pCbcId, i );
		}

		return val;
	}


	uint32_t Event::PipelineAddress( uint8_t pFeId, uint8_t pCbcId ) const
	{
		uint32_t cOffset = OFFSET_PIPELINE_ADDRESS;
		uint32_t cWidth = WIDTH_PIPELINE_ADDRESS;
		uint32_t val( 0 );

		for ( uint32_t i = 0; i < cWidth; i++ )
		{
			val = val << 1;
			val |= Bit( pFeId, pCbcId, cOffset + i );
		}

		return val;
	}

	bool Event::DataBit( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const
	{
		if ( i > NSENSOR )
			return 0;

		return Bit( pFeId, pCbcId, i + OFFSET_CBCDATA );
	}


	std::string Event::BitString( uint8_t pFeId, uint8_t pCbcId, uint32_t pOffset, uint32_t pWidth ) const
	{
		std::stringbuf tmp;
		std::ostream os( &tmp );

		for ( uint32_t i = 0; i < pWidth; i++ )
			os << Bit( pFeId, pCbcId, i + pOffset );

		return tmp.str();
	}


	std::vector<bool> Event::BitVector( uint8_t pFeId, uint8_t pCbcId, uint32_t pOffset, uint32_t pWidth ) const
	{
		std::vector<bool> tmp;

		for ( uint32_t i = 0; i < pWidth; i++ )
			tmp.push_back( Bit( pFeId, pCbcId, i + pOffset ) );

		return tmp;
	}


	std::string Event::DataBitString( uint8_t pFeId, uint8_t pCbcId ) const
	{
		return BitString( pFeId, pCbcId, OFFSET_CBCDATA, WIDTH_CBCDATA );
	}


	std::vector<bool> Event::DataBitVector( uint8_t pFeId, uint8_t pCbcId ) const
	{
		return BitVector( pFeId, pCbcId, OFFSET_CBCDATA, WIDTH_CBCDATA );
	}


	std::string Event::DataHexString( uint8_t pFeId, uint8_t pCbcId ) const
	{
		std::stringbuf tmp;
		std::ostream os( &tmp );

		os << std::hex;

		uint32_t cFirstByteP = OFFSET_CBCDATA / 8;
		uint32_t cFirstBitP = OFFSET_CBCDATA % 8;
		uint32_t cLastByteP = ( cFirstByteP + WIDTH_CBCDATA - 1 ) / 8;
		uint32_t cLastBitP = ( cFirstByteP + WIDTH_CBCDATA - 1 ) % 8;

		uint32_t cMask( 0 );
		uint32_t cMaskLastBit( 0 );
		uint32_t cMaskWidth( 0 );

		//First byte
		cMaskLastBit = cFirstByteP < cLastByteP ? 7 : cLastBitP;
		cMaskWidth = cMaskLastBit - cFirstBitP + 1;
		cMask = ( 1 << ( 7 - cMaskLastBit ) );

		for ( uint32_t i = 0; i < cMaskWidth; i++ )
		{
			cMask = cMask << 1;
			cMask |= 1;
		}

		os << std::uppercase << std::setw( 2 ) << std::setfill( '0' ) << ( GetCbcEvent( pFeId, pCbcId )[cFirstByteP]&cMask );

		if ( cFirstByteP == cLastByteP )
			return tmp.str();

		//Second to the second last byte
		if ( cFirstByteP != cLastByteP - 1 )
		{
			for ( uint32_t j = cFirstByteP + 1; j < cLastByteP; j++ ) os << std::uppercase << std::setw( 2 ) << std::setfill( '0' ) << ( GetCbcEvent( pFeId, pCbcId )[j] & 0xFF );
		}

		//Last byte
		cMaskLastBit = cLastBitP;
		cMaskWidth = cMaskLastBit + 1;
		cMask = ( 1 << ( 7 - cMaskLastBit ) );

		for ( uint32_t i = 0; i < cMaskWidth; i++ )
		{
			cMask = cMask << 1;
			cMask |= 1;
		}

		os << std::uppercase << std::setw( 2 ) << std::setfill( '0' ) << ( GetCbcEvent( pFeId, pCbcId )[cFirstByteP]&cMask );

		return tmp.str();
	}


	std::string Event::GlibFlagString( uint8_t pFeId, uint8_t pCbcId ) const
	{
		return BitString( pFeId, pCbcId, OFFSET_GLIBFLAG, WIDTH_GLIBFLAG );
	}


	std::string Event::StubBitString( uint8_t pFeId, uint8_t pCbcId ) const
	{
		return BitString( pFeId, pCbcId, OFFSET_CBCSTUBDATA, WIDTH_CBCSTUBDATA );
	}

	std::ostream& operator<<( std::ostream& os, const Event& ev )
	{
		os << "Bunch Counter: " << ev.GetBunch() << std::endl;
		os << "Orbit Counter: " << ev.GetOrbit() << std::endl;
		os << " Lumi Section: " << ev.GetLumi() << std::endl;
		os << "  L1A Counter: " << ev.GetEventCount() << std::endl;
		os << "  CBC Counter: " << ev.GetEventCountCBC() << std::endl;
		os << "  TDC Counter: " << ev.GetTDC() << std::endl;

		os << "CBC Data:" << std::endl;
		const EventMap& evmap = ev.GetEventMap();
		const int FIRST_LINE_WIDTH = 22;
		const int LINE_WIDTH = 32;
		const int LAST_LINE_WIDTH = 8;
		for ( EventMap::const_iterator it = evmap.begin(); it != evmap.end(); ++it )
		{
			uint32_t feId = it->first;
			for ( FeEventMap::const_iterator jt = it->second.begin(); jt != it->second.end(); ++jt )
			{
				uint32_t cbcId = jt->first;
				std::string data( ev.DataBitString( feId, cbcId ) );
				os << "FEId = " << feId << " CBCId = " << cbcId << " len(data) = " << data.size() << std::endl;
				os << std::setw( 32 ) << data.substr( 0, FIRST_LINE_WIDTH ) << std::endl;
				for ( int i = 0; i < 7; ++i )
					os << data.substr( FIRST_LINE_WIDTH + LINE_WIDTH * i, LINE_WIDTH ) << std::endl;
				os << data.substr( FIRST_LINE_WIDTH + LINE_WIDTH * 7, LAST_LINE_WIDTH ) << std::endl;
			}
			os << std::endl;
		}
		return os;
	}
}
