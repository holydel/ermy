#include "../os_entry_point.h"
#include "android_utils.h"
#include "../../logger.h"
#include <thread>
#include <chrono>
#include "../../application.h"

extern ANativeWindow* gMainWindow;

extern int32_t ImGui_ImplAndroid_HandleInputEvent(const AInputEvent* input_event);
static AInputQueue* gMainInputQueue = nullptr;

std::thread* mainThread = nullptr;

using namespace std::chrono_literals;
using namespace ermy;

/**
	 * NativeActivity has started.  See Java documentation for Activity.onStart()
	 * for more information.
	 */

int androidCursorX = 0;
int androidCursorY = 0;

int32_t HandleInputEvent(const AInputEvent* input_event)
{
    int32_t event_type = AInputEvent_getType(input_event);
    switch (event_type)
    {
    case AINPUT_EVENT_TYPE_KEY:
    {
        int32_t event_key_code = AKeyEvent_getKeyCode(input_event);
        int32_t event_scan_code = AKeyEvent_getScanCode(input_event);
        int32_t event_action = AKeyEvent_getAction(input_event);
        int32_t event_meta_state = AKeyEvent_getMetaState(input_event);

        switch (event_action)
        {
            // FIXME: AKEY_EVENT_ACTION_DOWN and AKEY_EVENT_ACTION_UP occur at once as soon as a touch pointer
            // goes up from a key. We use a simple key event queue/ and process one event per key per frame in
            // ImGui_ImplAndroid_NewFrame()...or consider using IO queue, if suitable: https://github.com/ocornut/imgui/issues/2787
        case AKEY_EVENT_ACTION_DOWN:
        case AKEY_EVENT_ACTION_UP:
        {
            //TODO: add key events
            break;
        }
        default:
            break;
        }
        break;
    }
    case AINPUT_EVENT_TYPE_MOTION:
    {
        int32_t event_action = AMotionEvent_getAction(input_event);
        int32_t event_pointer_index = (event_action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        event_action &= AMOTION_EVENT_ACTION_MASK;

        switch (AMotionEvent_getToolType(input_event, event_pointer_index))
        {
        case AMOTION_EVENT_TOOL_TYPE_MOUSE:

            break;
        case AMOTION_EVENT_TOOL_TYPE_STYLUS:
        case AMOTION_EVENT_TOOL_TYPE_ERASER:

            break;
        case AMOTION_EVENT_TOOL_TYPE_FINGER:
        default:

            break;
        }

        switch (event_action)
        {
        case AMOTION_EVENT_ACTION_DOWN:
        case AMOTION_EVENT_ACTION_UP:
        {
            // Physical mouse buttons (and probably other physical devices) also invoke the actions AMOTION_EVENT_ACTION_DOWN/_UP,
            // but we have to process them separately to identify the actual button pressed. This is done below via
            // AMOTION_EVENT_ACTION_BUTTON_PRESS/_RELEASE. Here, we only process "FINGER" input (and "UNKNOWN", as a fallback).
            int tool_type = AMotionEvent_getToolType(input_event, event_pointer_index);
            if (tool_type == AMOTION_EVENT_TOOL_TYPE_FINGER || tool_type == AMOTION_EVENT_TOOL_TYPE_UNKNOWN)
            {
                androidCursorX = AMotionEvent_getX(input_event, event_pointer_index);
                androidCursorY = AMotionEvent_getY(input_event, event_pointer_index);
               // io.AddMousePosEvent(AMotionEvent_getX(input_event, event_pointer_index), AMotionEvent_getY(input_event, event_pointer_index));
               // io.AddMouseButtonEvent(0, event_action == AMOTION_EVENT_ACTION_DOWN);
            }
            break;
        }
        case AMOTION_EVENT_ACTION_BUTTON_PRESS:
        case AMOTION_EVENT_ACTION_BUTTON_RELEASE:
        {
            int32_t button_state = AMotionEvent_getButtonState(input_event);
            //io.AddMouseButtonEvent(0, (button_state & AMOTION_EVENT_BUTTON_PRIMARY) != 0);
            //io.AddMouseButtonEvent(1, (button_state & AMOTION_EVENT_BUTTON_SECONDARY) != 0);
            //io.AddMouseButtonEvent(2, (button_state & AMOTION_EVENT_BUTTON_TERTIARY) != 0);
            break;
        }
        case AMOTION_EVENT_ACTION_HOVER_MOVE: // Hovering: Tool moves while NOT pressed (such as a physical mouse)
        case AMOTION_EVENT_ACTION_MOVE:       // Touch pointer moves while DOWN
            androidCursorX = AMotionEvent_getX(input_event, event_pointer_index);
            androidCursorY = AMotionEvent_getY(input_event, event_pointer_index);
            break;
        case AMOTION_EVENT_ACTION_SCROLL:
            //io.AddMouseWheelEvent(AMotionEvent_getAxisValue(input_event, AMOTION_EVENT_AXIS_HSCROLL, event_pointer_index), AMotionEvent_getAxisValue(input_event, AMOTION_EVENT_AXIS_VSCROLL, event_pointer_index));
            break;
        default:
            break;
        }
    }
    return 1;
    default:
        break;
    }

    return 0;
}

void onStart(ANativeActivity* activity)
{
	ERMY_LOG("onStart");
	mainThread = new std::thread([]()
	{
		while (gMainWindow == nullptr)
		{
			std::this_thread::sleep_for(1ms);
		}

		ErmyApplicationStart();
		while (ErmyApplicationStep())
		{
			if (gMainInputQueue != nullptr)
			{
				while (AInputQueue_hasEvents(gMainInputQueue) > 0)
				{
					AInputEvent* event = nullptr;
					AInputQueue_getEvent(gMainInputQueue, &event);

					if (AInputQueue_preDispatchEvent(gMainInputQueue, event)) {
						continue;
					}

					int32_t handled = 0;
                    
                    HandleInputEvent(event);
					ImGui_ImplAndroid_HandleInputEvent(event);
					//pass to input system

					AInputQueue_finishEvent(gMainInputQueue, event, handled);
				}
			}

			//std::this_thread::sleep_for(1ms);
		}
		ErmyApplicationShutdown();
	});
}

/**
 * NativeActivity has resumed.  See Java documentation for Activity.onResume()
 * for more information.
 */
void onResume(ANativeActivity* activity)
{
	ERMY_LOG("onResume");
}

/**
 * Framework is asking NativeActivity to save its current instance state.
 * See Java documentation for Activity.onSaveInstanceState() for more
 * information.  The returned pointer needs to be created with malloc();
 * the framework will call free() on it for you.  You also must fill in
 * outSize with the number of bytes in the allocation.  Note that the
 * saved state will be persisted, so it can not contain any active
 * entities (pointers to memory, file descriptors, etc).
 */
void* onSaveInstanceState(ANativeActivity* activity, size_t* outSize)
{
	ERMY_LOG("onSaveInstanceState");

	void* savedData = malloc(64);
	*outSize = 64;
	return savedData;
}

/**
 * NativeActivity has paused.  See Java documentation for Activity.onPause()
 * for more information.
 */
void onPause(ANativeActivity* activity)
{
	ERMY_LOG("onPause");
}

/**
 * NativeActivity has stopped.  See Java documentation for Activity.onStop()
 * for more information.
 */
void onStop(ANativeActivity* activity)
{
	ERMY_LOG("onStop");
}

/**
 * NativeActivity is being destroyed.  See Java documentation for Activity.onDestroy()
 * for more information.
 */
void onDestroy(ANativeActivity* activity)
{

	//need_to_stop = true;

	//while (is_running)
	//{
	//	std::this_thread::sleep_for(16ms);
	//}

	//mainThread->join();
	//delete mainThread;

	ERMY_LOG("onDestroy");
}

/**
 * Focus has changed in this NativeActivity's window.  This is often used,
 * for example, to pause a game when it loses input focus.
 */
void onWindowFocusChanged(ANativeActivity* activity, int hasFocus)
{
	ERMY_LOG("onWindowFocusChanged");
}

/**
 * The drawing window for this native activity has been created.  You
 * can use the given native window object to start drawing.
 */
void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window)
{
	ERMY_LOG("onNativeWindowCreated");
	gMainWindow = window;
}

