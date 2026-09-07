// Lib
#include <procmetrix/cpu.h>

// Internal
#include <internal/linux/cpu_linux_internal.h>
#include <internal/linux/core_topology.h>

// STD
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>

// Linux
#include <fcntl.h>
#include <unistd.h>
#include <linux/limits.h>

// Constants

//! Path to the stats file
static const char *g_procstat_file = "/proc/stat";

//! Path to the cpu info file
static const char *g_proccpuinfo_file = "/proc/cpuinfo";

// Internal impl

//! Jiffies counters read from /proc/stat
typedef struct proc_stat
{
    uint64_t user;
    uint64_t nice;
    uint64_t system;
    uint64_t idle;
    uint64_t iowait;
    uint64_t irq;
    uint64_t softirq;
    uint64_t steal;
    uint64_t guest;
    uint64_t guest_nice;
} proc_stat_t;

//! Convert jiffies to seconds and fill the cpu_times structure
static void jiffies_to_seconds(const proc_stat_t *stat, procmetrix_cpu_times_t *cpu_times)
{
    long ticks_per_second = sysconf(_SC_CLK_TCK);
    if (ticks_per_second <= 0)
        return;

    cpu_times->user = (double)stat->user / ticks_per_second;
    cpu_times->nice = (double)stat->nice / ticks_per_second;
    cpu_times->system = (double)stat->system / ticks_per_second;
    cpu_times->idle = (double)stat->idle / ticks_per_second;
    cpu_times->iowait = (double)stat->iowait / ticks_per_second;
    cpu_times->irq = (double)stat->irq / ticks_per_second;
    cpu_times->softirq = (double)stat->softirq / ticks_per_second;
    cpu_times->steal = (double)stat->steal / ticks_per_second;
    cpu_times->guest = (double)stat->guest / ticks_per_second;
    cpu_times->guest_nice = (double)stat->guest_nice / ticks_per_second;
}

//! Opens a file as readonly and close on exec
static FILE *linux_open_file_rdonly_cloexec(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return NULL;

#ifdef FD_CLOEXEC
    fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif

    FILE *fp = fdopen(fd, "r");
    if (fp == NULL)
        close(fd);

    return fp;
}

//! Checks if a line is "cpuX" format
static int scan_procstat_cpux_line(const char *line)
{
    unsigned u = 0;
    return sscanf(line, "cpu%u", &u);
}

//! Checks if a lines is "processor : X" format
static int scan_cpuinfo_processor_line(const char *line)
{
    unsigned u = 0;
    return sscanf(line, "processor : %u", &u);
}

procmetrix_error_t read_sysfs_cpu_scaling_freq_field(const char *base_path, const char *field_name, uint64_t *field_value)
{
    // Sanity
    if (NULL == base_path || NULL == field_name || NULL == field_value)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Format path to field
    char scaling_freq_field_path[PATH_MAX];
    snprintf(scaling_freq_field_path, sizeof(scaling_freq_field_path), "%s/%s", base_path, field_name);

    // Open field file
    FILE *scaling_freq_field_file = linux_open_file_rdonly_cloexec(scaling_freq_field_path);
    if (NULL == scaling_freq_field_file)
        return PROCMETRIX_ERROR_FILE_READ;

    // Read the field value
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;
    if (1 != fscanf(scaling_freq_field_file, "%" SCNu64, field_value))
    {
        status = PROCMETRIX_ERROR_MALFORMED;
        *field_value = 0;
    }

    // Cleanup
    fclose(scaling_freq_field_file);

    return status;
}

// Impl

