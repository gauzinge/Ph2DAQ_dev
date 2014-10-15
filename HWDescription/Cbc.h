/*!

        \file                   Cbc.h
        \brief                  Cbc Description class, config of the Cbcs
        \author                 Lorenzo BIDEGAIN
        \version                1.0
        \date                   25/06/14
        Support :               mail to : lorenzo.bidegain@gmail.com

 */


#ifndef Cbc_h__
#define Cbc_h__

#include "FrontEndDescription.h"
#include "CbcRegItem.h"
#include "../HWInterface/Visitor.h"
#include <iostream>
#include <map>
#include <string>
#include <stdint.h>
#include <utility>
#include <set>

// Cbc2 Chip HW Description Class


/*!
 * \namespace Ph2_HwDescription
 * \brief Namespace regrouping all the hardware description
 */
namespace Ph2_HwDescription
{

	typedef std::map < std::string, CbcRegItem > CbcRegMap;
	typedef std::pair <std::string, CbcRegItem> CbcRegPair;

	/*!
	 * \class Cbc
	 * \brief Read/Write Cbc's registers on a file, contains a register map
	 */
	class Cbc : public FrontEndDescription
	{

	  public:

		// C'tors with object FE Description
		Cbc( FrontEndDescription& pFeDesc, uint8_t pCbcId, const std::string& filename );

		// C'tors which take ShelveID, BeId, FeID, CbcId
		Cbc( uint8_t pShelveId, uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pCbcId, const std::string& filename );

		// Default C'tor
		Cbc();

		// Copy C'tor
		Cbc( const Cbc& cbcobj );

		// D'Tor
		~Cbc();

		/*!
		 * \brief acceptor method for HwDescriptionVisitor
		 * \param pVisitor
		 */
		void accept( HwDescriptionVisitor& pVisitor ) {
			pVisitor.visit( *this );
		}
		// void accept( HwDescriptionVisitor& pVisitor ) const {
		//  pVisitor.visit( *this );
		// }
		/*!
		* \brief Load RegMap from a file
		* \param filename
		*/
		void loadfRegMap( const std::string& filename );

		/*!
		* \brief Get any register from the Map
		* \param pReg
		* \return The value of the register
		*/
		uint8_t getReg( const std::string& pReg );
		/*!
		* \brief Set any register of the Map
		* \param pReg
		* \param psetValue
		*/
		void setReg( const std::string& pReg, uint8_t psetValue );

		/*!
		* \brief Write the registers of the Map in a file
		* \param filename
		*/
		void saveRegMap( const std::string& filename );

		/*!
		* \brief Get the Map of the registers
		* \return The map of register
		*/
		CbcRegMap getRegMap() const {
			return fRegMap;
		};

		/*!
		* \brief Get the Cbc Id
		* \return The Cbc ID
		*/
		uint8_t getCbcId() const {
			return fCbcId;
		};
		/*!
		 * \brief Set the Cbc Id
		 * \param pCbcId
		 */
		void setCbcId( uint8_t pCbcId ) {
			fCbcId = pCbcId;
		};


	  protected:

		// Map of Register Name vs. RegisterItem that contains: Page, Address, Default Value, Value
		CbcRegMap fRegMap;

		uint8_t fCbcId;

	};


	/*!
	 * \struct CbcComparer
	 * \brief Compare two Cbc by their ID
	 */
	struct CbcComparer
	{

		bool operator()( Cbc& cbc1, Cbc& cbc2 );

	};

	/*!
	 * \struct RegItemComparer
	 * \brief Compare two pair of Register Name Versus CbcRegItem by the Page and Adress of the CbcRegItem
	 */
	struct RegItemComparer
	{

		bool operator()( CbcRegPair pRegItem1, CbcRegPair pRegItem2 );

	};

}

#endif
