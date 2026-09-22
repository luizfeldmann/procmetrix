// Lib
#include <procmetrix/cpu.h>

size_t procmetrix_cpu_count_physical(void)
{
    // @TODO
    return 0;
}

size_t procmetrix_cpu_count_logical(void)
{
    // @TODO
    return 0;
}

procmetrix_error_t procmetrix_cpu_times_total(procmetrix_cpu_times_t *cpu_times)
{
    // @TODO
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t procmetrix_cpu_times_per_cpu(procmetrix_cpu_times_t *cpu_times, size_t maxCount, size_t *readCount)
{
    // @TODO
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t procmetrix_cpu_freqs(procmetrix_cpu_freq_t *cpu_freqs, size_t max_count, size_t *read_count)
{
    // @TODO
    return PROCMETRIX_NOT_IMPLEMENTED;
}