size_t procmetrix_impl_linux_cpu_count_physical_cpuinfo(FILE *cpuinfo_file)
{
    // Sanity
    if (NULL == cpuinfo_file)
        return 0;

    // List pairs of core id and package id
    procmetrix_core_topo_list_t list;
    procmetrix_core_topo_list_init(&list);

    // State variables
    size_t physical_id = 0;
    bool has_physical_id = false;

    size_t core_id = 0;
    bool has_core_id = false;

    // Iterate the file lines
    char line[1024];
    while (NULL != fgets(line, sizeof(line), cpuinfo_file))
    {
        size_t value = 0;

        if (1 == sscanf(line, "physical id : %zu", &value))
        {
            has_physical_id = true;
            physical_id = value;
        }
        else if (1 == sscanf(line, "core id : %zu", &value))
        {
            has_core_id = true;
            core_id = value;
        }

        // When both values are found, append to list
        if (has_core_id && has_physical_id)
        {
            procmetrix_core_topo_list_add(&list, physical_id, core_id);
            has_core_id = false;
            has_physical_id = false;
        }
    }

    // Count unique entries
    procmetrix_core_topo_list_sort(&list);
    size_t physical_count = procmetrix_core_topo_list_count_unique(&list);

    // Cleanup
    procmetrix_core_topo_list_free(&list);

    return physical_count;
}

size_t procmetrix_impl_linux_cpu_count_physical_topology(const glob_t *sysfs_cpus)
{
    // Sanity
    if (NULL == sysfs_cpus)
        return 0;

    // Iterate the CPU topology files nad list core keys
    procmetrix_core_topo_list_t list;
    procmetrix_core_topo_list_init(&list);

    for (size_t i = 0; i < sysfs_cpus->gl_pathc; i++)
    {
        // Format path to files
        char core_id_path[PATH_MAX];
        char package_id_path[PATH_MAX];

        snprintf(core_id_path, sizeof(core_id_path), "%s/topology/core_id", sysfs_cpus->gl_pathv[i]);
        snprintf(package_id_path, sizeof(package_id_path), "%s/topology/physical_package_id", sysfs_cpus->gl_pathv[i]);

        // Open the files
        FILE *core_id_file = linux_open_file_rdonly_cloexec(core_id_path);
        FILE *package_id_file = linux_open_file_rdonly_cloexec(package_id_path);

        // Parse the files
        if (NULL != core_id_file && NULL != package_id_file)
        {
            size_t core_id;
            size_t package_id;

            // Add key to the list
            if (1 == fscanf(core_id_file, "%zu", &core_id) && 1 == fscanf(package_id_file, "%zu", &package_id))
                procmetrix_core_topo_list_add(&list, package_id, core_id);
        }

        // Cleanup
        if (NULL != core_id_file)
            fclose(core_id_file);

        if (NULL != package_id_file)
            fclose(package_id_file);
    }

    // Count unique entries
    procmetrix_core_topo_list_sort(&list);
    size_t physical_count = procmetrix_core_topo_list_count_unique(&list);

    // Cleanup
    procmetrix_core_topo_list_free(&list);

    return physical_count;
}

size_t procmetrix_impl_linux_cpuinfo_count_processors(FILE *cpuinfo_file)
{
    // Sanity
    if (NULL == cpuinfo_file)
        return 0;

    // Count lines
    size_t processor_count = 0;

    char line[1024];
    while (NULL != fgets(line, sizeof(line), cpuinfo_file))
        if (0 != scan_cpuinfo_processor_line(line))
            ++processor_count;
    return processor_count;
}

size_t procmetrix_impl_linux_procstat_count_cpus(FILE *stat_file)
{
    // Sanity
    if (NULL == stat_file)
        return 0;

    // Skip first line (aggregate CPU times)
    char line[1024];
    if (NULL == fgets(line, sizeof(line), stat_file))
        return 0;

    // Read all the cpuX lines
    size_t cpu_count = 0;
    while (NULL != fgets(line, sizeof(line), stat_file))
    {
        if (0 == scan_procstat_cpux_line(line))
            break;
        ++cpu_count;
    }

    return cpu_count;
}

