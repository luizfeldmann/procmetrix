// Lib
#include <procmetrix/proc.h>

// Windows
#include <Windows.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <shlwapi.h>
#include <shellapi.h>
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "Shlwapi.lib")

// Utils

//! Doesnt seem to be properly defined in the headers
typedef struct
{
    NTSTATUS ExitStatus;
    PPEB PebBaseAddress;
    ULONG_PTR AffinityMask;
    KPRIORITY BasePriority;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR InheritedFromUniqueProcessId;
} PBI_t;

typedef struct
{
    BYTE Reserved1[16];
    PVOID Reserved2[5];
    UNICODE_STRING CurrentDirectoryPath;
    PVOID CurrentDirectoryHandle;
    UNICODE_STRING DllPath;
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
    LPCWSTR env;
} RUPP_t;

//! Finds the PROCESSENTRY32 associated with a PID
static procmetrix_error_t procmetrix_get_process_entry(procmetrix_pid_t pid, PROCESSENTRY32 *foundproc)
{
    // List of all processes
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == snapshot)
        return PROCMETRIX_ERROR_UNKNOWN;

    // Iterate all processes
    procmetrix_error_t status = PROCMETRIX_ERROR_INVALID_ARGUMENT;

    PROCESSENTRY32 pe = {0};
    pe.dwSize = sizeof(pe);
    if (Process32First(snapshot, &pe))
    {
        do
        {
            // Try to find this PID in the list
            if (pe.th32ProcessID == pid)
            {
                status = PROCMETRIX_ERROR_NONE;
                *foundproc = pe;
                break;
            }
        } while (Process32Next(snapshot, &pe));
    }

    // Cleanup
    CloseHandle(snapshot);

    return status;
}

//! Checks if a given PID exists in the list of all PIDs
static bool procmetrix_pid_in_pids_list(procmetrix_pid_t pid)
{
    // Check in the list of processes
    PROCESSENTRY32 pe = {0};
    return PROCMETRIX_ERROR_NONE == procmetrix_get_process_entry(pid, &pe);
}

static procmetrix_error_t procmetric_read_process_param(HANDLE hProcess, wchar_t **data, const UNICODE_STRING *src)
{
    // Sanity (1)
    if (NULL == data)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    *data = NULL; // robustness

    // Sanity (2)
    if (NULL == hProcess || NULL == src || NULL == src->Buffer)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Alloc space for the result
    *data = malloc(src->Length + sizeof(wchar_t));
    if (NULL == *data)
        return PROCMETRIX_ERROR_OUT_OF_MEMORY;

    // Read the memory
    if (!ReadProcessMemory(hProcess, src->Buffer, *data, src->Length, NULL))
    {
        // Cleanup
        free(*data);
        *data = NULL;

        return PROCMETRIX_ERROR_UNKNOWN;
    }

    // Ensure safe zero termination
    (*data)[src->Length / sizeof(wchar_t)] = L'\0';

    return PROCMETRIX_ERROR_NONE;
}

//! Reads params from a process's memory
static procmetrix_error_t procmetric_read_process_params(procmetrix_pid_t pid, wchar_t **cwd, wchar_t **cli, wchar_t **env)
{
    // Open the process
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (NULL == hProcess)
        return PROCMETRIX_ERROR_UNKNOWN;

    // Read basic info
    PROCESS_BASIC_INFORMATION pbi;
    if (!NT_SUCCESS(NtQueryInformationProcess(
            hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), NULL)))
    {
        CloseHandle(hProcess);
        return PROCMETRIX_ERROR_UNKNOWN;
    }

    // Read the PEB
    PEB peb = {0};
    if (!ReadProcessMemory(hProcess, pbi.PebBaseAddress, &peb, sizeof(PEB), NULL))
    {
        CloseHandle(hProcess);
        return PROCMETRIX_ERROR_UNKNOWN;
    }

    // Read process params
    RUPP_t proc_parameters;
    if (!ReadProcessMemory(hProcess, peb.ProcessParameters, &proc_parameters, sizeof(proc_parameters), NULL))
    {
        CloseHandle(hProcess);
        return PROCMETRIX_ERROR_UNKNOWN;
    }

    // Read one of the desired params
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;

    if (NULL != cwd)
    {
        status = procmetric_read_process_param(hProcess, cwd, &proc_parameters.CurrentDirectoryPath);
    }
    else if (NULL != cli)
    {
        status = procmetric_read_process_param(hProcess, cli, &proc_parameters.CommandLine);
    }

    // Cleanup
    CloseHandle(hProcess);

    return status;
}

