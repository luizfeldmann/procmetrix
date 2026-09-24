// Local proj
#include "main.h"

// Testing
#include <gtest/gtest.h>

//! Stores the arguments passed to main
std::vector<std::string> g_main_args;

std::vector<std::string> get_main_arguments()
{
    return g_main_args;
}

//! Entry point
int main(int argc, char **argv) {
  // Save the input args
  g_main_args.reserve(argc);
  for (size_t i = 0; i < argc; ++i)
    g_main_args.emplace_back(argv[i]);

  // Initializes & runs GTEST
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
