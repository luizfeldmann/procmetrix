// Tests
#include "utils.h"

// Per-platform
#ifdef _WIN32
    // Windows
    #include <Windows.h>
#elif defined(__linux__)
    // Linux
    #include <linux/limits.h>
    #include <unistd.h>
#elif defined (__FreeBSD__)
    #include <sys/syslimits.h>
    #include <unistd.h>
#else
    #error Unsupported system
#endif

// Impl

std::string get_working_dir()
{
#ifdef _WIN32
    char buf[MAX_PATH];

    DWORD len = GetCurrentDirectoryA(sizeof(buf), buf);

    if (len > 0)
        return buf;
#else
    char buf[PATH_MAX];

    if (getcwd(buf, sizeof(buf)) != NULL)
        return buf;
#endif

    // Fallback
    return {};
}
