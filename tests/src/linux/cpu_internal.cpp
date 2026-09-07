// Private impl
#include <internal/linux/cpu_linux_internal.h>

// STD
#include <fstream>

// Testing
#include <gtest/gtest.h>

// Helpers
#include "linux/CMemFilePtr.h"
#include "linux/CTempDirGlob.h"

// Test cases

/* COUNT CPUS (PHYSICAL) */

TEST(procmetrix_impl_linux_cpu_count_physical_topology, null_args)
{
    EXPECT_EQ(
        procmetrix_impl_linux_cpu_count_physical_topology(nullptr), 0);
}

TEST(procmetrix_impl_linux_cpu_count_physical_topology, single_core_no_smt)
{
    // Create temp sysfs
    CTempDirGlob temp;

    // CPU 0
    ASSERT_TRUE(temp.CreateDir("cpu0"));
    ASSERT_TRUE(temp.CreateDir("cpu0/topology"));
    ASSERT_TRUE(temp.WriteFile("cpu0/topology/core_id", "0"));
    ASSERT_TRUE(temp.WriteFile("cpu0/topology/physical_package_id", "0"));

    // Glob the generated files
    glob_t glob;
    EXPECT_TRUE(temp.Glob(&glob));

    // Count the physical CPUs
    EXPECT_EQ(
        procmetrix_impl_linux_cpu_count_physical_topology(&glob), 1);

    // Cleanup
    globfree(&glob);
}

TEST(procmetrix_impl_linux_cpu_count_physical_topology, dual_core_no_smt)
{
    // Create temp sysfs
    CTempDirGlob temp;

    // CPU 0
    ASSERT_TRUE(temp.CreateDir("cpu0"));
    ASSERT_TRUE(temp.CreateDir("cpu0/topology"));
    ASSERT_TRUE(temp.WriteFile("cpu0/topology/core_id", "0"));
    ASSERT_TRUE(temp.WriteFile("cpu0/topology/physical_package_id", "0"));

    // CPU 1
    ASSERT_TRUE(temp.CreateDir("cpu1"));
    ASSERT_TRUE(temp.CreateDir("cpu1/topology"));
    ASSERT_TRUE(temp.WriteFile("cpu1/topology/core_id", "1"));
    ASSERT_TRUE(temp.WriteFile("cpu1/topology/physical_package_id", "0"));

    // Glob the generated files
    glob_t glob;
    EXPECT_TRUE(temp.Glob(&glob));

    // Count the physical CPUs
    EXPECT_EQ(
        procmetrix_impl_linux_cpu_count_physical_topology(&glob), 2);

    // Cleanup
    globfree(&glob);
}

TEST(procmetrix_impl_linux_cpu_count_physical_topology, dual_core_hyperthreading)
{
    // Create temp sysfs
    CTempDirGlob temp;

    // CPU 0
    ASSERT_TRUE(temp.CreateDir("cpu0"));
    ASSERT_TRUE(temp.CreateDir("cpu0/topology"));
    ASSERT_TRUE(temp.WriteFile("cpu0/topology/core_id", "0"));
    ASSERT_TRUE(temp.WriteFile("cpu0/topology/physical_package_id", "0"));

    // CPU 1
    ASSERT_TRUE(temp.CreateDir("cpu1"));
    ASSERT_TRUE(temp.CreateDir("cpu1/topology"));
    ASSERT_TRUE(temp.WriteFile("cpu1/topology/core_id", "1"));
    ASSERT_TRUE(temp.WriteFile("cpu1/topology/physical_package_id", "0"));

    // CPU 2
    ASSERT_TRUE(temp.CreateDir("cpu2"));
    ASSERT_TRUE(temp.CreateDir("cpu2/topology"));
    ASSERT_TRUE(temp.WriteFile("cpu2/topology/core_id", "0"));
    ASSERT_TRUE(temp.WriteFile("cpu2/topology/physical_package_id", "0"));

    // CPU 3
    ASSERT_TRUE(temp.CreateDir("cpu3"));
    ASSERT_TRUE(temp.CreateDir("cpu3/topology"));
    ASSERT_TRUE(temp.WriteFile("cpu3/topology/core_id", "1"));
    ASSERT_TRUE(temp.WriteFile("cpu3/topology/physical_package_id", "0"));

    // Glob the generated files
    glob_t glob;
    EXPECT_TRUE(temp.Glob(&glob));

    // Count the physical CPUs
    EXPECT_EQ(
        procmetrix_impl_linux_cpu_count_physical_topology(&glob), 2);

    // Cleanup
    globfree(&glob);
}

