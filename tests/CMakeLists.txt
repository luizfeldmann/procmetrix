# Use CTest
include(CTest)

# Dependency: Google Test
find_package(GTest REQUIRED)
include(GoogleTest)

# Create test application
add_executable(unitTests
    "src/sanity.cpp")

target_include_directories(unitTests PRIVATE "include")

target_link_libraries(unitTests PRIVATE ${CMAKE_PROJECT_NAME}
    GTest::GTest GTest::Main)

# Auto discover the tests
gtest_discover_tests(unitTests)
