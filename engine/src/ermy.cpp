#include "ermy.h"
#include <iostream>

bool ermy::InitializeEngine()
{
    std::cout << "InitializeEngine TEST EMSCRIPTEN" << std::endl;
    return true;
}

bool ermy::ShutdownEngine()
{
    std::cout << "ShutdownEngine TEST EMSCRIPTEN" << std::endl;
    return true;
}