procmetrix_error_t procmetrix_impl_linux_cpu_times_total(FILE *stat_file, procmetrix_cpu_times_t *cpu_times)
{
    // Sanity check
    if (NULL == cpu_times || NULL == stat_file)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    memset(cpu_times, 0, sizeof(procmetrix_cpu_times_t));

    // Read the first line of the file
    proc_stat_t stat = {0};
    int scanCount = fscanf(stat_file, "cpu %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64,
                           // Always present
                           &stat.user, &stat.nice, &stat.system, &stat.idle,
                           // Linux >= 2.5.41
                           &stat.iowait,
                           // Linux >= 2.6.0
                           &stat.irq,
                           &stat.softirq,
                           // Linux >= 2.6.11
                           &stat.steal,
                           // Linux >= 2.6.24
                           &stat.guest,
                           // Linux >= 2.6.33
                           &stat.guest_nice);

    if (scanCount < 4)
    {
        // Unable to read the minimum fields
        return PROCMETRIX_ERROR_MALFORMED;
    }

    // Convert the units
    jiffies_to_seconds(&stat, cpu_times);

    return PROCMETRIX_ERROR_NONE;
}

procmetrix_error_t procmetrix_impl_linux_cpu_times_per_cpu(FILE *stat_file, procmetrix_cpu_times_t *cpu_times, size_t max_count, size_t *read_count)
{
    // Sanity check
    if (NULL == cpu_times || NULL == stat_file)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    if (0 == max_count)
        return PROCMETRIX_ERROR_MORE_DATA;

    // Zero the output array
    memset(cpu_times, 0, sizeof(procmetrix_cpu_times_t) * max_count);

    if (read_count != NULL)
        *read_count = 0;

    // Skip first line (aggregate CPU times)
    char buf[1024];
    if (NULL == fgets(buf, sizeof(buf), stat_file))
    {
        // Unable to read first line, malformed file
        return PROCMETRIX_ERROR_MALFORMED;
    }

    // Read each line of the file, looking for lines that start with "cpuN"
    size_t cpu_index = 0;
    while (NULL != fgets(buf, sizeof(buf), stat_file))
    {
        // Check if the line starts with "cpu" followed by a number
        // If not a CPU line, then end of CPU lines was reached
        if (0 == scan_procstat_cpux_line(buf))
            break;

        // Read the CPU times from the line
        proc_stat_t stat = {0};
        int scanCount = sscanf(buf, "cpu%*d %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64,
                               // Always present
                               &stat.user, &stat.nice, &stat.system, &stat.idle,
                               // Linux kernel version depends
                               &stat.iowait, &stat.irq, &stat.softirq, &stat.steal, &stat.guest, &stat.guest_nice);

        // Unable to read the minimum fields, malformed file
        if (scanCount < 4)
            return PROCMETRIX_ERROR_MALFORMED;

        // Check if there is space in the output array
        if (cpu_index >= max_count)
            return PROCMETRIX_ERROR_MORE_DATA;

        // Convert the units and store in the output array
        jiffies_to_seconds(&stat, &cpu_times[cpu_index]);

        // Store current read count
        cpu_index++;
        if (read_count != NULL)
            *read_count = cpu_index;
    }

    return PROCMETRIX_ERROR_NONE;
}

