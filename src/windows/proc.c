// Internals
#include <internal/algo.h>

// Windows
#include <Psapi.h>
#include <Windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <tlhelp32.h>
#include <winternl.h>
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "Shlwapi.lib")

// Lib interals
#include <internal/windows/common_windows_internal.h>

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

typedef struct RTL_DRIVE_LETTER_CURDIR
{
    USHORT Flags;
    USHORT Length;
    ULONG TimeStamp;
    UNICODE_STRING DosPath;

} RTL_DRIVE_LETTER_CURDIR;

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
    RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[0x20];
    ULONG_PTR volatile EnvironmentSize;
} RUPP_t;

//! Finds the PROCESSENTRY32 associated with a PID
static procmetrix_error_t
procmetrix_get_process_entry(procmetrix_pid_t pid, PROCESSENTRY32* foundproc)
{
    // List of all processes
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == snapshot)
        return PROCMETRIX_ERROR_UNKNOWN;

    // Iterate all processes
    procmetrix_error_t status = PROCMETRIX_ERROR_INVALID_ARGUMENT;

    PROCESSENTRY32 pe = { 0 };
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
    PROCESSENTRY32 pe = { 0 };
    return PROCMETRIX_ERROR_NONE == procmetrix_get_process_entry(pid, &pe);
}

static procmetrix_error_t procmetric_read_process_param(
    HANDLE hProcess, wchar_t** data, const void* base_address, size_t read_len)
{
    // Sanity (1)
    if (NULL == data)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    *data = NULL; // robustness

    // Sanity (2)
    if (NULL == hProcess || NULL == base_address || 0 == read_len)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Alloc space for the result, ensure a final '\0' always
    *data = calloc(read_len + sizeof(wchar_t), 1);
    if (NULL == *data)
        return PROCMETRIX_ERROR_OUT_OF_MEMORY;

    // Read the memory
    if (!ReadProcessMemory(hProcess, base_address, *data, read_len, NULL))
    {
        // Cleanup
        free(*data);
        *data = NULL;

        return PROCMETRIX_ERROR_UNKNOWN;
    }

    // Ensure safe zero termination
    (*data)[read_len / sizeof(wchar_t)] = L'\0';

    return PROCMETRIX_ERROR_NONE;
}

//! Process parameter output
typedef struct
{
    //! Output buffer data
    wchar_t* buf;
    //! Data length
    size_t nbytes;
} procmetrix_proc_param_out_t;

//! Reads params from a process's memory
static procmetrix_error_t procmetric_read_process_params(
    procmetrix_pid_t pid,
    procmetrix_proc_param_out_t* cwd,
    procmetrix_proc_param_out_t* cli,
    procmetrix_proc_param_out_t* env)
{
    // Open the process
    HANDLE hProcess =
        OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
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
    PEB peb = { 0 };
    if (!ReadProcessMemory(
            hProcess, pbi.PebBaseAddress, &peb, sizeof(PEB), NULL))
    {
        CloseHandle(hProcess);
        return PROCMETRIX_ERROR_UNKNOWN;
    }

    // Read process params
    RUPP_t proc_parameters;
    if (!ReadProcessMemory(
            hProcess,
            peb.ProcessParameters,
            &proc_parameters,
            sizeof(proc_parameters),
            NULL))
    {
        CloseHandle(hProcess);
        return PROCMETRIX_ERROR_UNKNOWN;
    }

    // Read one of the desired params
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;

    if (NULL != cwd)
    {
        cwd->nbytes = proc_parameters.CurrentDirectoryPath.Length;
        status = procmetric_read_process_param(
            hProcess,
            &cwd->buf,
            proc_parameters.CurrentDirectoryPath.Buffer,
            proc_parameters.CurrentDirectoryPath.Length);
    }
    else if (NULL != cli)
    {
        cli->nbytes = proc_parameters.CommandLine.Length;
        status = procmetric_read_process_param(
            hProcess,
            &cli->buf,
            proc_parameters.CommandLine.Buffer,
            proc_parameters.CommandLine.Length);
    }
    else if (NULL != env)
    {
        env->nbytes = proc_parameters.EnvironmentSize;
        status = procmetric_read_process_param(
            hProcess,
            &env->buf,
            proc_parameters.Environment,
            proc_parameters.EnvironmentSize);
    }
    else
    {
        status = PROCMETRIX_ERROR_INVALID_ARGUMENT;
    }

    // Cleanup
    CloseHandle(hProcess);

    return status;
}

//! Converts wide string to narrow string
static char* procmetrix_wide_to_narrow(
    const wchar_t* wide, int input_num_wchars, int* output_len)
{
    // Sanity
    if (NULL == wide)
        return NULL;

    // Discover required size
    int result_len = WideCharToMultiByte(
        CP_UTF8, 0, wide, input_num_wchars, NULL, 0, NULL, NULL);
    if (result_len <= 0)
        return NULL;

    // Perform the conversion
    char* narrow = (char*)malloc(result_len);
    result_len = WideCharToMultiByte(
        CP_UTF8, 0, wide, input_num_wchars, narrow, result_len, NULL, NULL);
    if (result_len <= 0)
    {
        free(narrow);
        narrow = NULL;
    }
    else if (NULL != output_len)
    {
        *output_len = result_len;
    }

    return narrow;
}

