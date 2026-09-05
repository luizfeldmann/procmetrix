// Lib
#include <procmetrix/cpu.h>

// Internal
#include <internal/linux/cpu_linux_internal.h>

// STD
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>

// Linux
#include <fcntl.h>
#include <unistd.h>

// Constants

//! Path to the stats file
static const char *g_procstat_file = "/proc/stat";

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

// Impl

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
        if (strncmp(buf, "cpu", 3) != 0 || !isdigit(buf[3]))
        {
            // Not a CPU line, end of CPU lines reached
            break;
        }

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

// Public impl

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
