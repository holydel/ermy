add_library(engine STATIC)

target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/Vulkan-Headers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/VulkanMemoryAllocator/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui
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
src/rendering/vulkan/vulkan_interface.cpp
src/rendering/vulkan/vulkan_framegraph_interface.cpp
    )

target_sources(engine
    PRIVATE
    src/ermy.cpp
    src/static_constraints.cpp
    src/application.cpp
    src/logger.cpp
    src/logger.cpp
    src/rendering/framegraph.cpp
    src/rendering/rendering.cpp
    src/rendering/swapchain_interface.cpp
    src/graphics/canvas.cpp
    ${PLATFORM_DEPENDENT_SRC}
    ${VULKAN_BACKEND_SRC}
        # Add source files here as they are created
)

# Export the target for use by other subprojects
set_target_properties(engine PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Make the library available to other subprojects
add_library(ermy::engine ALIAS engine)
