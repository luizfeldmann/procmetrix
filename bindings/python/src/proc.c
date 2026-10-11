// Bindings
#include "error.h"
#include "init.h"

// Lib
#include <procmetrix/proc.h>

static PyObject* get_pid(PyObject* self, PyObject* args)
{
    procmetrix_pid_t pid = procmetrix_get_pid();
    return PyLong_FromUnsignedLong(pid);
}

static PyObject* pid_exists(PyObject* self, PyObject* args)
{
    // Input args
    procmetrix_pid_t pid = 0;
    if (!PyArg_ParseTuple(args, "I", &pid))
        return NULL;

    // Check exists
    if (procmetrix_pid_exists(pid))
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyObject* list_pids(PyObject* self, PyObject* args)
{
    // Get the list of PIDs
    size_t count = 0;
    procmetrix_pid_t* native_list = NULL;
    procmetrix_error_t status = procmetrix_list_pids(&native_list, &count);

    if (PROCMETRIX_ERROR_NONE != status)
    {
        procmetrix_free_pids(&native_list);
        return py_procmetrix_error(status);
    }

    // Allocate output list
    PyObject* py_list = PyList_New((Py_ssize_t)count);
    if (NULL == py_list)
    {
        procmetrix_free_pids(&native_list);
        return NULL;
    }

    // Copy each item into the list
    for (size_t i = 0; i < count; ++i)
    {
        PyObject* value = PyLong_FromUnsignedLong(native_list[i]);
        if (NULL == value || 0 != PyList_SetItem(py_list, (Py_ssize_t)i, value))
        {
            Py_XDECREF(value);
            Py_DECREF(py_list);
            py_list = NULL;
            break;
        }
    }

    // Cleanup
    procmetrix_free_pids(&native_list);

    return py_list;
}

static PyObject* get_proc_parent_pid(PyObject* self, PyObject* args)
{
    // Input args
    procmetrix_pid_t pid = 0;
    if (!PyArg_ParseTuple(args, "I", &pid))
        return NULL;

    // Get parent pid
    procmetrix_pid_t parent_pid = 0;
    procmetrix_error_t status =
        procmetrix_get_proc_parent_pid(pid, &parent_pid);

    if (PROCMETRIX_ERROR_NONE != status)
        return py_procmetrix_error(status);

    return PyLong_FromUnsignedLong(parent_pid);
}

static PyObject* get_proc_name(PyObject* self, PyObject* args)
{
    // Input args
    procmetrix_pid_t pid = 0;
    if (!PyArg_ParseTuple(args, "I", &pid))
        return NULL;

    // Get the name
    char name[1024];
    procmetrix_error_t status =
        procmetrix_get_proc_name(pid, name, sizeof(name));

    if (PROCMETRIX_ERROR_NONE != status)
        return py_procmetrix_error(status);

    return PyUnicode_FromString(name);
}

static PyObject* get_proc_exe(PyObject* self, PyObject* args)
{
    // Input args
    procmetrix_pid_t pid = 0;
    if (!PyArg_ParseTuple(args, "I", &pid))
        return NULL;

    // Get the exe
    char path[4096];
    procmetrix_error_t status =
        procmetrix_get_proc_exe(pid, path, sizeof(path));

    if (PROCMETRIX_ERROR_NONE != status)
        return py_procmetrix_error(status);

    return PyUnicode_FromString(path);
}

static PyObject* get_proc_cwd(PyObject* self, PyObject* args)
{
    // Input args
    procmetrix_pid_t pid = 0;
    if (!PyArg_ParseTuple(args, "I", &pid))
        return NULL;

    // Get the cwd
    char cwd[4096];
    procmetrix_error_t status = procmetrix_get_proc_cwd(pid, cwd, sizeof(cwd));

    if (PROCMETRIX_ERROR_NONE != status)
        return py_procmetrix_error(status);

    return PyUnicode_FromString(cwd);
}

static PyObject* get_proc_cmdline(PyObject* self, PyObject* args)
{
    // Input args
    procmetrix_pid_t pid = 0;
    if (!PyArg_ParseTuple(args, "I", &pid))
        return NULL;

    // Get the command line
    procmetrix_proc_cmdline_t cmdline = { 0 };
    procmetrix_error_t status = procmetrix_get_proc_cmdline(pid, &cmdline);

    if (PROCMETRIX_ERROR_NONE != status)
    {
        procmetrix_free_proc_cmdline(&cmdline);
        return py_procmetrix_error(status);
    }

    // Allocate output list
    PyObject* list = PyList_New((Py_ssize_t)cmdline.argc);
    if (NULL == list)
    {
        procmetrix_free_proc_cmdline(&cmdline);
        return NULL;
    }

    // Copy cmdline to the output
    for (size_t i = 0; i < cmdline.argc; ++i)
    {
        // Allocate item string
        PyObject* argv = PyUnicode_FromString(cmdline.argv[i]);
        if (NULL == argv)
        {
            // Free partial list and abort the loop
            Py_DECREF(list);
            list = NULL;
            break;
        }

        // Add item to the list
        PyList_SetItem(list, (Py_ssize_t)i, argv);
    }

    // Cleanup
    procmetrix_free_proc_cmdline(&cmdline);

    return list;
}

static PyObject* get_proc_environ(PyObject* self, PyObject* args)
{
    // Input args
    procmetrix_pid_t pid = 0;
    if (!PyArg_ParseTuple(args, "I", &pid))
        return NULL;

    // Read the environment
    procmetrix_proc_environ_t proc_environ = { 0 };
    procmetrix_error_t status = procmetrix_get_proc_environ(pid, &proc_environ);

    if (PROCMETRIX_ERROR_NONE != status)
    {
        procmetrix_free_proc_environ(&proc_environ);
        return py_procmetrix_error(status);
    }

    // Allocate output dict
    PyObject* dict = PyDict_New();
    if (!dict)
    {
        procmetrix_free_proc_environ(&proc_environ);
        return NULL;
    }

    // Copy items to the output
    for (size_t i = 0; i < proc_environ.count; ++i)
    {
        procmetrix_proc_environ_var_t* var = &proc_environ.vars[i];

        PyObject* value = PyUnicode_FromString(var->value);

        if (NULL == value || 0 != PyDict_SetItemString(dict, var->name, value))
        {
            // Free partial dict and abort loop
            Py_XDECREF(value);
            Py_DECREF(dict);
            dict = NULL;
            break;
        }

        // PyDict_SetItemString does not steal the reference
        Py_DECREF(value);
    }

    // Cleanup
    procmetrix_free_proc_environ(&proc_environ);

    return dict;
}

// ============================================================================
// Process' Memory
// ============================================================================

typedef struct proc_memory_wrapper
{
    PyObject_HEAD
    procmetrix_proc_memory_info_t proc_mem;
} proc_memory_wrapper_t;

static PyObject* proc_mem_rss(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((proc_memory_wrapper_t*)self)->proc_mem.rss);
}

