#include "ermy.h"
#include <iostream>

bool ermy::InitializeEngine()
{
    std::cout << "InitializeEngine" << std::endl;
    return true;
}

bool ermy::ShutdownEngine()
{
    std::cout << "ShutdownEngine" << std::endl;
    return true;
}