/**
 * The drawing window for this native activity has been resized.  You should
 * retrieve the new size from the window and ensure that your rendering in
 * it now matches.
 */
void onNativeWindowResized(ANativeActivity* activity, ANativeWindow* window)
{
	ERMY_LOG("onNativeWindowResized");
}

/**
 * The drawing window for this native activity needs to be redrawn.  To avoid
 * transient artifacts during screen changes (such resizing after rotation),
 * applications should not return from this function until they have finished
 * drawing their window in its current state.
 */
void onNativeWindowRedrawNeeded(ANativeActivity* activity, ANativeWindow* window)
{
	ERMY_LOG("onNativeWindowRedrawNeeded");
}

/**
 * The drawing window for this native activity is going to be destroyed.
 * You MUST ensure that you do not touch the window object after returning
 * from this function: in the common case of drawing to the window from
 * another thread, that means the implementation of this callback must
 * properly synchronize with the other thread to stop its drawing before
 * returning from here.
 */
void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window)
{
	ERMY_LOG("onNativeWindowDestroyed");
	gMainWindow = nullptr;
}
/**
 * The input queue for this native activity's window has been created.
 * You can use the given input queue to start retrieving input events.
 */
void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue)
{
	ERMY_LOG("onInputQueueCreated");
	gMainInputQueue = queue;
}

