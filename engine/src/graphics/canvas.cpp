#include "canvas.h"

float canvas::BgColor[4] = {1.0f,1.0f,1.0f,1.0f};

void ermy::canvas::SetClearColor(float r, float g, float b, float a)
{
    ::canvas::BgColor[0] = r;
    ::canvas::BgColor[1] = g;
    ::canvas::BgColor[2] = b;
    ::canvas::BgColor[3] = a;
}