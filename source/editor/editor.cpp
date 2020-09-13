#include <iostream>
#include "ermy.h"

int main()
{
	ermy::Initialize();
	ermy::Free();

	std::cin.get();
	return 0;
}
