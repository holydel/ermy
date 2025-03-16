add_library(engine STATIC)

target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/Vulkan-Headers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/VulkanMemoryAllocator/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/miniaudio
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/OpenXR-SDK/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/OpenXR-SDK/src/common
)


# Define platform-specific source files
if(WIN32)
    set(PLATFORM_DEPENDENT_SRC
        src/os/windows/win_entry_point.cpp
        src/os/windows/win_utils.cpp
        src/os/windows/win_window.cpp
    )
elseif(ANDROID)
    set(PLATFORM_DEPENDENT_SRC
        src/os/android/android_entry_point.cpp
        src/os/android/android_utils.cpp
        src/os/android/android_window.cpp
    )
elseif(EMSCRIPTEN)
    set(PLATFORM_DEPENDENT_SRC
        src/os/emscripten/emscripten_entry_point.cpp
        src/os/emscripten/emscripten_utils.cpp
        src/os/emscripten/emscripten_window.cpp

        src/rendering/webgpu/webgpu_interface.cpp
    )    
endif()

set(VULKAN_BACKEND_SRC
src/rendering/vulkan/ermy_vulkan.cpp
src/rendering/vulkan/vulkan_swapchain.cpp
src/rendering/vulkan/vulkan_commandlist.cpp
src/rendering/vulkan/vk_utils.cpp
src/rendering/vulkan/vulkan_rendering.cpp
src/rendering/vulkan/vulkan_interface.cpp
src/rendering/vulkan/vulkan_framegraph_interface.cpp
src/rendering/internal_shaders/spirv.cpp
    )

    set(MINIAUDIO_BACKEND_SRC
src/sound/sound_interface_miniaudio.cpp
    )
    
    set(OPENXR_BACKEND_SRC
    src/xr/openxr/ermy_openxr.cpp
    src/xr/openxr/openxr_interface.cpp
    )

target_sources(engine
    PRIVATE   
    src/graphics/canvas_interface.cpp
    src/rendering/format_names.cpp

    src/imgui/ermy_imgui.cpp
    src/imgui/imgui_interface.cpp

    src/rendering/framegraph.cpp
    src/rendering/rendering.cpp
    src/rendering/swapchain_interface.cpp
    
    src/scene/scene.cpp

    src/sound/sound_interface.cpp

    src/application.cpp
    src/ermy_hash_utils.cpp
    src/ermy_string_utils.cpp
    src/ermy.cpp
    src/geometry.cpp
    src/logger.cpp
    src/pak.cpp
    src/profiler.cpp
    src/static_constraints.cpp
    src/application.cpp
    

    ${PLATFORM_DEPENDENT_SRC}
    ${VULKAN_BACKEND_SRC}
    ${OPENXR_BACKEND_SRC}
    ${MINIAUDIO_BACKEND_SRC}

    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui/imgui_widgets.cpp
        # Add source files here as they are created
)

# Export the target for use by other subprojects
set_target_properties(engine PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Make the library available to other subprojects
add_library(ermy::engine ALIAS engine)
