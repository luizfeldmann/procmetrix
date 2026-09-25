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
    mem_file_ptr memfp("");
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
    mem_file_ptr memfp(
        "low      8000\n"
        "low      7000\n"
        "low      32\n"
        "low      0\n");

    uint64_t low_wmark = 0;

    EXPECT_EQ(
        procmetrix_impl_linux_zoneinfo_low_watermark(memfp.get(), &low_wmark),
        PROCMETRIX_ERROR_NONE);

    EXPECT_EQ(low_wmark, 61571072ULL);
}

/** Virtual memory */

TEST(procmetrix_impl_linux_system_virtual_memory, null_args_empty)
{
    mem_file_ptr memfp1("MemTotal: 16777216 kB");
    mem_file_ptr memfp2("MemFree: 8388608 kB");

    procmetrix_virtual_memory_t virtual_memory;

    // Null file
    EXPECT_EQ(
        procmetrix_impl_linux_system_virtual_memory(
            nullptr, nullptr, &virtual_memory),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Null output
    EXPECT_EQ(
        procmetrix_impl_linux_system_virtual_memory(
            memfp1.get(), nullptr, nullptr),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Has total, missing free
    EXPECT_EQ(
        procmetrix_impl_linux_system_virtual_memory(
            memfp1.get(), nullptr, &virtual_memory),
        PROCMETRIX_ERROR_MALFORMED);

    // Has free, missing total
    EXPECT_EQ(
        procmetrix_impl_linux_system_virtual_memory(
            memfp2.get(), nullptr, &virtual_memory),
        PROCMETRIX_ERROR_MALFORMED);
}

TEST(procmetrix_impl_linux_system_virtual_memory, minimal)
{
    // Only the mandatory fields have been provided
    mem_file_ptr memfp(
        "MemTotal:  16777216 kB\n"
        "MemFree:   8388608  kB\n");

    procmetrix_virtual_memory_t virtual_memory;
    EXPECT_EQ(
        procmetrix_impl_linux_system_virtual_memory(
            memfp.get(), nullptr, &virtual_memory),
        PROCMETRIX_ERROR_NONE);

    EXPECT_EQ(virtual_memory.total, 17179869184ULL);
    EXPECT_EQ(virtual_memory.free, 8589934592ULL);

    // Usage ratio was estimated from these only
    EXPECT_DOUBLE_EQ(virtual_memory.ratio, 0.5);
}

TEST(procmetrix_impl_linux_system_virtual_memory, all)
{
    // Only the mandatory fields have been provided
    mem_file_ptr memfp(
        "MemTotal:      8192000 kB\n"
        "MemFree:       1024000 kB\n"
        "Buffers:        512000 kB\n"
        "Cached:        2048000 kB\n"
        "SReclaimable:   256000 kB\n"
        "MemShared:      128000 kB\n" // Alternative
        "Active:        3072000 kB\n"
        "Inactive:      1536000 kB\n" // Alternative
        "Slab:           384000 kB\n"
        "MemAvailable:  3584000 kB\n");

    procmetrix_virtual_memory_t virtual_memory;
    EXPECT_EQ(
        procmetrix_impl_linux_system_virtual_memory(
            memfp.get(), nullptr, &virtual_memory),
        PROCMETRIX_ERROR_NONE);

    EXPECT_EQ(virtual_memory.total, 1024 * 8192000ULL);
    EXPECT_EQ(virtual_memory.free, 1024 * 1024000ULL);
    EXPECT_EQ(virtual_memory.buffers, 1024 * 512000ULL);
    EXPECT_EQ(virtual_memory.cached, 1024 * (2048000ULL + 256000ULL));
    EXPECT_EQ(virtual_memory.shared, 1024 * 128000ULL);
    EXPECT_EQ(virtual_memory.active, 1024 * 3072000ULL);
    EXPECT_EQ(virtual_memory.inactive, 1024 * 1536000ULL);
    EXPECT_EQ(virtual_memory.slab, 1024 * 384000ULL);
    EXPECT_EQ(virtual_memory.available, 1024 * 3584000ULL);

    EXPECT_EQ(
        virtual_memory.used, virtual_memory.total - virtual_memory.available);

    EXPECT_FLOAT_EQ(
        virtual_memory.ratio,
        (double)virtual_memory.used / (double)virtual_memory.total);
}

TEST(procmetrix_impl_linux_system_virtual_memory, estimate)
{
    mem_file_ptr zoneinfo(
        "low      6922\n"
        "low      7156\n"
        "low      32\n"
        "low      0\n");

    mem_file_ptr meminfo(
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
        "Slab:	        152516\n");

    procmetrix_virtual_memory_t virtual_memory;
    EXPECT_EQ(
        procmetrix_impl_linux_system_virtual_memory(
            meminfo.get(), zoneinfo.get(), &virtual_memory),
        PROCMETRIX_ERROR_NONE);

    // Plain fields
    EXPECT_EQ(virtual_memory.total, 1024 * 8132224ULL);
    EXPECT_EQ(virtual_memory.free, 1024 * 2585408ULL);
    EXPECT_EQ(virtual_memory.buffers, 1024 * 48824ULL);
    EXPECT_EQ(virtual_memory.cached, 1024 * (1443556ULL + 69200ULL));
    EXPECT_EQ(virtual_memory.shared, 1024 * 17604ULL);
    EXPECT_EQ(virtual_memory.active, 1024 * 659832ULL);
    EXPECT_EQ(
        virtual_memory.inactive, 1024 * (2170754ULL + 1085377ULL + 1085377ULL));
    EXPECT_EQ(virtual_memory.slab, 1024 * 152516ULL);

    // Estimate
    EXPECT_EQ(virtual_memory.available, 4077563904ULL);
}

/** Swap memory */

TEST(procmetrix_impl_linux_system_swap_memory, null_args_empty)
{
    mem_file_ptr memfp1("SwapTotal: 1234 kB");
    mem_file_ptr memfp2("SwapFree: 5678 kB");

    procmetrix_swap_memory_t swap_memory;

    // Null file
    EXPECT_EQ(
        procmetrix_impl_linux_system_swap_memory(
            nullptr, nullptr, &swap_memory),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Null output
    EXPECT_EQ(
        procmetrix_impl_linux_system_swap_memory(
            memfp1.get(), nullptr, nullptr),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Has total, missing free
    EXPECT_EQ(
        procmetrix_impl_linux_system_swap_memory(
            memfp1.get(), nullptr, &swap_memory),
        PROCMETRIX_ERROR_MALFORMED);

    // Has free, missing total
    EXPECT_EQ(
        procmetrix_impl_linux_system_swap_memory(
            memfp2.get(), nullptr, &swap_memory),
        PROCMETRIX_ERROR_MALFORMED);
}

TEST(procmetrix_impl_linux_system_swap_memory, minimal)
{
    // Only the mandatory fields have been provided
    mem_file_ptr memfp(
        "SwapTotal:  5678 kB\n"
        "SwapFree:   1234 kB\n");

    procmetrix_swap_memory_t swap_memory;
    EXPECT_EQ(
        procmetrix_impl_linux_system_swap_memory(
            memfp.get(), nullptr, &swap_memory),
        PROCMETRIX_ERROR_NONE);

    EXPECT_EQ(swap_memory.total, 1024 * 5678);
    EXPECT_EQ(swap_memory.free, 1024 * 1234);
    EXPECT_EQ(swap_memory.used, 1024 * (5678 - 1234));
}

TEST(procmetrix_impl_linux_system_swap_memory, all)
{
    mem_file_ptr memstat(
        "SwapTotal:  2000 kB\n"
        "SwapFree:   1000 kB\n");

    mem_file_ptr vmstat(
        "pswpin  100 kB\n"
        "pswpout 200 kB\n");

    procmetrix_swap_memory_t swap_memory;
    EXPECT_EQ(
        procmetrix_impl_linux_system_swap_memory(
            memstat.get(), vmstat.get(), &swap_memory),
        PROCMETRIX_ERROR_NONE);

    EXPECT_EQ(swap_memory.total, 1024 * 2000);
    EXPECT_EQ(swap_memory.free, 1024 * 1000);
    EXPECT_EQ(swap_memory.swap_in, 4096 * 100);
    EXPECT_EQ(swap_memory.swap_out, 4096 * 200);
}
