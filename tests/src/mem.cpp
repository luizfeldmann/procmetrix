// Local lib
#include <procmetrix/mem.h>

// Testing
#include <gtest/gtest.h>

// Test cases

TEST(procmetrix_system_virtual_memory, null_args)
{
    EXPECT_EQ(
        procmetrix_system_virtual_memory(nullptr),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_system_virtual_memory, non_zero)
{
    procmetrix_virtual_memory_t vmem;
    EXPECT_EQ(
        procmetrix_system_virtual_memory(&vmem),
        PROCMETRIX_ERROR_NONE);

    EXPECT_GT(vmem.total, 0);
    EXPECT_GT(vmem.free,  0);
    EXPECT_GT(vmem.total, vmem.free);
    EXPECT_GT(vmem.available, 0);
    EXPECT_GT(vmem.ratio, 0.0);
}