TEST(procmetrix_impl_linux_cpu_count_physical_topology, quad_core_dual_socket)
{
    // Create temp sysfs
    CTempDirGlob temp;

    // CPU 0
    ASSERT_TRUE(temp.CreateDir("cpu0"));
    ASSERT_TRUE(temp.CreateDir("cpu0/topology"));
    ASSERT_TRUE(temp.WriteFile("cpu0/topology/core_id", "0"));
    ASSERT_TRUE(temp.WriteFile("cpu0/topology/physical_package_id", "0"));

    // CPU 1
    ASSERT_TRUE(temp.CreateDir("cpu1"));
    ASSERT_TRUE(temp.CreateDir("cpu1/topology"));
    ASSERT_TRUE(temp.WriteFile("cpu1/topology/core_id", "1"));
    ASSERT_TRUE(temp.WriteFile("cpu1/topology/physical_package_id", "0"));

    // CPU 2
    ASSERT_TRUE(temp.CreateDir("cpu2"));
    ASSERT_TRUE(temp.CreateDir("cpu2/topology"));
    ASSERT_TRUE(temp.WriteFile("cpu2/topology/core_id", "0"));
    ASSERT_TRUE(temp.WriteFile("cpu2/topology/physical_package_id", "1"));

    // CPU 3
    ASSERT_TRUE(temp.CreateDir("cpu3"));
    ASSERT_TRUE(temp.CreateDir("cpu3/topology"));
    ASSERT_TRUE(temp.WriteFile("cpu3/topology/core_id", "1"));
    ASSERT_TRUE(temp.WriteFile("cpu3/topology/physical_package_id", "1"));

    // Glob the generated files
    glob_t glob;
    EXPECT_TRUE(temp.Glob(&glob));

    // Count the physical CPUs
    EXPECT_EQ(
        procmetrix_impl_linux_cpu_count_physical_topology(&glob), 4);

    // Cleanup
    globfree(&glob);
}

TEST(procmetrix_impl_linux_cpu_count_physical_cpuinfo, null_args)
{
    EXPECT_EQ(
        procmetrix_impl_linux_cpu_count_physical_cpuinfo(nullptr), 0);
}

TEST(procmetrix_impl_linux_cpu_count_physical_cpuinfo, empty)
{
    // Empty file
    CMemFilePtr memfp("");

    EXPECT_EQ(
        procmetrix_impl_linux_cpu_count_physical_cpuinfo(memfp.get()), 0);
}

TEST(procmetrix_impl_linux_cpu_count_physical_cpuinfo, single_core)
{
    CMemFilePtr memfp(
        "processor   : 0\n"
        "physical id : 0\n"
        "core id     : 0\n"
    );

    EXPECT_EQ(
        procmetrix_impl_linux_cpu_count_physical_cpuinfo(memfp.get()), 1);
}

TEST(procmetrix_impl_linux_cpu_count_physical_cpuinfo, dual_core_no_smt)
{
    CMemFilePtr memfp(
        "processor   : 0\n"
        "physical id : 0\n"
        "core id     : 0\n"
        "processor   : 1\n"
        "physical id : 0\n"
        "core id     : 1\n"
    );

    EXPECT_EQ(
        procmetrix_impl_linux_cpu_count_physical_cpuinfo(memfp.get()), 2);
}

TEST(procmetrix_impl_linux_cpu_count_physical_cpuinfo, dual_core_hyperthreading)
{
    CMemFilePtr memfp(
        "processor      :0 \n"
        "physical id    :0 \n"
        "core id        :0 \n"
        "processor      :1 \n"
        "physical id    :0 \n"
        "core id        :0 \n"
        "processor      :2 \n"
        "physical id    :0 \n"
        "core id        :1 \n"
        "processor      :3 \n"
        "physical id    :0 \n"
        "core id        :1 \n"
    );

    EXPECT_EQ(
        procmetrix_impl_linux_cpu_count_physical_cpuinfo(memfp.get()), 2);
}

TEST(procmetrix_impl_linux_cpu_count_physical_cpuinfo, quad_core_dual_socket)
{
    CMemFilePtr memfp(
        "processor      : 0 \n"
        "physical id    : 0 \n"
        "core id        : 0 \n"

        "processor      : 1 \n"
        "physical id    : 0 \n"
        "core id        : 1 \n"

        "processor      : 2 \n"
        "physical id    : 1 \n"
        "core id        : 0 \n"

        "processor      : 3 \n"
        "physical id    : 1 \n"
        "core id        : 1 \n"
    );

    EXPECT_EQ(
        procmetrix_impl_linux_cpu_count_physical_cpuinfo(memfp.get()), 4);
}

/* COUNT CPUS (LOGICAL) */

