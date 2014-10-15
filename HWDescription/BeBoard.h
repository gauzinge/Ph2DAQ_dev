/*!

        \file                BeBoard.h
        \brief                           BeBoard Description class, configs of the BeBoard
        \author                          Lorenzo BIDEGAIN
        \date                            14/07/14
        \version                         1.0
        Support :                        mail to : lorenzo.bidegain@gmail.com

 */

#ifndef _BeBoard_h__
#define _BeBoard_h__

#include "Definition.h"
#include "Module.h"
#include "../HWInterface/Visitor.h"
#include <vector>
#include <map>
#include <stdint.h>


/*!
 * \namespace Ph2_HwDescription
 * \brief Namespace regrouping all the hardware description
 */
namespace Ph2_HwDescription
{

	typedef std::map< std::string, uint16_t > BeBoardRegMap;     /*!< Map containing the registers of a board */

	/*!
	 * \class BeBoard
	 * \brief Read/Write BeBoard's registers on a file, handles a register map and handles a vector of Module which are connected to the BeBoard
	 */
	class BeBoard
	{

	  public:

		// C'tors: the BeBoard only needs to know about it's shelf and which BE it is
		/*!
		 * \brief Default C'tor
		 */
		BeBoard();

		/*!
		 * \brief Standard C'tor
		 * \param pShelveId
		 * \param pBeId
		 */
		BeBoard( uint8_t pShelveId, uint8_t pBeId );

		/*!
		* \brief C'tor for a standard BeBoard reading a config file
		* \param pShelveId
		* \param pBeId
		* \param filename of the configuration file
		*/
		BeBoard( uint8_t pShelveId, uint8_t pBeId, const std::string& filename );

		/*!
		* \brief Destructor
		*/
		~BeBoard() {}

		// Public Methods

		/*!
		 * \brief acceptor method for HwDescriptionVisitor
		 * \param pVisitor
		 */
		void accept( HwDescriptionVisitor& pVisitor ) {
			pVisitor.visit( *this );
			for ( auto& cFe : fModuleVector )
				cFe.accept( pVisitor );
		}
		// void accept( HwDescriptionVisitor& pVisitor ) const {
		//  pVisitor.visit( *this );
		//  for ( auto& cFe : fModuleVector )
		//      cFe.accept( pVisitor );
		// }

		/*!
		* \brief Get the number of modules connected to the BeBoard
		* \return The size of the vector
		*/
		uint8_t getNFe() const {
			return fModuleVector.size();
		}

		/*!
		* \brief Get any register from the Map
		* \param pReg
		* \return The value of the register
		*/
		uint16_t getReg( const std::string& pReg );
		/*!
		* \brief Set any register of the Map, if the register is not on the map, it adds it.
		* \param pReg
		* \param psetValue
		*/
		void setReg( const std::string& pReg, uint16_t psetValue );

		/*!
		 * \brief Adding a module to the vector
		 * \param pModule
		 */
		void addModule( Module& pModule );
		/*!
		 * \brief Remove a Module from the vector
		 * \param pModuleId
		 * \return a bool which indicate if the removing was successful
		 */
		bool removeModule( uint8_t pModuleId );
		/*!
		 * \brief Get a module from the vector
		 * \param pModuleId
		 * \return a pointer of module, so we can manipulate directly the module contained in the vector
		 */
		Module* getModule( uint8_t pModuleId );

		/*!
		* \brief Get the Map of the registers
		* \return The map of register
		*/
		BeBoardRegMap getBeBoardRegMap() const {
			return fRegMap;
		}

		/*!
		* \brief Get the BeBoardId of the BeBoard
		* \return the BeBoard Id
		*/
		uint8_t getBeId() const {
			return fBeId;
		}
		/*!
		* \brief Get the Shelve Id of the BeBoard
		* \return the ShelveId
		*/
		uint8_t getShelveId() const {
			return fShelveId;
		}
		/*!
		* \brief Set the Be Id of the BeBoard
		* \param pBeId
		*/
		void setBeId( uint8_t pBeId ) {
			fBeId = pBeId;
		};
		/*!
		* \brief Set the Shelve Id of the BeBoard
		* \param pShelveId
		*/
		void setShelveId( uint8_t pShelveId ) {
			fShelveId = pShelveId;
		};

		// Vector of FEModules, each module is supposed to know which FMC slot it is connected to...
		std::vector< Module > fModuleVector;

	  protected:
		//Connection Members
		uint8_t fShelveId;
		uint8_t fBeId;


		BeBoardRegMap fRegMap;             /*!< Map of BeBoard Register Names vs. Register Values */

	  private:

		/*!
		* \brief Load RegMap from a file
		* \param filename
		*/
		void loadConfigFile( const std::string& filename );
	};
}

#endif
