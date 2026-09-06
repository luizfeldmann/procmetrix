#ifndef _PROCMETRIX_CPU_H_
#define _PROCMETRIX_CPU_H_

// Lib
#include <procmetrix/error.h>

// STD
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    //! Gets the number of physical CPU cores.
    //! @return Zero if undetermined or error.
    PROCMETRIX_API size_t procmetrix_cpu_count_physical(void);

    //! Gets the number of local CPUs, i.e. the number of physical cores multiplied by the number of threads that can run on each core
    //! @return Zero if undetermined or error.
    PROCMETRIX_API size_t procmetrix_cpu_count_logical(void);

    //! Every attribute represents the seconds the CPU has spent in the given mode.
    typedef struct procmetrix_cpu_times
    {
        //! Time spent by normal processes executing in user mode.
        //! On Linux this also includes guest time.
        double user;

        //! Time spent by processes executing in kernel mode.
        double system;

        //! Time spent doing nothing.
        double idle;

        //! Time spent by niced (prioritized) processes executing in user mode.
        //! On Linux this also includes guest_nice time.
        double nice;

        //! Time spent waiting for I/O to complete.
        //! This is not accounted in idle time counter.
        double iowait;

        //! Time spent for servicing hardware interrupts.
        //! (Linux, BSD).
        double irq;

        //! Time spent for servicing software interrupts.
        //! (Linux).
        double softirq;

        //! Time spent by other operating systems running in a virtualized environment
        //! (Linux 2.6.11+).
        double steal;

        //! Time spent running a virtual CPU for guest operating systems under the control of the Linux kernel
        //! (Linux 2.6.24+).
        double guest;

        //! Time spent running a niced guest.
        //! Virtual CPU for guest operating systems under the control of the Linux kernel.
        //! (Linux 3.2.0+).
        double guest_nice;

        //! Time spent for servicing hardware interrupts.
        //! (Windows).
        double interrupt;

        //! Time spent servicing deferred procedure calls (DPCs).
        //! DPCs are interrupts that run at a lower priority than standard interrupts.
        //! (Windows).
        double dpc;
    } procmetrix_cpu_times_t;

    //! Return system total CPU times.
    //! @param[out] cpu_times Structure that will be filled with the CPU times.
    PROCMETRIX_API procmetrix_error_t procmetrix_cpu_times_total(procmetrix_cpu_times_t *cpu_times);

    //! Returns the CPU times for each logical CPU in the system.
    //! The order of the list is consistent across calls.
    //! @param[out] cpu_times Array of structures that will be filled with the CPU times.
    //! @param[in] max_count Number of elements allocated in the array.
    //! @param[out] read_count Number of elements actually read (optional, can be NULL).
    PROCMETRIX_API procmetrix_error_t procmetrix_cpu_times_per_cpu(procmetrix_cpu_times_t *cpu_times, size_t max_count, size_t *read_count);

    //! Calculates the delta time between two time counters snapshots.
    //! @details Negative time deltas are clamped at zero.
    //! @param[in] before Previous CPU times snapshot.
    //! @param[in] after Current CPU times snapshot.
    //! @param[out] delta Receives the CPU times with the time difference.
    PROCMETRIX_API procmetrix_error_t procmetrix_cpu_times_delta(const procmetrix_cpu_times_t *before, const procmetrix_cpu_times_t *after, procmetrix_cpu_times_t *delta);

    //! Calculates the sum of all CPU times.
    PROCMETRIX_API double procmetrix_cpu_times_sum(const procmetrix_cpu_times_t *cpu_times);

    //! Calculates the utilization ratio of the CPU from the given time difference.
    //! @param[in] delta A CPU times difference computed by #procmetrix_cpu_times_delta
    //! @return A ratio in range [0 .. 1].
    //!         Where 0.0 means fully idle and 1.0 means fully busy.
    PROCMETRIX_API double procmetrix_cpu_utilization_ratio(const procmetrix_cpu_times_t *delta);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PROCMETRIX_CPU_H_
