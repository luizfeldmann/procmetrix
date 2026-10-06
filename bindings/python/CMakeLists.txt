
# Find dependencies
find_package(Python3 COMPONENTS Development.SABIModule REQUIRED)

# Create the module
add_library(pyprocmetrix MODULE
    "include/error.h"
    "include/init.h"
    "src/error.c"
    "src/cpu.c"
    "src/mem.c"
    "src/init.c")

target_include_directories(pyprocmetrix PUBLIC "include")

target_link_libraries(pyprocmetrix PRIVATE
    procmetrix
    Python3::SABIModule)

target_compile_definitions(pyprocmetrix PRIVATE 
    Py_LIMITED_API=0x030B0000)

set_target_properties(pyprocmetrix PROPERTIES
    PREFIX "")
