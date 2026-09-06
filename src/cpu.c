// Lib
#include <procmetrix/cpu.h>

// STD
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Public impl

procmetrix_error_t procmetrix_cpu_times_delta(const procmetrix_cpu_times_t *before, const procmetrix_cpu_times_t *after, procmetrix_cpu_times_t *delta)
{
    // Sanity
    if (NULL == before || NULL == after || NULL == delta)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Diff every field
    delta->user =
        fmax(0.0, after->user - before->user);

    delta->system =
        fmax(0.0, after->system - before->system);

    delta->idle =
        fmax(0.0, after->idle - before->idle);

    delta->nice =
        fmax(0.0, after->nice - before->nice);

    delta->iowait =
        fmax(0.0, after->iowait - before->iowait);

    delta->irq =
        fmax(0.0, after->irq - before->irq);

    delta->softirq =
        fmax(0.0, after->softirq - before->softirq);

    delta->steal =
        fmax(0.0, after->steal - before->steal);

    delta->guest =
        fmax(0.0, after->guest - before->guest);

    delta->guest_nice =
        fmax(0.0, after->guest_nice - before->guest_nice);

    delta->interrupt =
        fmax(0.0, after->interrupt - before->interrupt);

    delta->dpc =
        fmax(0.0, after->dpc - before->dpc);

    // Ok
    return PROCMETRIX_ERROR_NONE;
}

double procmetrix_cpu_times_sum(const procmetrix_cpu_times_t *cpu_times)
{
    // Sanity
    if (NULL == cpu_times)
        return 0.0;

    // Total of all CPU times
    double total =
        cpu_times->user +
        cpu_times->system +
        cpu_times->idle +
        cpu_times->nice +
        cpu_times->iowait +
        cpu_times->irq +
        cpu_times->softirq +
        cpu_times->steal +
        cpu_times->interrupt +
        cpu_times->dpc;

    // Linux already accounts "guest" time inside "user" time and
    // "guest_nice" time inside "nice" time, so these values
    // must not be added again to the total.
    // For other systems these must be added explicitly.
#ifndef __linux__
    total += cpu_times->guest;
    total += cpu_times->guest_nice;
#endif // __linux__

    return total;
}

double procmetrix_cpu_utilization_ratio(const procmetrix_cpu_times_t *delta)
{
    // Sanity
    if (NULL == delta)
        return 0.0;

    // Total of all CPU times
    double total = procmetrix_cpu_times_sum(delta);

    // Avoid division by zero
    if (total <= 0.0)
        return 0.0;

    // Busy time percentage
    double idle =
        delta->idle +
        delta->iowait;

    double percentage = (total - idle) / total;

    // Enforce bounds
    return fmax(0.0, fmin(percentage, 1.0));
}

procmetrix_error_t procmetrix_cpu_freqs_average(const procmetrix_cpu_freq_t *cpu_freqs, size_t count, procmetrix_cpu_freq_t *average)
{
    // Sanity
    if (NULL == cpu_freqs || NULL == average)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Give a sensible result regardless of error validation below
    memset(average, 0, sizeof(procmetrix_cpu_freq_t));

    // Avoid division by zero on empty list
    if (0 == count)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Sum totals
    for (size_t i = 0; i < count; ++i)
    {
        average->freq_cur += cpu_freqs[i].freq_cur;
        average->freq_min += cpu_freqs[i].freq_min;
        average->freq_max += cpu_freqs[i].freq_max;
    }

    // Get the average
    average->freq_cur /= count;
    average->freq_min /= count;
    average->freq_max /= count;

    return PROCMETRIX_ERROR_NONE;
}

procmetrix_error_t procmetrix_cpu_freq_system(procmetrix_cpu_freq_t *system_freq)
{
    // Sanity
    if (NULL == system_freq)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Avoid undefined result even in case of errors
    memset(system_freq, 0, sizeof(procmetrix_cpu_freq_t));

    // Allocate an array element per logical CPU
    size_t ncpus = procmetrix_cpu_count_logical();
    if (0 == ncpus)
        return PROCMETRIX_ERROR_UNKNOWN;

    procmetrix_cpu_freq_t *cpu_freqs =
        (procmetrix_cpu_freq_t *)malloc(sizeof(procmetrix_cpu_freq_t) * ncpus);

    if (NULL == cpu_freqs)
        return PROCMETRIX_ERROR_OUT_OF_MEMORY;

    // Read the frequencies
    size_t read_count = 0;

    procmetrix_error_t status = procmetrix_cpu_freqs(cpu_freqs, ncpus, &read_count);

    if (PROCMETRIX_ERROR_NONE == status || PROCMETRIX_ERROR_MORE_DATA == status)
    {
        if (0 == read_count)
            status = PROCMETRIX_ERROR_UNKNOWN;
        else
            status = procmetrix_cpu_freqs_average(cpu_freqs, read_count, system_freq);
    }

    // Cleanup
    free(cpu_freqs);

    return status;
}
