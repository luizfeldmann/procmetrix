// Bindings
#include "error.h"
#include "init.h"

// Lib
#include <procmetrix/cpu.h>

// STD
#include <string.h>

// ============================================================================
// CPU Times
// ============================================================================

typedef struct
{
    PyObject_HEAD;
    procmetrix_cpu_times_t times;
} cpu_times_wrapper_t;

static PyObject* cpu_times_user(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(((cpu_times_wrapper_t*)self)->times.user);
}

static PyObject* cpu_times_system(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(((cpu_times_wrapper_t*)self)->times.system);
}

static PyObject* cpu_times_idle(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(((cpu_times_wrapper_t*)self)->times.idle);
}

static PyObject* cpu_times_nice(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(((cpu_times_wrapper_t*)self)->times.nice);
}

static PyObject* cpu_times_iowait(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(((cpu_times_wrapper_t*)self)->times.iowait);
}

static PyObject* cpu_times_irq(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(((cpu_times_wrapper_t*)self)->times.irq);
}

static PyObject* cpu_times_softirq(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(((cpu_times_wrapper_t*)self)->times.softirq);
}

static PyObject* cpu_times_steal(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(((cpu_times_wrapper_t*)self)->times.steal);
}

static PyObject* cpu_times_guest(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(((cpu_times_wrapper_t*)self)->times.guest);
}

static PyObject* cpu_times_guest_nice(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(((cpu_times_wrapper_t*)self)->times.guest_nice);
}

static PyObject* cpu_times_interrupt(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(((cpu_times_wrapper_t*)self)->times.interrupt);
}

static PyObject* cpu_times_dpc(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(((cpu_times_wrapper_t*)self)->times.dpc);
}

static PyGetSetDef cpu_times_getters[] = {
    // clang-format off
    { (char*)"user",        cpu_times_user },
    { (char*)"system",      cpu_times_system },
    { (char*)"idle",        cpu_times_idle },
    { (char*)"nice",        cpu_times_nice },
    { (char*)"iowait",      cpu_times_iowait },
    { (char*)"irq",         cpu_times_irq },
    { (char*)"softirq",     cpu_times_softirq },
    { (char*)"steal",       cpu_times_steal },
    { (char*)"guest",       cpu_times_guest },
    { (char*)"guest_nice",  cpu_times_guest_nice },
    { (char*)"interrupt",   cpu_times_interrupt },
    { (char*)"dpc",         cpu_times_dpc },
    // clang-format on
    { NULL },
};

static PyType_Slot cpu_times_slots[] = {
    { Py_tp_getset, cpu_times_getters },
    { 0, nullptr },
};

static PyType_Spec cpu_times_spec = {
    "procmetrix.SystemCpuTimes",
    sizeof(cpu_times_wrapper_t),
    0,
    Py_TPFLAGS_DEFAULT,
    cpu_times_slots,
};

// assigned in the registration
static PyObject* cpu_times_type = NULL;

static PyObject* system_cpu_times_total(PyObject* self, PyObject* args)
{
    // Allocate object
    cpu_times_wrapper_t* wrapper =
        (cpu_times_wrapper_t*)PyObject_CallNoArgs(cpu_times_type);

    if (NULL == wrapper)
        return py_procmetrix_error(PROCMETRIX_ERROR_UNKNOWN);

    // Read total cpu times
    procmetrix_error_t status = procmetrix_cpu_times_total(&wrapper->times);

    if (PROCMETRIX_ERROR_NONE != status)
    {
        Py_DECREF(wrapper);
        return py_procmetrix_error(status);
    }

    // Return dict
    return (PyObject*)wrapper;
}

static PyObject* system_cpu_times_per_cpu(PyObject* self, PyObject* args)
{
    // Get the number of CPUs
    size_t ncpus = procmetrix_cpu_count_logical();
    if (0 == ncpus)
        return py_procmetrix_error(PROCMETRIX_ERROR_UNKNOWN);

    // Read all the times
    procmetrix_cpu_times_t* cpu_times =
        PyMem_Calloc(ncpus, sizeof(procmetrix_cpu_times_t));

    size_t read_count = 0;
    procmetrix_error_t status =
        procmetrix_cpu_times_per_cpu(cpu_times, ncpus, &read_count);

    // Convert to the python object format
    PyObject* result = NULL;
    if (PROCMETRIX_ERROR_NONE != status)
        result = py_procmetrix_error(status);
    else
    {
        // Copy each item
        result = PyList_New(read_count);
        for (size_t i = 0; i < read_count; ++i)
        {
            cpu_times_wrapper_t* wrapper =
                (cpu_times_wrapper_t*)PyObject_CallNoArgs(cpu_times_type);
            memcpy(&wrapper->times, &cpu_times[i], sizeof(*cpu_times));
            PyList_SetItem(result, i, (PyObject*)wrapper);
        }
    }

    // Cleanup
    PyMem_Free(cpu_times);

    return result;
}

// ============================================================================
// CPU Freqs
// ============================================================================

// ============================================================================
// Registration
// ============================================================================

static PyMethodDef methods[] = {
    // System memory
    { "cpu_times_total", system_cpu_times_total, METH_VARARGS },
    { "cpu_times_per_cpu", system_cpu_times_per_cpu, METH_VARARGS },
    // End of list
    { NULL, NULL, 0 }
};

void pyprocmetrix_init_cpu(PyObject* module)
{
    // Register types
    cpu_times_type = PyType_FromSpec(&cpu_times_spec);
    PyModule_AddObjectRef(module, "SystemCpuTimes", cpu_times_type);

    // Register methods
    PyModule_AddFunctions(module, methods);
}
