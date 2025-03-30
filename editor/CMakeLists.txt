include(FetchContent)

add_executable(editor)

if(MSVC)
    target_compile_options(editor PRIVATE "/utf-8")
endif()

# Set Compressonator library paths based on configuration
set(PREBUILT_DEBUG_LIB
"${CMAKE_CURRENT_SOURCE_DIR}/../third_party_prebuilt/compressonator/windows/Compressonator_MDd.lib"
"$ENV{VULKAN_SDK}/Lib/slangd.lib"
"$ENV{VULKAN_SDK}/Lib/slang-rtd.lib")

set(PREBUILT_RELEASE_LIB
"${CMAKE_CURRENT_SOURCE_DIR}/../third_party_prebuilt/compressonator/windows/Compressonator_MD.lib"
"$ENV{VULKAN_SDK}/Lib/slang.lib"
"$ENV{VULKAN_SDK}/Lib/slang-rt.lib")

target_include_directories(editor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/pugixml/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/miniaudio
        ${CMAKE_CURRENT_SOURCE_DIR}/../third_party_prebuilt/compressonator/include    
        $ENV{VULKAN_SDK}/Include
)

target_sources(editor
    PRIVATE

    src/assets/asset.cpp
    src/assets/assets_loader.cpp            
    src/assets/geometry_asset.cpp        
    src/assets/sound_asset.cpp
    src/assets/texture_asset.cpp
    src/assets/texture_names.cpp
    src/assets/video_asset.cpp
    
    src/assets/AssimpLoader.cpp
    src/assets/KTXLoader.cpp
    src/assets/FFMPEGLoader.cpp
    src/assets/DDSLoader.cpp
    src/assets/CompressonatorLoader.cpp
    src/assets/OpenImageLoader.cpp
    src/assets/SoundsLoader.cpp

    src/project/eproject.cpp
    src/editor_file_utils.cpp
    
    src/screens/assets_browser_screen.cpp
    src/screens/editor_screen.cpp
    src/screens/hierarchy_screen.cpp
    src/screens/property_editor_screen.cpp

    src/shader_compiler/shader_compiler.cpp

    src/shaders/spirv.cpp

    src/assets_packed_cache.cpp
    src/assets_packer.cpp
    src/assets_preview_manager.cpp
    src/compressonator_lib.cpp
    src/editor_main.cpp
    src/main_window.cpp
    src/preview_renderer.cpp        

    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/pugixml/src/pugixml.cpp
)

target_link_libraries(editor
    PRIVATE
        ermy::engine
        $<$<CONFIG:Debug>:${PREBUILT_DEBUG_LIB}>
        $<$<NOT:$<CONFIG:Debug>>:${PREBUILT_RELEASE_LIB}>
)

# Find required packages from vcpkg
find_package(OpenImageIO CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(pxr CONFIG REQUIRED)
find_package(KTX CONFIG REQUIRED)
find_package(DirectXTex CONFIG REQUIRED)

# Debug output to see available targets
get_property(targets DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY IMPORTED_TARGETS)
message(STATUS "Available imported targets: ${targets}")

target_link_directories(editor PRIVATE ${FFMPEG_LIBRARY_DIRS})

target_link_libraries(editor PRIVATE 
    OpenImageIO::OpenImageIO
    assimp::assimp
    ${FFMPEG_LIBRARIES}
    ar gf hd hf
    KTX::ktx
    Microsoft::DirectXTex
)

target_include_directories(editor PRIVATE
    ${FFMPEG_INCLUDE_DIRS}
)