static PyObject* proc_mem_vms(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((proc_memory_wrapper_t*)self)->proc_mem.vms);
}

static PyObject* proc_mem_shared(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((proc_memory_wrapper_t*)self)->proc_mem.shared);
}

static PyObject* proc_mem_text(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((proc_memory_wrapper_t*)self)->proc_mem.text);
}

static PyObject* proc_mem_lib(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((proc_memory_wrapper_t*)self)->proc_mem.lib);
}

static PyObject* proc_mem_data(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((proc_memory_wrapper_t*)self)->proc_mem.data);
}

static PyObject* proc_mem_dirty(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((proc_memory_wrapper_t*)self)->proc_mem.dirty);
}

static PyGetSetDef g_getters_proc_mem[] = {
    // clang-format off
    { (char*)"rss",     proc_mem_rss },
    { (char*)"vms",     proc_mem_vms },
    { (char*)"shared",  proc_mem_shared },
    { (char*)"text",    proc_mem_text },
    { (char*)"lib",     proc_mem_lib },
    { (char*)"data",    proc_mem_data },
    { (char*)"dirty",   proc_mem_dirty },
    // clang-format on
    { NULL },
};

static PyType_Slot g_slots_proc_mem[] = {
    { Py_tp_getset, g_getters_proc_mem },
    { 0, NULL },
};

static PyType_Spec g_type_spec_proc_mem = {
    "procmetrix.ProcessMemory",    // name
    sizeof(proc_memory_wrapper_t), // basicsize
    0,                             // itemsize
    Py_TPFLAGS_DEFAULT,            // flags
    g_slots_proc_mem,              // slots
};

static PyObject* get_proc_memory_info(PyObject* self, PyObject* args)
{
    // Input args
    procmetrix_pid_t pid = 0;
    if (!PyArg_ParseTuple(args, "I", &pid))
        return NULL;

    // Allocate the result
    py_procmetrix_module_state_t* state = PyModule_GetState(self);

    proc_memory_wrapper_t* wrapper =
        (proc_memory_wrapper_t*)PyObject_CallNoArgs(
            state->types[PROCMETRIX_TYPE_PROCESS_MEMORY]);

    if (NULL == wrapper)
        return NULL;

    // Get the memory info
    procmetrix_error_t status =
        procmetrix_get_proc_memory_info(pid, &wrapper->proc_mem);

    if (PROCMETRIX_ERROR_NONE != status)
    {
        Py_DECREF(wrapper);
        return py_procmetrix_error(status);
    }

    return (PyObject*)wrapper;
}

