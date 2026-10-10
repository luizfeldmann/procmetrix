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
    PyObject* py_list = PyList_New(count);
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
    PyObject* list = PyList_New(cmdline.argc);
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
    // End of list
    { NULL, NULL, 0 }
};

int pyprocmetrix_init_proc(PyObject* module)
{
    // Register methods
    if (PyModule_AddFunctions(module, g_methods_proc) != PY_INIT_OK)
        return PY_INIT_FAIL;

    return PY_INIT_OK;
}