TEST(procmetrix_impl_linux_procstat_count_cpus, null_args)
{
    EXPECT_EQ(
        procmetrix_impl_linux_procstat_count_cpus(nullptr), 0);
}

TEST(procmetrix_impl_linux_procstat_count_cpus, empty)
{
    // Empty file
    CMemFilePtr memfp("");

    EXPECT_EQ(
        procmetrix_impl_linux_procstat_count_cpus(memfp.get()), 0);
}

TEST(procmetrix_impl_linux_procstat_count_cpus, count)
{
    CMemFilePtr memfp(
        "cpu\n"
        "cpu0\n"
        "cpu1\n"
        "cpu2\n"
        "whatver"
    );

    EXPECT_EQ(
        procmetrix_impl_linux_procstat_count_cpus(memfp.get()), 3);
}

TEST(procmetrix_impl_linux_cpuinfo_count_processors, null_args)
{
    EXPECT_EQ(
        procmetrix_impl_linux_cpuinfo_count_processors(nullptr), 0);
}

TEST(procmetrix_impl_linux_cpuinfo_count_processors, empty)
{
    // Empty file
    CMemFilePtr memfp("");

    EXPECT_EQ(
        procmetrix_impl_linux_cpuinfo_count_processors(memfp.get()), 0);
}

TEST(procmetrix_impl_linux_cpuinfo_count_processors, count)
{
    CMemFilePtr memfp(
        "processor      : 0\n"
        "physical id    : 0\n"
        "siblings       : 8\n"
        "core id        : 0\n"
        "processor      : 1\n"
        "physical id    : 0\n"
        "siblings       : 8\n"
        "core id        : 0\n"
    );

    EXPECT_EQ(
        procmetrix_impl_linux_cpuinfo_count_processors(memfp.get()), 2);
}

/* TOTAL TIMES */

