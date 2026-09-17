// Local proj
#include "main.h"

// Testing
#include <gtest/gtest.h>

//! Stores the arguments passed to main
std::vector<std::string> gMainArgs;

std::vector<std::string> getMainArguments()
{
    return gMainArgs;
}

//! Entry point
int main(int argc, char **argv) {
  // Save the input args
  gMainArgs.reserve(argc);
  for (size_t i = 0; i < argc; ++i)
    gMainArgs.emplace_back(argv[i]);

  // Initializes & runs GTEST
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
