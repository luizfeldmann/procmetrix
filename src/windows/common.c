// Lib internals
#include <internal/windows/common_windows_internal.h>

double procmetrix_impl_windows_large_uint_to_secs(const ULARGE_INTEGER* uli)
{
    // each unit is 100 nanoseconds
    return (double)uli->QuadPart / 1.0E7;
}

double procmetrix_impl_windows_large_int_to_secs(const LARGE_INTEGER* sli)
{
    // each unit is 100 nanoseconds
    return (double)sli->QuadPart / 1.0E7;
}

double procmetrix_impl_windows_filetime_to_secs(const FILETIME* pft)
{
    ULARGE_INTEGER uli;
    uli.LowPart = pft->dwLowDateTime;
    uli.HighPart = pft->dwHighDateTime;

    // each unit is 100 nanoseconds
    return procmetrix_impl_windows_large_uint_to_secs(&uli);
}
