// Local lib
#include <procmetrix/cpu.h>

// Testing
#include <gtest/gtest.h>

// Test cases

TEST(procmetrix_cpu_times_total, null_args)
{
    // Null output pointer
    EXPECT_EQ(procmetrix_cpu_times_total(nullptr), PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_cpu_times_per_cpu, null_args)
{
    // Null output array
    EXPECT_EQ(procmetrix_cpu_times_per_cpu(nullptr, 1, nullptr), PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Valid output array, but zero count
    procmetrix_cpu_times_t cpu_times;
    EXPECT_EQ(procmetrix_cpu_times_per_cpu(&cpu_times, 0, nullptr), PROCMETRIX_ERROR_MORE_DATA);
}
