// Lib internals
#include <internal/windows/common_windows_internal.h>

double procmetrix_impl_windows_large_uint_to_secs(const ULARGE_INTEGER* li)
{
    // each unit is 100 nanoseconds
    return (double)li->QuadPart / 1.0E7;
}

double procmetrix_impl_windows_large_int_to_secs(const LARGE_INTEGER* li)
{
    // each unit is 100 nanoseconds
    return (double)li->QuadPart / 1.0E7;
}

double procmetrix_impl_windows_filetime_to_secs(const FILETIME* pft)
{
    ULARGE_INTEGER v;
    v.LowPart = pft->dwLowDateTime;
    v.HighPart = pft->dwHighDateTime;

    // each unit is 100 nanoseconds
    return procmetrix_impl_windows_large_uint_to_secs(&v);
}