// Impl

procmetrix_pid_t procmetrix_get_pid()
{
    return GetCurrentProcessId();
}

bool procmetrix_pid_exists(procmetrix_pid_t pid)
{
    // Special case for PID 0 "System Idle Process"
    if (pid == 0)
        return true;

    bool exists = false;

    // Try to open the process directly
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (NULL != hProcess)
    {
        // We need to check if the process is really running or has already exited
        DWORD exit_code = 0;
        if (GetExitCodeProcess(hProcess, &exit_code))
        {
            // If the process is still running, then it exists
            exists = (STILL_ACTIVE == exit_code);
        }
        else if (ERROR_ACCESS_DENIED == GetLastError())
        {
            // Access denied means it exists, we just can't access it
            exists = true;
        }
        else
        {
            // Fallback to checking of the process exists in the PID list
            exists = procmetrix_pid_in_pids_list(pid);
        }

        CloseHandle(hProcess);
    }
    else if (ERROR_INVALID_PARAMETER != GetLastError())
    {
        // ERROR_INVALID_PARAMETER means "no such process"
        // Otherwise we can't know for sure, and need to use the fallback
        exists = procmetrix_pid_in_pids_list(pid);
    }

    return exists;
}

procmetrix_error_t procmetrix_list_pids(procmetrix_pid_t **list, size_t *out_count)
{
    // Sanity
    if (list == NULL || *list != NULL || out_count == NULL)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Don't let this keep garbage
    *out_count = 0;

    // Snapshot of all processes
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == snapshot)
        return PROCMETRIX_ERROR_UNKNOWN;

    // Count how many in the list
    size_t num_procs = 0;
    PROCESSENTRY32 pe = {0};
    pe.dwSize = sizeof(pe);

    if (Process32First(snapshot, &pe))
    {
        ++num_procs;
        while (Process32Next(snapshot, &pe))
            ++num_procs;
    }

    // Allocate size of items in the list
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;

    procmetrix_pid_t *pids = (procmetrix_pid_t *)calloc(num_procs, sizeof(procmetrix_pid_t));
    if (NULL == (*list = pids))
        status = PROCMETRIX_ERROR_OUT_OF_MEMORY;
    else
    {
        pe.dwSize = sizeof(pe);
        if (Process32First(snapshot, &pe))
        {
            do
            {
                pids[*out_count] = pe.th32ProcessID;
                ++(*out_count);
            } while (Process32Next(snapshot, &pe));
        }
    }

    // Cleanup
    CloseHandle(snapshot);

    return status;
}

procmetrix_error_t procmetrix_get_proc_parent_pid(procmetrix_pid_t pid, procmetrix_pid_t *ppid)
{
    // Sanity
    if (NULL == ppid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // In Windows the PID 0 exists but it has no parent...
    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Defensive result
    *ppid = 0;
    procmetrix_error_t status = PROCMETRIX_ERROR_UNKNOWN;

    // Try to open the process directly
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (NULL != hProcess)
    {
        PBI_t pbi = {0};
        if (NT_SUCCESS(NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), NULL)))
        {
            *ppid = pbi.InheritedFromUniqueProcessId;
            status = PROCMETRIX_ERROR_NONE;
        }

        CloseHandle(hProcess);
    }
    else if (ERROR_INVALID_PARAMETER != GetLastError())
    {
        // Maybe just permission error,
        // As fallback read the full processes list
        PROCESSENTRY32 pe = {0};
        if (PROCMETRIX_ERROR_NONE == (status = procmetrix_get_process_entry(pid, &pe)))
            *ppid = pe.th32ParentProcessID;
    }

    return status;
}

procmetrix_error_t procmetrix_get_proc_name(procmetrix_pid_t pid, char *name, size_t len)
{
    // Sanity
    if (NULL == name || 0 == len)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    memset(name, 0, len);

    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Get full exe path
    char path[MAX_PATH];
    procmetrix_error_t status = procmetrix_get_proc_exe(pid, path, sizeof(path));

    // Extract only the base name
    strncpy(name, PathFindFileName(path), len);
    name[len - 1] = '\0';

    return status;
}