procmetrix_error_t procmetrix_impl_linux_cpu_freqs_policies(const glob_t *sysfs_policies, procmetrix_cpu_freq_t *cpu_freqs, size_t max_count, size_t *read_count)
{
    // Sanity
    if (NULL == sysfs_policies || NULL == cpu_freqs || 0 == max_count)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Defensively clear garbage
    memset(cpu_freqs, 0, max_count * sizeof(procmetrix_cpu_freq_t));
    if (NULL != read_count)
        *read_count = 0;

    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;

    // Read each policy
    for (size_t i = 0; i < sysfs_policies->gl_pathc; ++i)
    {
        // Open list of affected CPUs
        char affected_cpus_path[PATH_MAX];
        snprintf(affected_cpus_path, sizeof(affected_cpus_path), "%s/affected_cpus", sysfs_policies->gl_pathv[i]);

        FILE *affected_cpus_file = linux_open_file_rdonly_cloexec(affected_cpus_path);
        if (NULL == affected_cpus_file)
        {
            // Cannot proceed because output could be filled in non-contiguous chunks
            // Say, information was read for cpus 0, 2, but missing for 1, 4
            status = PROCMETRIX_ERROR_FILE_READ;
            break;
        }

        // Read the fields
        // Don't produce a global error if we fail to read the fields for some cpus, we'll keep zero there
        // Having incomplete frequency data is common
        uint64_t scaling_cur_freq = 0, scaling_max_freq = 0, scaling_min_freq = 0;
        (void)read_sysfs_cpu_scaling_freq_field(sysfs_policies->gl_pathv[i], "scaling_cur_freq", &scaling_cur_freq);
        (void)read_sysfs_cpu_scaling_freq_field(sysfs_policies->gl_pathv[i], "scaling_max_freq", &scaling_max_freq);
        (void)read_sysfs_cpu_scaling_freq_field(sysfs_policies->gl_pathv[i], "scaling_min_freq", &scaling_min_freq);

        // For each affected CPU
        size_t affected_cpu = 0;
        while (1 == fscanf(affected_cpus_file, "%zu", &affected_cpu))
        {
            // Check if CPU index is in range of the array
            if (affected_cpu >= max_count)
            {
                status = PROCMETRIX_ERROR_MORE_DATA;
                break;
            }

            // Must use "max()" function because the list may be out of order
            if (NULL != read_count && *read_count < affected_cpu + 1)
                *read_count = affected_cpu + 1;

            // Copy data to the item, convert from KHz to MHz
            cpu_freqs[affected_cpu].freq_cur = (double)scaling_cur_freq / 1000.0;
            cpu_freqs[affected_cpu].freq_max = (double)scaling_max_freq / 1000.0;
            cpu_freqs[affected_cpu].freq_min = (double)scaling_min_freq / 1000.0;
        }

        // Cleanup
        fclose(affected_cpus_file);
    }

    return status;
}

procmetrix_error_t procmetrix_impl_linux_cpuinfo_freqs(FILE *cpuinfo_file, procmetrix_cpu_freq_t *cpu_freqs, size_t max_count, size_t *read_count)
{
    // Sanity
    if (NULL == cpuinfo_file || NULL == cpu_freqs || 0 == max_count)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Defensively clear garbage
    memset(cpu_freqs, 0, max_count * sizeof(procmetrix_cpu_freq_t));
    if (NULL != read_count)
        *read_count = 0;

    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;

    // Read file lines
    char line[1024];
    size_t cpu_index = 0;
    while (NULL != fgets(line, sizeof(line), cpuinfo_file))
    {
        // x86 says "cpu MHz", ppc "clock" (with a MHz suffix),
        // s390x "cpu MHz dynamic" plus a "static" one we skip.
        double value = 0.0;
        if (1 == sscanf(line, "cpu MHz : %lf", &value) ||
            1 == sscanf(line, "clock : %lf MHz", &value) ||
            1 == sscanf(line, "cpu MHz dynamic : %lf", &value))
        {
            // Check if we can fit the output
            if (cpu_index >= max_count)
            {
                status = PROCMETRIX_ERROR_MORE_DATA;
                break;
            }

            // Store the current value
            cpu_freqs[cpu_index++].freq_cur = value;
        }
    }

    // Total read items
    if (NULL != read_count)
        *read_count = cpu_index;

    return status;
}

// Public impl

size_t procmetrix_cpu_count_physical(void)
{
    // Glob cpu topology from sysfs
    glob_t g;
    if (0 == glob("/sys/devices/system/cpu/cpu[0-9]*", 0, NULL, &g))
    {
        size_t physical_count = procmetrix_impl_linux_cpu_count_physical_topology(&g);

        // Cleanup
        globfree(&g);

        if (physical_count > 0)
            return physical_count;
    }

    // Fallback:
    // Read "physical id" and "core id" from /proc/cpuinfo
    FILE *cpuinfo_file = linux_open_file_rdonly_cloexec(g_proccpuinfo_file);
    if (NULL != cpuinfo_file)
    {
        size_t physical_count = procmetrix_impl_linux_cpu_count_physical_cpuinfo(cpuinfo_file);

        // Cleanup
        fclose(cpuinfo_file);

        // Success
        if (physical_count > 0)
            return physical_count;
    }

    // Unable to determine
    return 0;
}

