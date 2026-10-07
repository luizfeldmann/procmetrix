
# Find dependencies
find_package(Python3 COMPONENTS Development.SABIModule REQUIRED)

# Create the module
Python3_add_library(pyprocmetrix MODULE USE_SABI 3.10 WITH_SOABI
    "include/error.h"
    "include/init.h"
    "src/error.c"
    "src/cpu.c"
    "src/mem.c"
    "src/proc.c"
    "src/init.c")

target_include_directories(pyprocmetrix PUBLIC "include")

target_link_libraries(pyprocmetrix PRIVATE
    procmetrix
    Python3::SABIModule)

set_target_properties(pyprocmetrix PROPERTIES
    PREFIX "")

# Prevent #pragma comment(lib, ...) directives from overriding the libs found by find_package(Python...)
if (MSVC)
    set(PY_DEBUG_LIB "python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}_d.lib")
    message(STATUS "Ignoring Python DEBUG lib: ${PY_DEBUG_LIB}")

    target_link_options(pyprocmetrix PRIVATE
        "$<$<CONFIG:Debug>:/NODEFAULTLIB:${PY_DEBUG_LIB}>")
endif()

# This component is installed when generating the python wheel
install(TARGETS pyprocmetrix
    COMPONENT PythonBinding # from pyproject.toml
    DESTINATION pyprocmetrix)
