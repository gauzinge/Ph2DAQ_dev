#ifndef COMMONVISITORS_H__
#define COMMONVISITORS_H__


#include "../HWInterface/BeBoardFWInterface.h"
#include "../HWDescription/Shelve.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
#include "../System/ConsoleColor.h"
#include "../HWInterface/Visitor.h"

#include <iostream>
#include <vector>
#include <stdlib.h>
# include <string>



// wriite single reg

using namespace Ph2_HwInterface;
using namespace Ph2_HwDescription;

struct CbcRegWriter : public HwDescriptionVisitor
{
	CbcInterface* fInterface;
	std::string fRegName;
	uint8_t fRegValue;

	CbcRegWriter( CbcInterface* pInterface, std::string pRegName, uint8_t pRegValue ): fInterface( pInterface ), fRegName( pRegName ), fRegValue( pRegValue ) {}

	void visit( Ph2_HwDescription::Cbc& pCbc ) {
		fInterface->WriteCbcReg( &pCbc, fRegName, fRegValue );
	}
};

//write multi reg
struct CbcMultiRegWriter : public HwDescriptionVisitor
{
	CbcInterface* fInterface;
	std::vector<std::pair<std::string, uint8_t>> fRegVec;

	CbcMultiRegWriter( CbcInterface* pInterface, std::vector<std::pair<std::string, uint8_t>> pRegVec ): fInterface( pInterface ), fRegVec( pRegVec ) {}

	void visit( Ph2_HwDescription::Cbc& pCbc ) {
		fInterface->WriteCbcMultReg( &pCbc, fRegVec );
	}
};

// HwDescription Objects Counter
class Counter : public HwDescriptionVisitor
{
  private:
	uint32_t fNCbc = 0;
	uint32_t fNFe = 0;
	uint32_t fNBe = 0;
	uint32_t fNShelve = 0;
  public:
	void visit( Ph2_HwDescription::Cbc& pCbc ) {
		fNCbc++;
	}
	void visit( Ph2_HwDescription::Module& pModule ) {
		fNFe++;
	}
	void visit( Ph2_HwDescription::BeBoard& pBoard ) {
		fNBe++;
	}
	void visit( Ph2_HwDescription::Shelve& pShelve ) {
		fNShelve++;
	}
	uint32_t getNCbc() {
		return fNCbc;
	}
	uint32_t getNFe() {
		return fNFe;
	}
	uint32_t getNBe() {
		return fNBe;
	}
	uint32_t getNShelves() {
		return fNShelve;
	};
};

// Configurator
class Configurator: public HwDescriptionVisitor
{
  private:
	BeBoardInterface* fBeBoardInterface;
	CbcInterface* fCbcInterface;
  public:
	Configurator( BeBoardInterface* pBeBoardInterface, CbcInterface* pCbcInterface ): fBeBoardInterface( pBeBoardInterface ), fCbcInterface( pCbcInterface ) {}
	void visit( BeBoard& pBoard ) {
		fBeBoardInterface->ConfigureBoard( &pBoard );
		std::cout << "Successfully configured Board " << int( pBoard.getBeId() ) << std::endl;
	}
	void visit( Cbc& pCbc ) {
		fCbcInterface->ConfigureCbc( &pCbc );
		std::cout << "Successfully configured Cbc " << int( pCbc.getCbcId() ) << std::endl;

	}
};

// read a single CBC register from fRegMap, from the physical CBC
struct CbcRegReader : public HwDescriptionVisitor
{
	std::string fRegName;
	uint8_t fRegValue;
	uint8_t fReadRegValue;
	CbcInterface* fInterface;

	CbcRegReader( CbcInterface* pInterface, std::string pRegName ): fInterface( pInterface ), fRegName( pRegName ) {}
	void visit( Cbc& pCbc ) {
		fRegValue = pCbc.getReg( fRegName );
		fInterface->ReadCbcReg( &pCbc, fRegName );
		fReadRegValue = pCbc.getReg( fRegName );

		std::cout << "Reading Reg " << RED << fRegName << RESET << " on CBC " << ( int )pCbc.getCbcId() << " memory value: " << ( int )fRegValue << " read value: " << ( int )fReadRegValue << std::endl;
	}
};


#endif
