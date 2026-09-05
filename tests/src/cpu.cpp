// Local lib
#include <procmetrix/cpu.h>

// Testing
#include <gtest/gtest.h>
#include <gmock/gmock.h>
using namespace ::testing;

// STD
#include <thread>

// Test cases

TEST(procmetrix_cpu_times_total, null_args)
{
    // Null output pointer
    EXPECT_EQ(
        procmetrix_cpu_times_total(nullptr), 
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_cpu_times_total, sanity)
{
    // Read succeeds
    procmetrix_cpu_times_t cpu_times;
    EXPECT_EQ(procmetrix_cpu_times_total(&cpu_times), 
        PROCMETRIX_ERROR_NONE);

    // Main times are positive
    EXPECT_GT(cpu_times.user, 0.0);
    EXPECT_GT(cpu_times.idle, 0.0);
    EXPECT_GT(cpu_times.system, 0.0);
}

TEST(procmetrix_cpu_times_per_cpu, null_args)
{
    // Null output array
    EXPECT_EQ(
        procmetrix_cpu_times_per_cpu(nullptr, 1, nullptr), 
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Valid output array, but zero count
    procmetrix_cpu_times_t cpu_times;
    EXPECT_EQ(
        procmetrix_cpu_times_per_cpu(&cpu_times, 0, nullptr),
        PROCMETRIX_ERROR_MORE_DATA);
}

TEST(procmetrix_cpu_times_per_cpu, sanity)
{
    // Try to read a few CPUs
    size_t read_count = 0;
    procmetrix_cpu_times_t cpu_times[4] {};

    EXPECT_THAT(
        procmetrix_cpu_times_per_cpu(cpu_times, std::size(cpu_times), &read_count),
        AnyOf(
            Eq(PROCMETRIX_ERROR_NONE),
            Eq(PROCMETRIX_ERROR_MORE_DATA)));
    
    // At least one was read
    EXPECT_GT(read_count, 0);

    // All main counters are positive
    for (size_t i = 0; i < read_count; ++ i)
    {
        EXPECT_GT(cpu_times[i].user, 0.0);
        EXPECT_GT(cpu_times[i].idle, 0.0);
        EXPECT_GT(cpu_times[i].system, 0.0);
    }
}

TEST(procmetrix_cpu_times_delta, null_args)
{
    procmetrix_cpu_times_t cpu_times, delta;

    // 1st arg null
    EXPECT_EQ(
        procmetrix_cpu_times_delta(nullptr, &cpu_times, &delta),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // 2nd arg null
    EXPECT_EQ(
        procmetrix_cpu_times_delta(&cpu_times, nullptr, &delta), 
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // 3rd arg null
    EXPECT_EQ(
        procmetrix_cpu_times_delta(&cpu_times, &cpu_times, nullptr), 
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_cpu_times_delta, compute)
{
    const procmetrix_cpu_times_t before {
        100.5,
        150.6,
        125.9,
        200.8,
        312.1,
        712.4,
        812.5,
        499.9,
        647.5,
        578.7,
        831.0,
        946.2,
    };

    const procmetrix_cpu_times_t after {
        119.6,
        178.5,
        146.8,
        241.9,
        363.2,
        784.1,
        906.9,
        508.5,
        659.6,
        593.0,
        862.1,
        981.4,
    };

    const procmetrix_cpu_times expected_diff {
        19.1,
        27.9,
        20.9,
        41.1,
        51.1,
        71.7,
        94.4,
         8.6,
        12.1,
        14.3,
        31.1,
        35.2,
    };

    // Calculate deltas
    procmetrix_cpu_times_t delta;

    EXPECT_EQ(
        procmetrix_cpu_times_delta(&before, &after, &delta),
        PROCMETRIX_ERROR_NONE);

    EXPECT_FLOAT_EQ(delta.user, expected_diff.user);
    EXPECT_FLOAT_EQ(delta.system, expected_diff.system);
    EXPECT_FLOAT_EQ(delta.idle, expected_diff.idle);
    EXPECT_FLOAT_EQ(delta.nice, expected_diff.nice);
    EXPECT_FLOAT_EQ(delta.iowait, expected_diff.iowait);
    EXPECT_FLOAT_EQ(delta.irq, expected_diff.irq);
    EXPECT_FLOAT_EQ(delta.softirq, expected_diff.softirq);
    EXPECT_FLOAT_EQ(delta.steal, expected_diff.steal);
    EXPECT_FLOAT_EQ(delta.guest, expected_diff.guest);
    EXPECT_FLOAT_EQ(delta.guest_nice, expected_diff.guest_nice);
    EXPECT_FLOAT_EQ(delta.interrupt, expected_diff.interrupt);
    EXPECT_FLOAT_EQ(delta.dpc, expected_diff.dpc);

    // Sum must increase
    double sum_before = procmetrix_cpu_times_sum(&before);
    double sum_after  = procmetrix_cpu_times_sum(&after);

    EXPECT_LT(sum_before, sum_after);

    // Sum of delta must match delta of sum
    double sum_delta = procmetrix_cpu_times_sum(&delta);

    EXPECT_FLOAT_EQ(sum_delta, sum_after - sum_before);

    // Inverting times (negative deltas) clamps at zero
    EXPECT_EQ(
        procmetrix_cpu_times_delta(&after, &before, &delta),
        PROCMETRIX_ERROR_NONE);

    EXPECT_FLOAT_EQ(delta.user, 0.0);
    EXPECT_FLOAT_EQ(delta.system, 0.0);
    EXPECT_FLOAT_EQ(delta.idle, 0.0);
    EXPECT_FLOAT_EQ(delta.nice, 0.0);
    EXPECT_FLOAT_EQ(delta.iowait, 0.0);
    EXPECT_FLOAT_EQ(delta.irq, 0.0);
    EXPECT_FLOAT_EQ(delta.softirq, 0.0);
    EXPECT_FLOAT_EQ(delta.steal, 0.0);
    EXPECT_FLOAT_EQ(delta.guest, 0.0);
    EXPECT_FLOAT_EQ(delta.guest_nice, 0.0);
    EXPECT_FLOAT_EQ(delta.interrupt, 0.0);
    EXPECT_FLOAT_EQ(delta.dpc, 0.0);
}

TEST(procmetrix_cpu_times_sum, null_args)
{
    EXPECT_DOUBLE_EQ(
        procmetrix_cpu_times_sum(nullptr),
        0.0);
}

TEST(procmetrix_cpu_times_sum, delay)
{
    // Note:
    // Must use the time difference of a single cpu
    // Using all would cause the summation to be multiplied by the number of cpus

    // First snapshot
    procmetrix_cpu_times before[1];
    procmetrix_cpu_times_per_cpu(before, 1, nullptr);

    // Wait a bit
    std::this_thread::sleep_for(
        std::chrono::seconds(1));

    // Second snapshot
    procmetrix_cpu_times after[1];
    procmetrix_cpu_times_per_cpu(after, 1, nullptr);

    // Time difference must be near the delay
    procmetrix_cpu_times delta;
    procmetrix_cpu_times_delta(&before[0], &after[0], &delta);

    double observed_delay = procmetrix_cpu_times_sum(&delta);
    EXPECT_NEAR(observed_delay, 1.0, 0.5);

    // Note:
    // sleep_for is not very precise
    // so comparing wall-clock delay with cpu metrics can be flaky
    // lets use a very large tolerance and treat this test a "sanity" only
}

TEST(procmetrix_cpu_utilization_ratio, null_args)
{
    // Zero utilization without crashing
    double cpu_ratio = procmetrix_cpu_utilization_ratio(nullptr);
    EXPECT_EQ(cpu_ratio, 0.0);
}

TEST(procmetrix_cpu_utilization_ratio, div_by_zero)
{
    procmetrix_cpu_times delta {0};

    // Zero utilization without crashing
    double cpu_ratio = procmetrix_cpu_utilization_ratio(&delta);
    EXPECT_EQ(cpu_ratio, 0.0);
}

TEST(procmetrix_cpu_utilization_ratio, delay)
{
    // Get utilization between a time difference
    procmetrix_cpu_times before, after, delta;
    procmetrix_cpu_times_total(&before);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    procmetrix_cpu_times_total(&after);
    procmetrix_cpu_times_delta(&before, &after, &delta);

    // CPU utilization is not zero
    double cpu_ratio = procmetrix_cpu_utilization_ratio(&delta);
    EXPECT_GT(cpu_ratio, 0.0);
    GTEST_LOG_(INFO) << "CPU percentage: " << std::round(cpu_ratio * 100.0); 
}
