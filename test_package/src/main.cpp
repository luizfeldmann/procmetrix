// Test includes
#include <procmetrix/procmetrix.h>

// Test linking to the library
// Call at least one function from each topic
int main()
{
    // Version must be larger than "0.0.0"
    const auto major = procmetrix_version_major();
    const auto minor = procmetrix_version_minor();
    const auto patch = procmetrix_version_patch();
    if (major + minor + patch <= 0)
        return -1;

    // Computer has at least on CPU
    if (procmetrix_cpu_count_logical() <= 1)
        return -1;
    if (procmetrix_cpu_count_physical() <= 1)
        return -1;

    // Has non-zero physical memory
    procmetrix_virtual_memory_t virtual_memory { 0 };
    procmetrix_error_t status =
        procmetrix_system_virtual_memory(&virtual_memory);

    if (PROCMETRIX_ERROR_NONE != status)
        return -1;

    if (virtual_memory.total <= 1)
        return -1;

    // Own process has a valid PID
    procmetrix_pid_t own_pid = procmetrix_get_pid();
    if (own_pid <= 0)
        return -1;

    // Passed sanity
    return 0;
}
