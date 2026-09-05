// Lib
#include <procmetrix/cpu.h>

// STD
#include <math.h>

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
