# This target uses C++
enable_language(CXX)

# Find Node
find_program(NODE_EXECUTABLE node REQUIRED)
message(STATUS "node: ${NODE_EXECUTABLE}")

execute_process(
    COMMAND ${NODE_EXECUTABLE}
            -p "process.versions.node"
    OUTPUT_VARIABLE NODE_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "node version: ${NODE_VERSION}")

# Download and create link target for node's headers
include(FetchContent)
cmake_policy(SET CMP0135 NEW)
FetchContent_Declare(
    node_headers
    URL https://nodejs.org/dist/v${NODE_VERSION}/node-v${NODE_VERSION}-headers.tar.gz
)

FetchContent_MakeAvailable(node_headers)

add_library(node-headers INTERFACE)
target_include_directories(node-headers INTERFACE "${node_headers_SOURCE_DIR}/include/node")

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(NODE_ARCH "win-x64")
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
        set(NODE_ARCH "win-x86")
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64")
        set(NODE_ARCH "win-arm64")
    else()
        message(FATAL_ERROR "Unsupported NodeJS architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    set(NODE_LIB "${CMAKE_BINARY_DIR}/node.lib")
    file(DOWNLOAD "https://nodejs.org/dist/v${NODE_VERSION}/${NODE_ARCH}/node.lib" "${NODE_LIB}")

    target_link_libraries(node-headers INTERFACE ${NODE_LIB})
endif()

# Link target for NAPI
execute_process(
    COMMAND ${NODE_EXECUTABLE}
            -p
            "require('node-addon-api').include"
    OUTPUT_VARIABLE NODE_ADDON_API_INCLUDE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REPLACE "\"" "" NODE_ADDON_API_INCLUDE "${NODE_ADDON_API_INCLUDE}")
message(STATUS "node-addon-api: ${NODE_ADDON_API_INCLUDE}")

add_library(node-addon-api INTERFACE)
target_include_directories(node-addon-api INTERFACE "${NODE_ADDON_API_INCLUDE}")

# Create the addon
add_library(procmetrix-js SHARED
    "src/init.cpp")

target_include_directories(procmetrix-js PUBLIC "src")

target_link_libraries(procmetrix-js PRIVATE
    node-headers
    node-addon-api
    procmetrix)

target_compile_features(procmetrix-js PUBLIC cxx_std_17)

target_compile_definitions(procmetrix-js PRIVATE
    NAPI_VERSION=8
    NODE_ADDON_API_CPP_EXCEPTIONS)

set_target_properties(procmetrix-js PROPERTIES
    PREFIX "" 
    OUTPUT_NAME "procmetrix"
    SUFFIX ".node")

# This component is installed on npm install
install(TARGETS procmetrix-js
    COMPONENT NodeBinding
    DESTINATION "native")
