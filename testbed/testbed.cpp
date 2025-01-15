#include <iostream>
#include "ermy.h"

#include "ermy_entry_point.h"

int main()
{
    std::cout << "testbed entry point" << std::endl;
    
    ermy::InitializeEngine();
    ermy::ShutdownEngine();

    return 0;
}