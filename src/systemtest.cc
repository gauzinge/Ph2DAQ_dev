#include "../System/SystemController.h"

using namespace Ph2_System;

int main()
{
	SystemController cSystemController;
	cSystemController.InitializeHw( XML_DESCRIPTION_FILE_2CBC );
	std::cout << uint32_t( cSystemController.fShelveVector[0]->getBoard( 0 )->getModule( 0 )->getCbc( 0 )->getCbcId() ) << std::endl;
	cSystemController.ConfigureHw();
	std::cout << "*** End of the System test ***" << std::endl;
	return 0;
}
