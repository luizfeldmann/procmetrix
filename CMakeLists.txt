# Copyright (c) 2026 Luiz Gustavo P Feldmann

CMAKE_MINIMUM_REQUIRED(VERSION 3.22)

# Project metadata
PROJECT(procmetrix LANGUAGES C)

# Generated files
SET(VERSION_FILE "${CMAKE_BINARY_DIR}/generated/version.c")
configure_file("src/version.c.in" "${VERSION_FILE}" @ONLY)

# Create the library
add_library(${CMAKE_PROJECT_NAME})

target_sources(
    ${CMAKE_PROJECT_NAME}
    # SOURCE FILES
    PRIVATE
        "src/procmetrix.c"
    # GENERATED FILES
        "${VERSION_FILE}"
    # PUBLIC INCLUDES
    PUBLIC FILE_SET public_headers
    TYPE HEADERS BASE_DIRS "include" 
    FILES
        "include/procmetrix/api.h"
        "include/procmetrix/cpu.h"
        "include/procmetrix/mem.h"
        "include/procmetrix/version.h"
        "include/procmetrix/procmetrix.h")

target_compile_definitions(${CMAKE_PROJECT_NAME}
    PRIVATE PROCMETRIX_BUILDING_LIBRARY)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC 
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> )

# Only build tests if enabled
option(PROCMETRIX_BUILD_TESTS "Build unit tests" ON)

if(PROCMETRIX_BUILD_TESTS)
    enable_language(CXX)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install target
include(GNUInstallDirs)
install(
    TARGETS ${CMAKE_PROJECT_NAME}
    EXPORT ${CMAKE_PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    FILE_SET public_headers DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