TEST(procmetrix_impl_linux_cpu_times_total, null_args)
{
    EXPECT_EQ(
        procmetrix_impl_linux_cpu_times_total(nullptr, nullptr), 
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

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

/* PER-CPU TIMES */

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

/** FREQUENCIES */

TEST(procmetrix_impl_linux_cpu_freqs_policies, null_args)
{
    glob_t glob;
    size_t read_count = 0;
    procmetrix_cpu_freq_t cpu_freq { 0 };

    //! Null glob
    EXPECT_EQ(
        procmetrix_impl_linux_cpu_freqs_policies(nullptr, &cpu_freq, 1, &read_count),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
    
    // Null output array
    EXPECT_EQ(
        procmetrix_impl_linux_cpu_freqs_policies(&glob, nullptr, 1, &read_count),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Zero output size
    EXPECT_EQ(
        procmetrix_impl_linux_cpu_freqs_policies(&glob, &cpu_freq, 0, &read_count),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_impl_linux_cpu_freqs_policies, tempfiles)
{
    // Create temp sysfs
    CTempDirGlob temp;

    // Policy 0
    // Affects cputs 0, 2
    ASSERT_TRUE(temp.CreateDir("policy0"));
    ASSERT_TRUE(temp.WriteFile("policy0/affected_cpus", "0 2"));
    ASSERT_TRUE(temp.WriteFile("policy0/scaling_cur_freq", "2000000")); // Values are in kHz
    ASSERT_TRUE(temp.WriteFile("policy0/scaling_min_freq", "1000000"));
    ASSERT_TRUE(temp.WriteFile("policy0/scaling_max_freq", "3000000"));

    // Policy 1
    // Affects cpus 1, 3
    ASSERT_TRUE(temp.CreateDir("policy1"));
    ASSERT_TRUE(temp.WriteFile("policy1/affected_cpus", "1 3"));
    ASSERT_TRUE(temp.WriteFile("policy1/scaling_cur_freq", "3000000"));
    ASSERT_TRUE(temp.WriteFile("policy1/scaling_min_freq", "2000000"));
    ASSERT_TRUE(temp.WriteFile("policy1/scaling_max_freq", "4000000"));

    // Glob the generated files
    glob_t glob;
    EXPECT_TRUE(temp.Glob(&glob));

    // Parse the filesystem
    size_t read_count = 0;
    procmetrix_cpu_freq_t cpu_freq[4] { 0 };

    EXPECT_EQ(
        procmetrix_impl_linux_cpu_freqs_policies(&glob, cpu_freq, std::size(cpu_freq), &read_count),
        PROCMETRIX_ERROR_NONE);

    // Cleanup
    globfree(&glob);

    // Check read output
    EXPECT_EQ(read_count, 4);

    EXPECT_DOUBLE_EQ(cpu_freq[0].freq_cur, 2000.0);
    EXPECT_DOUBLE_EQ(cpu_freq[0].freq_min, 1000.0);
    EXPECT_DOUBLE_EQ(cpu_freq[0].freq_max, 3000.0);

    EXPECT_DOUBLE_EQ(cpu_freq[2].freq_cur, 2000.0);
    EXPECT_DOUBLE_EQ(cpu_freq[2].freq_min, 1000.0);
    EXPECT_DOUBLE_EQ(cpu_freq[2].freq_max, 3000.0);

    EXPECT_DOUBLE_EQ(cpu_freq[1].freq_cur, 3000.0);
    EXPECT_DOUBLE_EQ(cpu_freq[1].freq_min, 2000.0);
    EXPECT_DOUBLE_EQ(cpu_freq[1].freq_max, 4000.0);
    
    EXPECT_DOUBLE_EQ(cpu_freq[3].freq_cur, 3000.0);
    EXPECT_DOUBLE_EQ(cpu_freq[3].freq_min, 2000.0);
    EXPECT_DOUBLE_EQ(cpu_freq[3].freq_max, 4000.0);
}

TEST(procmetrix_impl_linux_cpuinfo_freqs, null_args)
{
    size_t read_count = 0;
    procmetrix_cpu_freq_t cpu_freq { 0 };
    CMemFilePtr memfp("");

    // Null file
    EXPECT_EQ(
        procmetrix_impl_linux_cpuinfo_freqs(nullptr, &cpu_freq, 1, &read_count),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Null output
    EXPECT_EQ(
        procmetrix_impl_linux_cpuinfo_freqs(memfp.get(), nullptr, 1, &read_count),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Zero size output
    EXPECT_EQ(
        procmetrix_impl_linux_cpuinfo_freqs(memfp.get(), &cpu_freq, 0, &read_count),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_impl_linux_cpuinfo_freqs, empty)
{
    size_t read_count = 0;
    procmetrix_cpu_freq_t cpu_freq { 0 };

    // Empty input file
    CMemFilePtr memfp("");

    // Succeeds reading zero items
    EXPECT_EQ(
        procmetrix_impl_linux_cpuinfo_freqs(memfp.get(), &cpu_freq, 1, &read_count),
        PROCMETRIX_ERROR_NONE);

    EXPECT_EQ(read_count, 0);
}

TEST(procmetrix_impl_linux_cpuinfo_freqs, mix_items)
{
    size_t read_count = 0;
    procmetrix_cpu_freq_t cpu_freq[4];

    CMemFilePtr memfp(
        // x86 format
        "processor : 0\n"
        "cpu MHz   : 2533.3\n"

        "processor : 1\n"
        "cpu MHz   : 3000\n"

        // Power PC format
        "processor : 2\n"
        "cpu MHz   : 2400.0\n"

        // s390x
        "processor      :3\n"
        "cpu MHz dynamic:1234.5\n"
    );

    // Succeeds reading all items
    EXPECT_EQ(
        procmetrix_impl_linux_cpuinfo_freqs(memfp.get(), cpu_freq, std::size(cpu_freq), &read_count),
        PROCMETRIX_ERROR_NONE);

    EXPECT_EQ(read_count, 4);
    EXPECT_DOUBLE_EQ(cpu_freq[0].freq_cur, 2533.3);
    EXPECT_DOUBLE_EQ(cpu_freq[1].freq_cur, 3000.0);
    EXPECT_DOUBLE_EQ(cpu_freq[2].freq_cur, 2400.0);
    EXPECT_DOUBLE_EQ(cpu_freq[3].freq_cur, 1234.5);
}

TEST(procmetrix_impl_linux_cpuinfo_freqs, overflow)
{
    // Array fits 2 but actual count is 4
    size_t read_count = 0;
    procmetrix_cpu_freq_t cpu_freq[2];

    CMemFilePtr memfp(
        "cpu MHz   : 1000\n"
        "cpu MHz   : 2000\n"
        "cpu MHz   : 3000\n"
        "cpu MHz   : 4000\n"
    );

    // Returns "more data was available"
    EXPECT_EQ(
        procmetrix_impl_linux_cpuinfo_freqs(memfp.get(), cpu_freq, std::size(cpu_freq), &read_count),
        PROCMETRIX_ERROR_MORE_DATA);
    
    // The first 2 items were read correctly
    EXPECT_EQ(read_count, 2);
    EXPECT_DOUBLE_EQ(cpu_freq[0].freq_cur, 1000.0);
    EXPECT_DOUBLE_EQ(cpu_freq[1].freq_cur, 2000.0);
}
