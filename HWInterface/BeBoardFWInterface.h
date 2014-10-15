/*!

        \file                BeBoardFWInterface.h
        \brief                           BeBoardFWInterface base class of all type of boards
        \author                          Lorenzo BIDEGAIN, Nicolas PIERRE
        \version             1.0
        \date                            28/07/14
        Support :                        mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com

 */

#ifndef __BEBOARDFWINTERFACE_H__
#define __BEBOARDFWINTERFACE_H__

#include <boost/format.hpp>
#include <uhal/uhal.hpp>
#include "RegManager.h"
#include "Event.h"
#include "Data.h"
#include "Utilities.h"
#include "Exception.h"
#include "../HWDescription/BeBoard.h"
#include "../HWDescription/Definition.h"
#include "../HWDescription/CbcRegItem.h"
#include "../HWDescription/Cbc.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"

using namespace Ph2_HwDescription;

/*!
 * \namespace Ph2_HwInterface
 * \brief Namespace regrouping all the interfaces to the hardware
 */
namespace Ph2_HwInterface
{
	/*!
	 * \class BeBoardFWInterface
	 * \brief Class separating board system FW interface from uHal wrapper
	 */
	class BeBoardFWInterface : public RegManager
	{

	  public:
		unsigned int fNTotalAcq;

		Data* fData; /*!< Data read storage*/

	  public:

		/*!
		* \brief Constructor of the BeBoardFWInterface class
		* \param puHalConfigFileName : path of the uHal Config File
		*/
		BeBoardFWInterface( const char* puHalConfigFileName, uint32_t pBoardId );
		/*!
		* \brief Destructor of the BeBoardFWInterface class
		*/
		virtual ~BeBoardFWInterface();
		/*!
		* \brief Get the board type
		*/
		virtual std::string getBoardType();
		/*!
		* \brief Get the board infos
		*/
		virtual void getBoardInfo();

		//These two methods will be implemented soon
		virtual void FlashProm() {}
		virtual void ProgramCdce() {}

		//Encode/Decode Cbc values
		/*!
		* \brief Encode a/several word(s) readable for a Cbc
		* \param pRegItem : RegItem containing infos (name, adress, value...) about the register to write
		* \param pCbcId : Id of the Cbc to work with
		* \param pVecReq : Vector to stack the encoded words
		*/
		virtual void EncodeReg( CbcRegItem& pRegItem, uint8_t pCbcId, std::vector<uint32_t>& pVecReq ); /*!< Encode a/several word(s) readable for a Cbc*/
		/*!
		* \brief Decode a word from a read of a register of the Cbc
		* \param pRegItem : RegItem containing infos (name, adress, value...) about the register to read
		* \param pCbcId : Id of the Cbc to work with
		* \param pWord : variable to put the decoded word
		*/
		virtual void DecodeReg( CbcRegItem& pRegItem, uint8_t pCbcId, uint32_t pWord ); /*!< Decode a word from a read of a register of the Cbc*/


		//virtual pure methods which are defined in the proper BoardFWInterface class
		//r/w the Cbc registers
		/*!
		* \brief Write register blocks of a Cbc
		* \param pFeId : FrontEnd to work with
		* \param pVecReq : Block of words to write
		*/
		virtual void WriteCbcBlockReg( uint8_t pFeId, std::vector<uint32_t>& pVecReq ) = 0;
		/*!
		* \brief Read register blocks of a Cbc
		* \param pFeId : FrontEnd to work with
		* \param pVecReq : Vector to stack the read words
		*/
		virtual void ReadCbcBlockReg( uint8_t pFeId, std::vector<uint32_t>& pVecReq ) = 0;
		/*!
		* \brief Configure the board with its Config File
		* \param pBoard
		*/
		virtual void ConfigureBoard( BeBoard* pBoard ) = 0;
		/*!
		 * \brief Start a DAQ
		 */
		virtual void Start() = 0;
		/*!
		 * \brief Stop a DAQ
		 * \param pNthAcq : actual number of acquisitions
		 */
		virtual void Stop( uint32_t pNthAcq ) = 0;
		/*!
		 * \brief Pause a DAQ
		 */
		virtual void Pause() = 0;
		/*!
		 * \brief Resume a DAQ
		 */
		virtual void Resume() = 0;
		/*!
		 * \brief Read data from DAQ
		 * \param pBoard
		 * \param pNthAcq : actual number of acquisitions
		 * \param pBreakTrigger : if true, enable the break trigger
		 */
		virtual void ReadData( BeBoard* pBoard, uint32_t pNthAcq, bool pBreakTrigger ) = 0;
		/*!
		 * \brief Get next event from data buffer
		 * \return Next event
		 */
		virtual const Event* GetNextEvent( BeBoard* pBoard ) = 0;
		/*!
		 * \brief Get the data buffer
		 * \param pBufSize : recovers the data buffer size
		 * \return Data buffer
		 */
		virtual const char* GetBuffer( uint32_t& pBufSize ) const = 0;
	};
}

#endif