size_t procmetrix_cpu_count_logical(void)
{
    // Primary implementation
    long nproc = sysconf(_SC_NPROCESSORS_ONLN);
    if (nproc > 0)
        return nproc;

    // Fallback:
    // Count "processor: X" lines in cpuinfo
    FILE *cpuinfo_file = linux_open_file_rdonly_cloexec(g_proccpuinfo_file);
    if (NULL != cpuinfo_file)
    {
        // Parse counting lines
        nproc = procmetrix_impl_linux_cpuinfo_count_processors(cpuinfo_file);

        // Cleanup
        fclose(cpuinfo_file);

        // Success
        if (nproc > 0)
            return nproc;
    }

    // Fallback:
    // Count "cpuX" lines in procstat
    FILE *procstat_file = linux_open_file_rdonly_cloexec(g_procstat_file);
    if (NULL != procstat_file)
    {
        // Parse counting lines
        nproc = procmetrix_impl_linux_procstat_count_cpus(cpuinfo_file);

        // Cleanup
        fclose(procstat_file);

        // Success
        if (nproc > 0)
            return nproc;
    }

    // Unable to determine
    return 0;
}

procmetrix_error_t procmetrix_cpu_times_total(procmetrix_cpu_times_t *cpu_times)
{
    // Open proc stat
    FILE *stat_file = linux_open_file_rdonly_cloexec(g_procstat_file);
    if (stat_file == NULL)
        return PROCMETRIX_ERROR_FILE_READ;

    // Parse the file
    procmetrix_error_t result = procmetrix_impl_linux_cpu_times_total(stat_file, cpu_times);

    // Close the file
    fclose(stat_file);

    return result;
}

procmetrix_error_t procmetrix_cpu_times_per_cpu(procmetrix_cpu_times_t *cpu_times, size_t maxCount, size_t *readCount)
{
    // Open proc stat
    FILE *stat_file = linux_open_file_rdonly_cloexec(g_procstat_file);
    if (stat_file == NULL)
        return PROCMETRIX_ERROR_FILE_READ;

    // Call the implementation
    procmetrix_error_t result = procmetrix_impl_linux_cpu_times_per_cpu(stat_file, cpu_times, maxCount, readCount);

    // Close the file
    fclose(stat_file);

    return result;
}

procmetrix_error_t procmetrix_cpu_freqs(procmetrix_cpu_freq_t *cpu_freqs, size_t max_count, size_t *read_count)
{
    // Sanity
    if (NULL == cpu_freqs || 0 == max_count)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Glob CPU freq policies from sysfs
    glob_t g;
    if (0 == glob("/sys/devices/system/cpu/cpufreq/policy[0-9]*", 0, NULL, &g))
    {
        procmetrix_error_t result = procmetrix_impl_linux_cpu_freqs_policies(&g, cpu_freqs, max_count, read_count);

        // Cleanup
        globfree(&g);

        return result;
    }

    // Fallback:
    // Read from /proc/cpuinfo
    // Only current value will be read, while min and max will be zeroed
    FILE *cpuinfo_file = linux_open_file_rdonly_cloexec(g_proccpuinfo_file);
    if (NULL != cpuinfo_file)
    {
        // Parse counting lines
        procmetrix_error_t result = procmetrix_impl_linux_cpuinfo_freqs(cpuinfo_file, cpu_freqs, max_count, read_count);

        // Cleanup
        fclose(cpuinfo_file);

        return result;
    }

    // Unable to read policies
    return PROCMETRIX_ERROR_FILE_READ;
}
