# Use CTest
include(CTest)

# Dependency: Google Test
find_package(GTest REQUIRED)
include(GoogleTest)

# Create test application
add_executable(unitTests)

target_include_directories(unitTests PRIVATE "include")

target_link_libraries(unitTests PRIVATE ${CMAKE_PROJECT_NAME}
    GTest::GTest GTest::Main)

# Test utils
target_sources(unitTests PRIVATE
    "include/CMemFilePtr.h")

# Generic test for all platforms
target_sources(unitTests PRIVATE
    "src/sanity.cpp"
    "src/cpu.cpp")

# Per-platform sources
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_sources(unitTests PRIVATE
        "src/linux/cpu_internal.cpp" )
endif()

# Auto discover the tests
gtest_discover_tests(unitTests)
