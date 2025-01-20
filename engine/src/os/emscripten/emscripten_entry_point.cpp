#include "../ermy_os_entry_point.h"
#include "emscripten_utils.h"
#include "../../logger.h"
#include "../ermy_os_utils.h"
#include "application.h"

EM_BOOL one_iter(double time, void* userData) {
    // Can render to the screen here, etc.
    //if(!gApplication->Update())
    //{
    //    puts("exit");
    //}
    //engine::update();
    // Return true to keep the loop running.
    float ftime = time;
   // printf("one frame. time: %f\n",ftime);
    ErmyApplicationStep();
    
    return EM_TRUE;
}

float emMouseX = 0;
float emMouseY = 0;
bool emMouseDown[5] = {false};

bool MouseEvent(int eventType, const EmscriptenMouseEvent *mouseEvent __attribute__((nonnull)), void *userData)
{
    if(EMSCRIPTEN_EVENT_CLICK == eventType)
    {
    }

    if(EMSCRIPTEN_EVENT_MOUSEDOWN == eventType)
    {
        emMouseDown[mouseEvent->button] = true;
    }

    if(EMSCRIPTEN_EVENT_MOUSEUP == eventType)
    {
        emMouseDown[mouseEvent->button] = false;
    }

    if(EMSCRIPTEN_EVENT_MOUSEMOVE == eventType)
    {
        printf("canvasX: %d\n",mouseEvent->canvasX);
        printf("canvasY: %d\n",mouseEvent->canvasY);

       printf("targetX: %d\n",mouseEvent->targetX);
        printf("targetY: %d\n",mouseEvent->targetY);

        printf("clientX: %d\n",mouseEvent->clientX);
        printf("clientY: %d\n",mouseEvent->clientY);

        emMouseX = mouseEvent->targetX;
        emMouseY = mouseEvent->targetY;
    }

    return true;
}

int main()
{
    printf("hello emscripten\n");

    emscripten_set_mousemove_callback("canvas", nullptr, true, MouseEvent);
    emscripten_set_mousedown_callback("canvas", nullptr, true, MouseEvent);
    emscripten_set_mouseup_callback("canvas", nullptr, true, MouseEvent);

    emscripten_request_animation_frame_loop(one_iter, 0);

    ErmyApplicationStart();

    return 0;
}