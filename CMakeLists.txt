cmake_minimum_required(VERSION 3.20)

project(ermy VERSION 0.1.0 LANGUAGES CXX)

# Set configurations
# set(CMAKE_CONFIGURATION_TYPES Debug Dev Retail)

# Debug configuration
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")
# set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")

# Dev configuration - optimized with debug info
# set(CMAKE_CXX_FLAGS_DEV "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Zi")
# set(CMAKE_EXE_LINKER_FLAGS_DEV "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /DEBUG")

# Retail configuration - optimized without debug info
#set(CMAKE_CXX_FLAGS_RETAIL "${CMAKE_CXX_FLAGS_RELEASE}")
#set(CMAKE_EXE_LINKER_FLAGS_RETAIL "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    add_compile_definitions(
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/glm)

# Emscripten WebGPU platform setup
if(EMSCRIPTEN)
    # Ensure Emscripten compiler is being used
    if(NOT CMAKE_C_COMPILER MATCHES "emcc")
        message(FATAL_ERROR "Emscripten builds require using emcc compiler")
    endif()

    # Set Emscripten specific flags for WebGPU support
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_WEBGPU=1")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_WEBGPU=1")
    
    # Additional Emscripten settings
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WASM=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ALLOW_MEMORY_GROWTH=1")
    
    # Enable threading if needed
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_PTHREADS=1")
    # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_PTHREADS=1")
endif()

add_subdirectory(engine)
add_subdirectory(testbed)
add_subdirectory(editor)