/**
 * The input queue for this native activity's window is being destroyed.
 * You should no longer try to reference this object upon returning from this
 * function.
 */
void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue)
{
	gMainInputQueue = nullptr;
	ERMY_LOG("onInputQueueDestroyed");
}

/**
 * The rectangle in the window in which content should be placed has changed.
 */
void onContentRectChanged(ANativeActivity* activity, const ARect* rect)
{
	ERMY_LOG("onContentRectChanged");
}

/**
 * The current device AConfiguration has changed.  The new configuration can
 * be retrieved from assetManager.
 */
void onConfigurationChanged(ANativeActivity* activity)
{
	ERMY_LOG("onConfigurationChanged");
}

/**
 * The system is running low on memory.  Use this callback to release
 * resources you do not need, to help the system avoid killing more
 * important processes.
 */
void onLowMemory(ANativeActivity* activity)
{
	ERMY_LOG("onLowMemory");
}

extern "C" JNIEXPORT void ANativeActivity_onCreate(ANativeActivity* activity,
	void* savedState, size_t savedStateSize)
{
	activity->callbacks->onConfigurationChanged = onConfigurationChanged;
	activity->callbacks->onContentRectChanged = onContentRectChanged;
	activity->callbacks->onDestroy = onDestroy;
	activity->callbacks->onInputQueueCreated = onInputQueueCreated;
	activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;
	activity->callbacks->onLowMemory = onLowMemory;
	activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
	activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
	activity->callbacks->onNativeWindowRedrawNeeded = onNativeWindowRedrawNeeded;
	activity->callbacks->onNativeWindowResized = onNativeWindowResized;
	activity->callbacks->onPause = onPause;
	activity->callbacks->onResume = onResume;
	activity->callbacks->onSaveInstanceState = onSaveInstanceState;
	activity->callbacks->onStart = onStart;
	activity->callbacks->onStop = onStop;
	activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;

	ERMY_LOG("ANativeActivity_onCreate");
	//mercury::write_log_message("ANativeActivity_onCreate");
}