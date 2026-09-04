// Local lib
#include <procmetrix/version.h>

// Testing
#include <gtest/gtest.h>

// Test cases

TEST(sanity, version)
{
    procmetrix_version_major();
    procmetrix_version_minor();
    procmetrix_version_patch();
}
