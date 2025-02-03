#pragma once
#include <ermy_canvas.h>
#include <ermy_commandlist.h>

namespace canvas_interface
{
    extern float BgColor[4];

    void Initialize();
    void Shutdown();

    void SetCommandList(ermy::rendering::CommandList* cl);
}