// Private impl
#include <internal/linux/mem_linux_internal.h>

// Testing
#include <gtest/gtest.h>

// Helpers
#include "linux/CMemFilePtr.h"

// Test cases

/* Watermark */

TEST(procmetrix_impl_linux_zoneinfo_low_watermark, null_args_empty)
{
    CMemFilePtr memfp("");
    uint64_t low_wmark = 1;

    // Null file
    EXPECT_EQ(
        procmetrix_impl_linux_zoneinfo_low_watermark(nullptr, &low_wmark),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Null output
    EXPECT_EQ(
        procmetrix_impl_linux_zoneinfo_low_watermark(memfp.get(), nullptr),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Read empty
    EXPECT_EQ(
        procmetrix_impl_linux_zoneinfo_low_watermark(memfp.get(), &low_wmark),
        PROCMETRIX_ERROR_NONE);

    EXPECT_EQ(low_wmark, 0);
}

TEST(procmetrix_impl_linux_zoneinfo_low_watermark, parse)
{
    CMemFilePtr memfp(
        "low      8000\n"
        "low      7000\n"
        "low      32\n"
        "low      0\n"
    );

    uint64_t low_wmark = 0;

    EXPECT_EQ(
        procmetrix_impl_linux_zoneinfo_low_watermark(memfp.get(), &low_wmark),
        PROCMETRIX_ERROR_NONE);

    EXPECT_EQ(low_wmark, 61571072ull);
}

/** Virtual memory */

TEST(procmetrix_impl_linux_system_virtual_memory, null_args_empty)
{
    CMemFilePtr memfp1(
        "MemTotal: 16777216 kB"
    );
    CMemFilePtr memfp2(
        "MemFree: 8388608 kB"
    );

    procmetrix_virtual_memory_t virtual_memory;

    // Null file
    EXPECT_EQ(
        procmetrix_impl_linux_system_virtual_memory(nullptr, nullptr, &virtual_memory),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Null output
    EXPECT_EQ(
        procmetrix_impl_linux_system_virtual_memory(memfp1.get(), nullptr, nullptr),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Has total, missing free
    EXPECT_EQ(
        procmetrix_impl_linux_system_virtual_memory(memfp1.get(), nullptr, &virtual_memory),
        PROCMETRIX_ERROR_MALFORMED);
    
    // Has free, missing total
    EXPECT_EQ(
        procmetrix_impl_linux_system_virtual_memory(memfp2.get(), nullptr, &virtual_memory),
        PROCMETRIX_ERROR_MALFORMED);
}

TEST(procmetrix_impl_linux_system_virtual_memory, minimal)
{
    // Only the mandatory fields have been provided
    CMemFilePtr memfp(
        "MemTotal:  16777216 kB\n"
        "MemFree:   8388608  kB\n"
    );
    
    procmetrix_virtual_memory_t virtual_memory;
    EXPECT_EQ(
        procmetrix_impl_linux_system_virtual_memory(memfp.get(), nullptr, &virtual_memory),
        PROCMETRIX_ERROR_NONE);
    
    EXPECT_EQ(virtual_memory.total, 17179869184ull);
    EXPECT_EQ(virtual_memory.free,   8589934592ull);

    // Usage ratio was estimated from these only
    EXPECT_DOUBLE_EQ(virtual_memory.ratio, 0.5);
}

TEST(procmetrix_impl_linux_system_virtual_memory, all)
{
    // Only the mandatory fields have been provided
    CMemFilePtr memfp(
        "MemTotal:      8192000 kB\n"
        "MemFree:       1024000 kB\n"
        "Buffers:        512000 kB\n"
        "Cached:        2048000 kB\n"
        "SReclaimable:   256000 kB\n"
        "MemShared:      128000 kB\n"   // Alternative
        "Active:        3072000 kB\n"
        "Inactive:      1536000 kB\n"   // Alternative
        "Slab:           384000 kB\n"
        "MemAvailable:  3584000 kB\n"
    );

    procmetrix_virtual_memory_t virtual_memory;
    EXPECT_EQ(
        procmetrix_impl_linux_system_virtual_memory(memfp.get(), nullptr, &virtual_memory),
        PROCMETRIX_ERROR_NONE);

    EXPECT_EQ(virtual_memory.total,     1024 * 8192000ull);
    EXPECT_EQ(virtual_memory.free,      1024 * 1024000ull);
    EXPECT_EQ(virtual_memory.buffers,   1024 * 512000ull);
    EXPECT_EQ(virtual_memory.cached,    1024 * (2048000ull + 256000ull));
    EXPECT_EQ(virtual_memory.shared,    1024 * 128000ull);
    EXPECT_EQ(virtual_memory.active,    1024 * 3072000ull);
    EXPECT_EQ(virtual_memory.inactive,  1024 * 1536000ull);
    EXPECT_EQ(virtual_memory.slab,      1024 * 384000ull);
    EXPECT_EQ(virtual_memory.available, 1024 * 3584000ull);

    EXPECT_EQ(
        virtual_memory.used, 
        virtual_memory.total - virtual_memory.available);

    EXPECT_FLOAT_EQ(
        virtual_memory.ratio, 
        (double)virtual_memory.used / (double)virtual_memory.total);
}

TEST(procmetrix_impl_linux_system_virtual_memory, estimate)
{
    CMemFilePtr zoneinfo(
        "low      6922\n"
        "low      7156\n"
        "low      32\n"
        "low      0\n"
    );

    CMemFilePtr meminfo(
        "MemTotal:	    8132224\n"
        "MemFree:	    2585408\n"
        "Buffers:	    48824\n"
        "Cached:	    1443556\n"
        "SReclaimable:	69200\n"
        "Shmem:     	17604\n"
        "Active:	    659832\n"
        "Active(file):	656904\n"
        "Inactive(file):817964\n"
        "Inact_dirty:	2170754\n"
        "Inact_clean:	1085377\n"
        "Inact_laundry:	1085377\n"
        "Slab:	        152516\n"
    );

    procmetrix_virtual_memory_t virtual_memory;
    EXPECT_EQ(
        procmetrix_impl_linux_system_virtual_memory(meminfo.get(), zoneinfo.get(), &virtual_memory),
        PROCMETRIX_ERROR_NONE);

    // Plain fields
    EXPECT_EQ(virtual_memory.total,     1024 *  8132224ull);
    EXPECT_EQ(virtual_memory.free,      1024 *  2585408ull);
    EXPECT_EQ(virtual_memory.buffers,   1024 *    48824ull);
    EXPECT_EQ(virtual_memory.cached,    1024 * (1443556ull + 69200ull));
    EXPECT_EQ(virtual_memory.shared,    1024 *   17604ull);
    EXPECT_EQ(virtual_memory.active,    1024 *  659832ull);
    EXPECT_EQ(virtual_memory.inactive,  1024 *(2170754ull + 1085377ull + 1085377ull));
    EXPECT_EQ(virtual_memory.slab,      1024 *  152516ull);

    // Estimate
    EXPECT_EQ(virtual_memory.available, 4077563904ull);
}
