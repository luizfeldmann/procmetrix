// Private impl
#include <internal/linux/cpu_linux_internal.h>

// Testing
#include <gtest/gtest.h>

// Helpers
#include "CMemFilePtr.h"

// Test cases

TEST(procmetrix_impl_linux_cpu_times_total, empty_file)
{
    // Empty file
    CMemFilePtr memfp("");

    procmetrix_cpu_times_t cpu_times {0};
    EXPECT_EQ(
        procmetrix_impl_linux_cpu_times_total(memfp.get(), &cpu_times), 
        PROCMETRIX_ERROR_MALFORMED);
}

TEST(procmetrix_impl_linux_cpu_times_total, minimal)
{
    // Only the minimal metrics are given
    CMemFilePtr memfp("cpu 100 200 300 400");

    procmetrix_cpu_times_t cpu_times {0};
    EXPECT_EQ(
        procmetrix_impl_linux_cpu_times_total(memfp.get(), &cpu_times),
        PROCMETRIX_ERROR_NONE);

    // Check minimal values in seconds
    long ticks_per_second = sysconf(_SC_CLK_TCK);
    EXPECT_DOUBLE_EQ(cpu_times.user, 100.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times.nice, 200.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times.system, 300.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times.idle, 400.0 / ticks_per_second);
}

TEST(procmetrix_impl_linux_cpu_times_total, full)
{
    // All the metrics are given
    CMemFilePtr memfp("cpu 0 0 0 0 100 200 300 400 500 600");

    procmetrix_cpu_times_t cpu_times {0};
    EXPECT_EQ(
        procmetrix_impl_linux_cpu_times_total(memfp.get(), &cpu_times), 
        PROCMETRIX_ERROR_NONE);

    // Check in seconds
    long ticks_per_second = sysconf(_SC_CLK_TCK);
    EXPECT_DOUBLE_EQ(cpu_times.iowait, 100.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times.irq, 200.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times.softirq, 300.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times.steal, 400.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times.guest, 500.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times.guest_nice, 600.0 / ticks_per_second);
}

TEST(procmetrix_impl_linux_cpu_times_per_cpu, empty_file)
{
    CMemFilePtr memfp("");

    size_t read_count = 0;
    procmetrix_cpu_times_t cpu_times[1];
    EXPECT_EQ(
        procmetrix_impl_linux_cpu_times_per_cpu(memfp.get(), cpu_times, std::size(cpu_times), &read_count),
        PROCMETRIX_ERROR_MALFORMED);
}

TEST(procmetrix_impl_linux_cpu_times_per_cpu, single_line)
{
    // One total and one cpu
    CMemFilePtr memfp(
        "cpu\n"
        "cpu0 1000 2000 3000 4000"
    );

    // Read into one line
    size_t read_count = 0;
    procmetrix_cpu_times_t cpu_times[1];
    EXPECT_EQ(
        procmetrix_impl_linux_cpu_times_per_cpu(memfp.get(), cpu_times, std::size(cpu_times), &read_count),
        PROCMETRIX_ERROR_NONE);

    // Read exactly one item
    EXPECT_EQ(read_count, 1);

    // Check every element
    long ticks_per_second = sysconf(_SC_CLK_TCK);
    EXPECT_DOUBLE_EQ(cpu_times[0].user, 1000.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[0].nice, 2000.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[0].system, 3000.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[0].idle, 4000.0 / ticks_per_second);
}

TEST(procmetrix_impl_linux_cpu_times_per_cpu, multi_line)
{
    // One total and two cpus
    CMemFilePtr memfp(
        "cpu\n"
        "cpu0 1000 2000 3000 4000 5000 6000 7000 8000 9000 10000\n"
        "cpu1 1500 2500 3500 4500 5500 6500 7500 8500 9500 15000\n"
    );

    size_t read_count = 0;
    procmetrix_cpu_times_t cpu_times[2];
    EXPECT_EQ(
        procmetrix_impl_linux_cpu_times_per_cpu(memfp.get(), cpu_times, std::size(cpu_times), &read_count),
        PROCMETRIX_ERROR_NONE);

    // Read exactly one item
    EXPECT_EQ(read_count, 2);

    // Check every element in 1st row
    long ticks_per_second = sysconf(_SC_CLK_TCK);
    EXPECT_DOUBLE_EQ(cpu_times[0].user, 1000.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[0].nice, 2000.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[0].system, 3000.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[0].idle, 4000.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[0].iowait, 5000.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[0].irq, 6000.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[0].softirq, 7000.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[0].steal, 8000.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[0].guest, 9000.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[0].guest_nice, 10000.0 / ticks_per_second);

    // Check 2nd row
    EXPECT_DOUBLE_EQ(cpu_times[1].user, 1500.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[1].nice, 2500.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[1].system, 3500.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[1].idle, 4500.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[1].iowait, 5500.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[1].irq, 6500.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[1].softirq, 7500.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[1].steal, 8500.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[1].guest, 9500.0 / ticks_per_second);
    EXPECT_DOUBLE_EQ(cpu_times[1].guest_nice, 15000.0 / ticks_per_second);
}

TEST(procmetrix_impl_linux_cpu_times_per_cpu, array_too_small)
{
    // One total and 4 cpus
    CMemFilePtr memfp(
        "cpu\n"
        "cpu0 1000 2000 3000 4000\n"
        "cpu1 1250 2250 3250 4250\n"
        "cpu2 1500 2500 3500 4500\n"
        "cpu3 1750 2750 3750 4750\n"
    );

    // The provided array is smaller than the actual count
    size_t read_count = 0;
    procmetrix_cpu_times_t cpu_times[2];
    EXPECT_EQ(
        procmetrix_impl_linux_cpu_times_per_cpu(memfp.get(), cpu_times, std::size(cpu_times), &read_count),
        PROCMETRIX_ERROR_MORE_DATA); // Request more buffer

    // Only read the first two lines
    EXPECT_EQ(read_count, 2);
}
