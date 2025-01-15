add_library(engine STATIC)

target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_sources(engine
    PRIVATE
    src/ermy.cpp
        # Add source files here as they are created
)

# Export the target for use by other subprojects
set_target_properties(engine PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Make the library available to other subprojects
add_library(ermy::engine ALIAS engine)