//! Parses the environment variables into the output struct
static procmetrix_error_t procmetrix_read_environment_variables(
    const wchar_t* wide_env,
    size_t wide_env_bytes,
    procmetrix_proc_environ_t* proc_environ)
{
    // Sanity
    if (NULL == wide_env || 0 == wide_env_bytes || NULL == proc_environ)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Convert to a narrow UTF-8 string
    int narrow_env_len = 0;
    char* narrow_env = procmetrix_wide_to_narrow(
        wide_env, wide_env_bytes / sizeof(wchar_t), &narrow_env_len);

    if (NULL == narrow_env || 0 == narrow_env_len)
        return PROCMETRIX_ERROR_UNKNOWN;

    // Tokenize as lines
    char** var_lines = NULL;
    size_t num_lines = 0;
    procmetrix_error_t status = procmetrix_split_zero_terminated_tokens(
        narrow_env, narrow_env_len, &var_lines, &num_lines);

    // Cleanup raw environment, no longer needed
    free(narrow_env);
    narrow_env = NULL;

    // Split key value pairs
    if (PROCMETRIX_ERROR_NONE == status)
    {
        // Windows has fake variables at the start, without a name, starting
        // with '='
        size_t num_valid = num_lines;
        char** var_valid = var_lines;
        while (num_valid && **var_valid == '=')
            ++var_valid, --num_valid;

        // Split the name and value by =
        status = procmetrix_split_environ_vars(
            (const char* const*)var_valid, num_valid, proc_environ);
    }

    // Cleanup
    for (size_t i = 0; i < num_lines; ++i)
        free(var_lines[i]);
    free(var_lines);

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
    HANDLE hProcess =
        OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (NULL != hProcess)
    {
        // We need to check if the process is really running or has already
        // exited
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

procmetrix_error_t
procmetrix_list_pids(procmetrix_pid_t** list, size_t* out_count)
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
    PROCESSENTRY32 pe = { 0 };
    pe.dwSize = sizeof(pe);

    if (Process32First(snapshot, &pe))
    {
        ++num_procs;
        while (Process32Next(snapshot, &pe))
            ++num_procs;
    }

    // Allocate size of items in the list
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;

    procmetrix_pid_t* pids =
        (procmetrix_pid_t*)calloc(num_procs, sizeof(procmetrix_pid_t));
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

procmetrix_error_t
procmetrix_get_proc_parent_pid(procmetrix_pid_t pid, procmetrix_pid_t* ppid)
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
    HANDLE hProcess =
        OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (NULL != hProcess)
    {
        PBI_t pbi = { 0 };
        if (NT_SUCCESS(NtQueryInformationProcess(
                hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), NULL)))
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
        PROCESSENTRY32 pe = { 0 };
        if (PROCMETRIX_ERROR_NONE ==
            (status = procmetrix_get_process_entry(pid, &pe)))
            *ppid = pe.th32ParentProcessID;
    }

    return status;
}

procmetrix_error_t
procmetrix_get_proc_name(procmetrix_pid_t pid, char* name, size_t len)
{
    // Sanity
    if (NULL == name || 0 == len)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    memset(name, 0, len);

    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Get full exe path
    char path[MAX_PATH];
    procmetrix_error_t status =
        procmetrix_get_proc_exe(pid, path, sizeof(path));

    // Extract only the base name
    if (PROCMETRIX_ERROR_NONE == status)
    {
        errno_t err = strncpy_s(name, len, PathFindFileName(path), _TRUNCATE);
        if (0 != err)
            status = PROCMETRIX_ERROR_MORE_DATA;
    }

    return status;
}

