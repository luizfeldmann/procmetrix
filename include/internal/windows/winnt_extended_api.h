//! @file
//! @brief Definitions for the NT native API
//! @details What <winternl.h> would have given us, declared here instead.
//! @see http://www.geoffchappell.com/studies/windows/win32/ntdll/api/native.htm

#ifndef NTEXTAPI_H
#define NTEXTAPI_H

// Windows
#include <Windows.h>
#include <winternl.h>

// NOLINTBEGIN(*)
typedef struct
{
    ULONG Number;
    ULONG MaxMhz;
    ULONG CurrentMhz;
    ULONG MhzLimit;
    ULONG MaxIdleState;
    ULONG CurrentIdleState;
} PROCMETRIX_PROCESSOR_POWER_INFORMATION;

typedef struct
{
    NTSTATUS ExitStatus;
    PPEB PebBaseAddress;
    ULONG_PTR AffinityMask;
    KPRIORITY BasePriority;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR InheritedFromUniqueProcessId;
} PROCMETRIX_PROCESS_BASIC_INFORMATION;

typedef struct
{
    USHORT Flags;
    USHORT Length;
    ULONG TimeStamp;
    UNICODE_STRING DosPath;
} PROCMETRIX_RTL_DRIVE_LETTER_CURDIR;

typedef struct
{
    BYTE Reserved1[16];
    PVOID Reserved2[5];
    UNICODE_STRING CurrentDirectoryPath;
    PVOID CurrentDirectoryHandle;
    UNICODE_STRING DllPath;
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
    PWSTR Environment;
    ULONG dwX;
    ULONG dwY;
    ULONG dwXSize;
    ULONG dwYSize;
    ULONG dwXCountChars;
    ULONG dwYCountChars;
    ULONG dwFillAttribute;
    ULONG dwFlags;
    ULONG wShowWindow;
    UNICODE_STRING WindowTitle;
    UNICODE_STRING Desktop;
    UNICODE_STRING ShellInfo;
    UNICODE_STRING RuntimeInfo;
    PROCMETRIX_RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[0x20];
    ULONG_PTR volatile EnvironmentSize;
} PROCMETRIX_RTL_USER_PROCESS_PARAMETERS;
// NOLINTEND(*)

#endif // NTEXTAPI_H