// ============================================================================
// Process' CPU use
// ============================================================================

typedef struct proc_cpu_times_wrapper
{
    PyObject_HEAD
    procmetrix_proc_cpu_times_t proc_cpu;
} proc_cpu_times_wrapper_t;

static PyObject* proc_cpu_user(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(((proc_cpu_times_wrapper_t*)self)->proc_cpu.user);
}

static PyObject* proc_cpu_system(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(
        ((proc_cpu_times_wrapper_t*)self)->proc_cpu.system);
}

static PyObject* proc_cpu_children_user(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(
        ((proc_cpu_times_wrapper_t*)self)->proc_cpu.children_user);
}

static PyObject* proc_cpu_children_system(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(
        ((proc_cpu_times_wrapper_t*)self)->proc_cpu.children_system);
}

static PyGetSetDef g_getters_proc_cpu_times[] = {
    // clang-format off
    { (char*)"user",            proc_cpu_user },
    { (char*)"system",          proc_cpu_system },
    { (char*)"children_user",   proc_cpu_children_user },
    { (char*)"children_system", proc_cpu_children_system },
    // clang-format on
    { NULL },
};

static PyType_Slot g_slots_proc_cpu_times[] = {
    { Py_tp_getset, g_getters_proc_cpu_times },
    { 0, NULL },
};

static PyType_Spec g_type_spec_proc_cpu_times = {
    "procmetrix.ProcessCpuTimes",     // name
    sizeof(proc_cpu_times_wrapper_t), // basicsize
    0,                                // itemsize
    Py_TPFLAGS_DEFAULT,               // flags
    g_slots_proc_cpu_times,           // slots
};

static PyObject* get_proc_cpu_times(PyObject* self, PyObject* args)
{
    // Input args
    procmetrix_pid_t pid = 0;
    if (!PyArg_ParseTuple(args, "I", &pid))
        return NULL;

    // Allocate the result
    py_procmetrix_module_state_t* state = PyModule_GetState(self);

    proc_cpu_times_wrapper_t* wrapper =
        (proc_cpu_times_wrapper_t*)PyObject_CallNoArgs(
            state->types[PROCMETRIX_TYPE_PROCESS_CPU_USAGE]);

    if (NULL == wrapper)
        return NULL;

    // Get the cpu usage times
    procmetrix_error_t status =
        procmetrix_get_proc_cpu_times(pid, &wrapper->proc_cpu);

    if (PROCMETRIX_ERROR_NONE != status)
    {
        Py_DECREF(wrapper);
        return py_procmetrix_error(status);
    }

    return (PyObject*)wrapper;
}

// ============================================================================
// Registration
// ============================================================================

static PyMethodDef g_methods_proc[] = {
    { "get_pid", get_pid, METH_NOARGS },
    { "pid_exists", pid_exists, METH_VARARGS },
    { "list_pids", list_pids, METH_NOARGS },
    { "get_proc_parent_pid", get_proc_parent_pid, METH_VARARGS },
    { "get_proc_name", get_proc_name, METH_VARARGS },
    { "get_proc_exe", get_proc_exe, METH_VARARGS },
    { "get_proc_cwd", get_proc_cwd, METH_VARARGS },
    { "get_proc_cmdline", get_proc_cmdline, METH_VARARGS },
    { "get_proc_environ", get_proc_environ, METH_VARARGS },
    { "get_proc_memory_info", get_proc_memory_info, METH_VARARGS },
    { "get_proc_cpu_times", get_proc_cpu_times, METH_VARARGS },
    // End of list
    { NULL, NULL, 0 }
};

int pyprocmetrix_init_proc(PyObject* module)
{
    py_procmetrix_module_state_t* state = PyModule_GetState(module);

    // Register types
    if (pyprocmetrix_register_type(
            module,
            state,
            "ProcessMemory",
            PROCMETRIX_TYPE_PROCESS_MEMORY,
            &g_type_spec_proc_mem) != PY_INIT_OK)
        return PY_INIT_FAIL;

    if (pyprocmetrix_register_type(
            module,
            state,
            "ProcessCpuTimes",
            PROCMETRIX_TYPE_PROCESS_CPU_USAGE,
            &g_type_spec_proc_cpu_times) != PY_INIT_OK)
        return PY_INIT_FAIL;

    // Register methods
    if (PyModule_AddFunctions(module, g_methods_proc) != PY_INIT_OK)
        return PY_INIT_FAIL;

    return PY_INIT_OK;
}
