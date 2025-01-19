#include "../ermy_os_entry_point.h"
#include "emscripten_utils.h"
#include "../../logger.h"
#include "../ermy_os_utils.h"
#include "application.h"

#include <iostream>
#include <thread>
#include <emscripten.h>
#include <emscripten/html5.h>
#include "emscripten/html5_webgpu.h"

int main()
{
    printf("hello emscripten\n");
    return 0;
}