add_library(engine STATIC)

target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
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

target_sources(engine
    PRIVATE
    src/ermy.cpp
    src/static_constraints.cpp
    src/application.cpp
    src/logger.cpp
    
    src/graphics/canvas.cpp
    ${PLATFORM_DEPENDENT_SRC}
        # Add source files here as they are created
)

# Export the target for use by other subprojects
set_target_properties(engine PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Make the library available to other subprojects
add_library(ermy::engine ALIAS engine)
