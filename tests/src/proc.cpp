// Local lib
#include <procmetrix/proc.h>

// Testing
#include <gtest/gtest.h>
#include <gmock/gmock.h>
using namespace ::testing;

// Utils
#include "utils.h"
#include "main.h"

// STD
#include <algorithm>

// Test cases

/** PID exists */

TEST(procmetrix_pid_exists, own_pid)
{
    procmetrix_pid_t own_pid = procmetrix_get_pid();

    ASSERT_NE(own_pid, 0);
    EXPECT_TRUE(procmetrix_pid_exists(own_pid));
}

/** PID list */

TEST(procmetrix_list_pids, null_args)
{
    size_t count = 0;
    procmetrix_pid_t* list = nullptr;

    // Null list
    EXPECT_EQ(
        procmetrix_list_pids(nullptr, &count),
            PROCMETRIX_ERROR_INVALID_ARGUMENT);
    
    // Null count
    EXPECT_EQ(
        procmetrix_list_pids(&list, nullptr),
            PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_list_pids, contains_own_pid)
{
    size_t count = 0;
    procmetrix_pid_t* list = nullptr;

    // Get the list of all PIDs
    EXPECT_EQ(
        procmetrix_list_pids(&list, &count),
            PROCMETRIX_ERROR_NONE);
    
    // Has a valid, non-empty list
    EXPECT_NE(list, nullptr);
    EXPECT_NE(count, 0);

    // My own PID is contained in the list
    procmetrix_pid_t own_pid = procmetrix_get_pid();

    auto itend = std::next(list, count);
    auto itfind = std::find(list, itend, own_pid);

    EXPECT_NE(itfind, itend) 
        << "own pid: " << own_pid << "; pids list size = " << count;

    // Cleanup
    procmetrix_free_pids(&list);
}

/** PID parent */

TEST(procmetrix_get_proc_parent_pid, null_args)
{
    // Invalid input pid
    procmetrix_pid_t parent_pid;
    EXPECT_EQ(
        procmetrix_get_proc_parent_pid(0, &parent_pid),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
    
    // Null output
    procmetrix_pid_t own_pid = procmetrix_get_pid();
    EXPECT_EQ(
        procmetrix_get_proc_parent_pid(own_pid, nullptr),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_get_proc_parent_pid, own_parent_exists)
{
    procmetrix_pid_t parent_pid = 0;
    procmetrix_pid_t own_pid = procmetrix_get_pid();

    // Reads own parent sucesfully
    EXPECT_EQ(
        procmetrix_get_proc_parent_pid(own_pid, &parent_pid),
        PROCMETRIX_ERROR_NONE);

    // Own parent exists
    EXPECT_TRUE(procmetrix_pid_exists(parent_pid));
}

/** Proc name */

TEST(procmetrix_get_proc_name, null_args)
{
    char name[256];

    // PID 0
    EXPECT_EQ(
        procmetrix_get_proc_name(0, name, sizeof(name)),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Null buffer
    procmetrix_pid_t own_pid = procmetrix_get_pid();
    EXPECT_EQ(
        procmetrix_get_proc_name(own_pid, nullptr, 0),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_get_proc_name, own_name)
{
    char name[256];
    procmetrix_pid_t own_pid = procmetrix_get_pid();

    EXPECT_EQ(
        procmetrix_get_proc_name(own_pid, name, sizeof(name)),
        PROCMETRIX_ERROR_NONE);

    #ifdef _WIN32
        EXPECT_STREQ(name, "unitTests.exe");
    #else
        EXPECT_STREQ(name, "unitTests");
    #endif
}

/** Proc exe */

TEST(procmetrix_get_proc_exe, null_args)
{
    char exe_path[1024];

    // PID 0
    EXPECT_EQ(
        procmetrix_get_proc_exe(0, exe_path, sizeof(exe_path)),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Null buffer
    procmetrix_pid_t own_pid = procmetrix_get_pid();
    EXPECT_EQ(
        procmetrix_get_proc_exe(own_pid, nullptr, 0),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_get_proc_exe, own_path)
{
    char exe_path[1024];
    procmetrix_pid_t own_pid = procmetrix_get_pid();

    EXPECT_EQ(
        procmetrix_get_proc_exe(own_pid, exe_path, sizeof(exe_path)),
        PROCMETRIX_ERROR_NONE);

    #ifdef _WIN32
        EXPECT_THAT(exe_path, EndsWith("unitTests.exe"));
    #else
        EXPECT_THAT(exe_path, EndsWith("unitTests"));
    #endif
}

/** Proc working dir */

TEST(procmetrix_get_proc_cwd, null_args)
{
    procmetrix_pid_t own_pid = procmetrix_get_pid();

    // Null output
    EXPECT_EQ(
        procmetrix_get_proc_cwd(own_pid, nullptr, 0),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_get_proc_cwd, own_cwd)
{
    procmetrix_pid_t own_pid = procmetrix_get_pid();
    
    // Read own working directory
    char cwd_path[1024];
    EXPECT_EQ(
        procmetrix_get_proc_cwd(own_pid, cwd_path, sizeof(cwd_path)),
        PROCMETRIX_ERROR_NONE);

    EXPECT_EQ(get_working_dir(), cwd_path);
}

/** Process command line */

TEST(procmetrix_get_proc_cmdline, null_args)
{
    procmetrix_pid_t own_pid = procmetrix_get_pid();
    procmetrix_proc_cmdline_t cmdline { 0 };

    // Zero PID
    EXPECT_EQ(
        procmetrix_get_proc_cmdline(0, &cmdline), 
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    EXPECT_EQ(cmdline.argc, 0);
    EXPECT_EQ(cmdline.argv, nullptr);

    // Null output
    EXPECT_EQ(
        procmetrix_get_proc_cmdline(own_pid, nullptr),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_get_proc_cmdline, own_cmdline)
{
    procmetrix_pid_t own_pid = procmetrix_get_pid();
    procmetrix_proc_cmdline_t cmdline { 0 };

    // Read own command line
    EXPECT_EQ(
        procmetrix_get_proc_cmdline(own_pid, &cmdline),
        PROCMETRIX_ERROR_NONE);

    // Compare to what was received in main()
    const auto mainArgs = getMainArguments();
    EXPECT_EQ(cmdline.argc, mainArgs.size());
    if (cmdline.argc == mainArgs.size()) {
        for (size_t i = 0; i < cmdline.argc; ++i)
            EXPECT_EQ(cmdline.argv[i], mainArgs[i]);
    }
    
    // Cleanup
    procmetrix_free_proc_cmdline(&cmdline);

    // No garbage after cleanup
    EXPECT_EQ(cmdline.argc, 0);
    EXPECT_EQ(cmdline.argv, nullptr);
}

/** Process environment */

TEST(procmetrix_get_proc_environ, null_args)
{
    procmetrix_pid_t own_pid = procmetrix_get_pid();
    procmetrix_proc_environ_t proc_environ { 0 };

    // Zero PID
    EXPECT_EQ(
        procmetrix_get_proc_environ(0, &proc_environ), 
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    EXPECT_EQ(proc_environ.count, 0);
    EXPECT_EQ(proc_environ.vars, nullptr);

    // Null output
    EXPECT_EQ(
        procmetrix_get_proc_environ(own_pid, nullptr), 
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_get_proc_environ, own_env)
{
    procmetrix_pid_t own_pid = procmetrix_get_pid();
    procmetrix_proc_environ_t proc_environ { 0 };

    // Read own environment
    EXPECT_EQ(
        procmetrix_get_proc_environ(own_pid, &proc_environ), 
        PROCMETRIX_ERROR_NONE);

    // Environment is not empty
    EXPECT_GT(proc_environ.count, 0);
    EXPECT_NE(proc_environ.vars, nullptr);

    // Check each var
    if (proc_environ.count != 0 && proc_environ.vars)
    {
        for (size_t i = 0; i < proc_environ.count; ++i)
            EXPECT_STREQ(proc_environ.vars[i].value, std::getenv(proc_environ.vars[i].name));
    }

    // Cleanup
    procmetrix_free_proc_environ(&proc_environ);

    // No garbage after cleanup
    EXPECT_EQ(proc_environ.count, 0);
    EXPECT_EQ(proc_environ.vars, nullptr);
}

/** Process memory */

TEST(procmetrix_get_proc_memory_info, null_args)
{
    // Zero PID
    procmetrix_proc_memory_info_t memory_info { 0 };
    EXPECT_EQ(
        procmetrix_get_proc_memory_info(0, &memory_info), 
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Null output
    procmetrix_pid_t own_pid = procmetrix_get_pid();
    EXPECT_EQ(
        procmetrix_get_proc_memory_info(own_pid, nullptr), 
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_get_proc_memory_info, own_pid)
{
    procmetrix_pid_t own_pid = procmetrix_get_pid();
    procmetrix_proc_memory_info_t memory_info { 0 };

    EXPECT_EQ(
        procmetrix_get_proc_memory_info(own_pid, &memory_info), 
        PROCMETRIX_ERROR_NONE);

    EXPECT_GT(memory_info.vms, 0);
    EXPECT_GT(memory_info.rss, 0);

#ifndef _WIN32
    EXPECT_GE(memory_info.vms, memory_info.rss);
#endif
}

/** CPU times */

TEST(procmetrix_get_proc_cpu_times, null_args)
{
    // Zero PID
    procmetrix_proc_cpu_times_t cpu_times { 0 };
    EXPECT_EQ(
        procmetrix_get_proc_cpu_times(0, &cpu_times),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Null output
    procmetrix_pid_t own_pid = procmetrix_get_pid();
    EXPECT_EQ(
        procmetrix_get_proc_cpu_times(0, nullptr),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_get_proc_cpu_times, own_pid)
{
    // Burn some CPU to ensure non-zero user time
    // even if the unit test process is short lived
    volatile uint64_t x = 0;
    for (uint64_t i = 0; i < 100000000ULL; ++i)
        x += i;

    // Read own process times
    procmetrix_pid_t own_pid = procmetrix_get_pid();
    procmetrix_proc_cpu_times_t cpu_times { 0 };
    EXPECT_EQ(
        procmetrix_get_proc_cpu_times(own_pid, &cpu_times),
        PROCMETRIX_ERROR_NONE);

    // CPU user time is positive
    EXPECT_GT(cpu_times.user, 0.0);
}

TEST(procmetrix_proc_cpu_times_delta, null_args)
{
    procmetrix_proc_cpu_times_t before, after, delta;

    // Null inputs
    EXPECT_EQ(
        procmetrix_proc_cpu_times_delta(nullptr, nullptr, &delta),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // No garbate in the output
    EXPECT_EQ(delta.user, 0.0);
    EXPECT_EQ(delta.system, 0.0);
    EXPECT_EQ(delta.children_user, 0.0);
    EXPECT_EQ(delta.children_system, 0.0);

    // Null output
    EXPECT_EQ(
        procmetrix_proc_cpu_times_delta(&before, &after, nullptr),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_proc_cpu_times_delta, delta)
{
    procmetrix_proc_cpu_times_t const before {
        1, 2, 3, 4
    };
    procmetrix_proc_cpu_times_t const after {
        2, 3, 4, 5
    };

    procmetrix_proc_cpu_times_t delta {0};
    EXPECT_EQ(
        procmetrix_proc_cpu_times_delta(&before, &after, &delta),
        PROCMETRIX_ERROR_NONE);

    EXPECT_DOUBLE_EQ(delta.user, 1.0);
    EXPECT_DOUBLE_EQ(delta.system, 1.0);
    EXPECT_DOUBLE_EQ(delta.children_user, 1.0);
    EXPECT_DOUBLE_EQ(delta.children_system, 1.0);
}

TEST(procmetrix_proc_cpu_times_sum, null_args)
{
    EXPECT_DOUBLE_EQ(procmetrix_proc_cpu_times_sum(nullptr), 0.0);
}

TEST(procmetrix_proc_cpu_times_sum, sum)
{
    procmetrix_proc_cpu_times_t cpu_times {
        2.0, 4.0, 8.0, 16.0
    };
    EXPECT_DOUBLE_EQ(procmetrix_proc_cpu_times_sum(&cpu_times), 6.0);
}