procmetrix_error_t
procmetrix_get_proc_exe(procmetrix_pid_t pid, char* path, size_t len)
{
    // Sanity
    if (NULL == path || 0 == len)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    memset(path, 0, len);

    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Open the process to read the info
    HANDLE hProcess =
        OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
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

procmetrix_error_t
procmetrix_get_proc_cwd(procmetrix_pid_t pid, char* dst_cwd, size_t dst_len)
{
    // Sanity
    if (NULL == dst_cwd || 0 == dst_len)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    memset(dst_cwd, 0, dst_len);

    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Read raw CWD from the process memory
    procmetrix_proc_param_out_t read_param = { 0 };
    procmetrix_error_t status =
        procmetric_read_process_params(pid, &read_param, NULL, NULL);

    if (PROCMETRIX_ERROR_NONE == status)
    {
        int copy_len = WideCharToMultiByte(
            CP_UTF8,
            0,
            read_param.buf,
            read_param.nbytes / sizeof(wchar_t),
            dst_cwd,
            dst_len - 1,
            NULL,
            NULL);
        if (0 == copy_len)
            status = PROCMETRIX_ERROR_MORE_DATA;
        else
        {
            // Ensure a terminating \0
            dst_cwd[copy_len] = '\0';

            // Trim trailing "\"" if it exists
            // Do not trim "C:\" to "C:"
            if (copy_len > 3 && dst_cwd[copy_len - 1] == '\\')
            {
                dst_cwd[copy_len - 1] = '\0';
            }
        }
    }

    // Cleanup
    free(read_param.buf);

    return status;
}

procmetrix_error_t procmetrix_get_proc_cmdline(
    procmetrix_pid_t pid, procmetrix_proc_cmdline_t* cmdline)
{
    // Sanity
    if (NULL == cmdline)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    memset(cmdline, 0, sizeof(*cmdline));

    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Read raw CLI from the process memory
    procmetrix_proc_param_out_t read_param = { 0 };
    procmetrix_error_t status =
        procmetric_read_process_params(pid, NULL, &read_param, NULL);

    // Split the full cli into argc and argv
    int argc = 0;
    wchar_t** argv = NULL;
    if (PROCMETRIX_ERROR_NONE == status)
    {
        argv = CommandLineToArgvW(read_param.buf, &argc);

        if (argc == 0 || NULL == argv)
            status = PROCMETRIX_ERROR_UNKNOWN;
    }

    // Cleanup, no longer need raw param
    free(read_param.buf);
    memset(&read_param, 0, sizeof(read_param));

    // Alloc space for the ansi vector
    if (PROCMETRIX_ERROR_NONE == status)
    {
        cmdline->argv = (char**)calloc(argc, sizeof(char*));
        if (NULL == cmdline->argv)
            status = PROCMETRIX_ERROR_OUT_OF_MEMORY;
    }

    // Copy the items
    if (PROCMETRIX_ERROR_NONE == status)
    {
        for (size_t i = 0; i < argc; ++i)
        {
            cmdline->argv[i] = procmetrix_wide_to_narrow(argv[i], -1, NULL);
            if (NULL == cmdline->argv[i])
            {
                status = PROCMETRIX_ERROR_OUT_OF_MEMORY;
                break;
            }
            ++cmdline->argc;
        }
    }

    // Cleanup
    LocalFree(argv);

    return status;
}

procmetrix_error_t procmetrix_get_proc_environ(
    procmetrix_pid_t pid, procmetrix_proc_environ_t* proc_environ)
{
    // Sanity
    if (NULL == proc_environ)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    memset(proc_environ, 0, sizeof(*proc_environ));

    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Read raw environment from the process memory
    procmetrix_proc_param_out_t read_param = { 0 };
    procmetrix_error_t status =
        procmetric_read_process_params(pid, NULL, NULL, &read_param);

    // Split the lines and key-value
    if (PROCMETRIX_ERROR_NONE == status)
    {
        status = procmetrix_read_environment_variables(
            read_param.buf, read_param.nbytes, proc_environ);
    }

    // Cleanup
    free(read_param.buf);

    return status;
}

procmetrix_error_t procmetrix_get_proc_memory_info(
    procmetrix_pid_t pid, procmetrix_proc_memory_info_t* memory_info)
{
    // Sanity
    if (NULL == memory_info)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    memset(memory_info, 0, sizeof(*memory_info));

    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Open process
    HANDLE hProcess =
        OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (NULL == hProcess)
        return PROCMETRIX_ERROR_UNKNOWN;

    // Read process memory counters
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;

    PROCESS_MEMORY_COUNTERS_EX cnt = { 0 };
    if (!GetProcessMemoryInfo(
            hProcess, (PPROCESS_MEMORY_COUNTERS)&cnt, sizeof(cnt)))
        status = PROCMETRIX_ERROR_UNKNOWN;
    else
    {
        memory_info->rss = cnt.WorkingSetSize;
        memory_info->vms = cnt.PrivateUsage;
    }

    // Cleanup
    CloseHandle(hProcess);

    return status;
}

procmetrix_error_t procmetrix_get_proc_cpu_times(
    procmetrix_pid_t pid, procmetrix_proc_cpu_times_t* cpu_times)
{
    // Sanity
    if (NULL == cpu_times)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    memset(cpu_times, 0, sizeof(*cpu_times));

    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Open process
    HANDLE hProcess =
        OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (NULL == hProcess)
        return PROCMETRIX_ERROR_UNKNOWN;

    // Read process times counters
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;

    FILETIME ftCreate = { 0 }, ftExit = { 0 }, ftKernel = { 0 }, ftUser = { 0 };
    if (!GetProcessTimes(hProcess, &ftCreate, &ftExit, &ftKernel, &ftUser))
        status = PROCMETRIX_ERROR_UNKNOWN;
    else
    {
        cpu_times->user = procmetrix_impl_windows_filetime_to_secs(&ftUser);
        cpu_times->system = procmetrix_impl_windows_filetime_to_secs(&ftKernel);
    }

    // Cleanup
    CloseHandle(hProcess);

    return status;
}