procmetrix_error_t procmetrix_get_proc_exe(procmetrix_pid_t pid, char *path, size_t len)
{
    // Sanity
    if (NULL == path || 0 == len)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    memset(path, 0, len);

    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Open the process to read the info
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (NULL == hProcess)
        return PROCMETRIX_ERROR_UNKNOWN;

    // Try to read the exe name
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;

    DWORD size = len;
    if (!QueryFullProcessImageName(hProcess, 0, path, &size))
        status = PROCMETRIX_ERROR_UNKNOWN;

    // Cleanup
    CloseHandle(hProcess);

    return status;
}

procmetrix_error_t procmetrix_get_proc_cwd(procmetrix_pid_t pid, char *dst_cwd, size_t dst_len)
{
    // Sanity
    if (NULL == dst_cwd || 0 == dst_len)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    memset(dst_cwd, 0, sizeof(dst_cwd));

    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Read raw CWD from the process memory
    wchar_t *wide_cwd = NULL;
    procmetrix_error_t status = procmetric_read_process_params(pid, &wide_cwd, NULL, NULL);

    if (PROCMETRIX_ERROR_NONE == status)
    {
        int copy_len = WideCharToMultiByte(CP_UTF8, 0, wide_cwd, -1, dst_cwd, dst_len, NULL, NULL);
        if (0 == copy_len)
            status = PROCMETRIX_ERROR_MORE_DATA;
        else if (copy_len > 4 && dst_cwd[copy_len - 2] == '\\')
        {
            //  Trim trailing \\ if it exists
            dst_cwd[copy_len - 2] = '\0';
        }
    }

    // Cleanup
    free(wide_cwd);

    return status;
}

procmetrix_error_t procmetrix_get_proc_cmdline(procmetrix_pid_t pid, procmetrix_proc_cmdline_t *cmdline)
{
    // Sanity
    if (NULL == cmdline)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    memset(cmdline, 0, sizeof(*cmdline));

    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Read raw CLI from the process memory
    wchar_t *cli = NULL;
    procmetrix_error_t status = procmetric_read_process_params(pid, NULL, &cli, NULL);

    // Split the full cli into argc and argv
    int argc = 0;
    wchar_t **argv = NULL;
    if (PROCMETRIX_ERROR_NONE == status)
    {
        argv = CommandLineToArgvW(cli, &argc);

        if (argc == 0 || NULL == argv)
            status = PROCMETRIX_ERROR_UNKNOWN;
    }

    // Alloc space for the ansi vector
    if (PROCMETRIX_ERROR_NONE == status)
    {
        cmdline->argv = (char **)calloc(argc, sizeof(char *));
        if (NULL == cmdline->argv)
            status = PROCMETRIX_ERROR_OUT_OF_MEMORY;
    }

    // Copy the items
    if (PROCMETRIX_ERROR_NONE == status)
    {
        for (size_t i = 0; i < argc; ++i)
        {
            // Find required buffer len
            int len = WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, NULL, 0, NULL, NULL);
            if (len <= 0)
            {
                status = PROCMETRIX_ERROR_UNKNOWN;
                break;
            }

            // Allocate that argument
            cmdline->argv[i] = (char *)malloc(len);
            if (NULL == cmdline->argv[i])
            {
                status = PROCMETRIX_ERROR_OUT_OF_MEMORY;
                break;
            }

            WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, cmdline->argv[i], len, NULL, NULL);
            ++cmdline->argc;
        }
    }

    // Cleanup
    LocalFree(argv);
    free(cli);

    return status;
}

procmetrix_error_t procmetrix_get_proc_environ(procmetrix_pid_t pid, procmetrix_proc_environ_t *proc_environ)
{
    // @TODO
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t procmetrix_get_proc_memory_info(procmetrix_pid_t pid, procmetrix_proc_memory_info_t *memory_info)
{
    // @TODO
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t procmetrix_get_proc_cpu_times(procmetrix_pid_t pid, procmetrix_proc_cpu_times_t *cpu_times)
{
    // @TODO
    return PROCMETRIX_NOT_IMPLEMENTED;
}
