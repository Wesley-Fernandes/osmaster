cmake_minimum_required(VERSION 3.15)
project(osmaster_native)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find node-addon-api
execute_process(
    COMMAND node -p "require('node-addon-api').include"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE NODE_ADDON_API_RAW_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE "\"" "" NODE_ADDON_API_INCLUDE ${NODE_ADDON_API_RAW_PATH})

# Local Dependencies
set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ZLIB_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/zlib)

set(EXPAT_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(EXPAT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(EXPAT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(EXPAT_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/expat)

add_subdirectory(deps/bzip2)

# Include directories
include_directories(
    "${NODE_ADDON_API_INCLUDE}"
    "${CMAKE_JS_INC}"
    "deps/libosmium/include"
    "deps/protozero/include"
    "deps/zlib"
    "${CMAKE_BINARY_DIR}/deps/zlib"
    "deps/expat/lib"
    "deps/bzip2"
    include
    core
)

# Sources
file(GLOB_RECURSE CORE_SOURCES "core/*.cpp")
file(GLOB_RECURSE BINDING_SOURCES "bindings/*.cpp")

add_library(${PROJECT_NAME} SHARED
    ${CORE_SOURCES}
    ${BINDING_SOURCES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# Link with N-API, node.lib and dependencies
target_compile_definitions(${PROJECT_NAME} PRIVATE NAPI_VERSION=8)

# OSMIUM Windows specific definitions
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
endif()

target_link_libraries(${PROJECT_NAME} 
    "${CMAKE_JS_LIB}"
    zlibstatic
    expat
    bz2
)
