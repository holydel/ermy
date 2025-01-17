#include "ermy.h"
#include <iostream>

bool ermy::InitializeEngine()
{
    std::cout << "InitializeEngine TEST MSVC" << std::endl;
    return true;
}

bool ermy::ShutdownEngine()
{
    std::cout << "ShutdownEngine TEST MSVC EDITED" << std::endl;
    return true;
}