// Bindings
#include "error.h"
#include "init.h"

// Lib
#include <procmetrix/cpu.h>

// STD
#include <string.h>

// ============================================================================
// CPU Count
// ============================================================================

static PyObject* cpu_count_physical(PyObject* self, PyObject* args)
{
    return PyLong_FromUnsignedLongLong(procmetrix_cpu_count_physical());
}

static PyObject* cpu_count_logical(PyObject* self, PyObject* args)
{
    return PyLong_FromUnsignedLongLong(procmetrix_cpu_count_logical());
}

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
    { 0, NULL },
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
        return NULL;

    // Read total cpu times
    procmetrix_error_t status = procmetrix_cpu_times_total(&wrapper->times);

    if (PROCMETRIX_ERROR_NONE != status)
    {
        Py_DECREF(wrapper);
        return py_procmetrix_error(status);
    }

    return (PyObject*)wrapper;
}

static PyObject* system_cpu_times_per_cpu(PyObject* self, PyObject* args)
{
    // Get the number of CPUs
    size_t ncpus = procmetrix_cpu_count_logical();
    if (0 == ncpus)
        return py_procmetrix_error(PROCMETRIX_ERROR_UNKNOWN);

    // Alloc results array
    procmetrix_cpu_times_t* cpu_times =
        PyMem_Calloc(ncpus, sizeof(procmetrix_cpu_times_t));

    if (NULL == cpu_times)
        return NULL;

    // Read all the times
    size_t read_count = 0;
    procmetrix_error_t status =
        procmetrix_cpu_times_per_cpu(cpu_times, ncpus, &read_count);

    if (PROCMETRIX_ERROR_NONE != status)
    {
        PyMem_Free(cpu_times);
        return py_procmetrix_error(status);
    }

    // Create output list
    PyObject* list = PyList_New(read_count);
    if (NULL == list)
    {
        PyMem_Free(cpu_times);
        return NULL;
    }

    // Copy each item
    for (size_t i = 0; i < read_count; ++i)
    {
        // Allocate the item
        cpu_times_wrapper_t* wrapper =
            (cpu_times_wrapper_t*)PyObject_CallNoArgs(cpu_times_type);

        if (NULL == wrapper)
        {
            Py_DECREF(list);
            list = NULL;
            break;
        }

        // Perform copy
        memcpy(&wrapper->times, &cpu_times[i], sizeof(*cpu_times));
        PyList_SetItem(list, i, (PyObject*)wrapper);
    }

    // Cleanup
    PyMem_Free(cpu_times);

    return list;
}

static PyObject* cpu_times_delta(PyObject* self, PyObject* args)
{
    // Parse inputs
    PyObject* before = NULL;
    PyObject* after = NULL;

    if (!PyArg_ParseTuple(args, "OO", &before, &after))
        return NULL;

    if (1 != PyObject_IsInstance(before, cpu_times_type) ||
        1 != PyObject_IsInstance(after, cpu_times_type))
        return py_procmetrix_error(PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Allocate output
    cpu_times_wrapper_t* delta =
        (cpu_times_wrapper_t*)PyObject_CallNoArgs(cpu_times_type);
    if (NULL == delta)
        return NULL;

    // Calculate the delta
    procmetrix_error_t status = procmetrix_cpu_times_delta(
        &((cpu_times_wrapper_t*)before)->times,
        &((cpu_times_wrapper_t*)after)->times,
        &delta->times);

    if (PROCMETRIX_ERROR_NONE != status)
    {
        Py_DECREF(delta);
        return py_procmetrix_error(status);
    }

    return (PyObject*)delta;
}

static PyObject* cpu_times_sum(PyObject* self, PyObject* args)
{
    // Parse inputs
    PyObject* cpu_times = NULL;

    if (!PyArg_ParseTuple(args, "O", &cpu_times))
        return NULL;

    if (1 != PyObject_IsInstance(cpu_times, cpu_times_type))
        return py_procmetrix_error(PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Calculate sum
    return PyFloat_FromDouble(
        procmetrix_cpu_times_sum(&((cpu_times_wrapper_t*)cpu_times)->times));
}

static PyObject* cpu_utilization_ratio(PyObject* self, PyObject* args)
{
    // Parse inputs
    PyObject* cpu_times = NULL;

    if (!PyArg_ParseTuple(args, "O", &cpu_times))
        return NULL;

    if (1 != PyObject_IsInstance(cpu_times, cpu_times_type))
        return py_procmetrix_error(PROCMETRIX_ERROR_INVALID_ARGUMENT);

    return PyFloat_FromDouble(procmetrix_cpu_utilization_ratio(
        &((cpu_times_wrapper_t*)cpu_times)->times));
}

// ============================================================================
// CPU Freqs
// ============================================================================

typedef struct
{
    PyObject_HEAD;
    procmetrix_cpu_freq_t freqs;
} cpu_freq_wrapper_t;

static PyObject* cpu_freq_cur(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(((cpu_freq_wrapper_t*)self)->freqs.freq_cur);
}

static PyObject* cpu_freq_min(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(((cpu_freq_wrapper_t*)self)->freqs.freq_min);
}

static PyObject* cpu_freq_max(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(((cpu_freq_wrapper_t*)self)->freqs.freq_max);
}

static PyGetSetDef cpu_freq_getters[] = {
    // clang-format off
    { (char*)"cur", cpu_freq_cur },
    { (char*)"min", cpu_freq_min },
    { (char*)"max", cpu_freq_max },
    // clang-format on
    { NULL },
};

static PyType_Slot cpu_freq_slots[] = {
    { Py_tp_getset, cpu_freq_getters },
    { 0, NULL },
};

static PyType_Spec cpu_freq_spec = {
    "procmetrix.SystemCpuFreq",
    sizeof(cpu_freq_wrapper_t),
    0,
    Py_TPFLAGS_DEFAULT,
    cpu_freq_slots,
};

// assigned in the registration
static PyObject* cpu_freq_type = NULL;

static PyObject* cpu_freqs(PyObject* self, PyObject* args)
{
    // Get the number of CPUs
    size_t ncpus = procmetrix_cpu_count_logical();
    if (0 == ncpus)
        return py_procmetrix_error(PROCMETRIX_ERROR_UNKNOWN);

    // Alloc results array
    procmetrix_cpu_freq_t* cpu_freqs =
        PyMem_Calloc(ncpus, sizeof(procmetrix_cpu_freq_t));

    if (NULL == cpu_freqs)
        return NULL;

    // Read all the frequencies
    size_t read_count = 0;
    procmetrix_error_t status =
        procmetrix_cpu_freqs(cpu_freqs, ncpus, &read_count);

    if (PROCMETRIX_ERROR_NONE != status)
    {
        PyMem_Free(cpu_freqs);
        return py_procmetrix_error(status);
    }

    // Create output list
    PyObject* list = PyList_New(read_count);
    if (NULL == list)
    {
        PyMem_Free(cpu_freqs);
        return NULL;
    }

    // Copy each item
    for (size_t i = 0; i < read_count; ++i)
    {
        // Allocate the item
        cpu_freq_wrapper_t* wrapper =
            (cpu_freq_wrapper_t*)PyObject_CallNoArgs(cpu_freq_type);

        if (NULL == wrapper)
        {
            Py_DECREF(list);
            list = NULL;
            break;
        }

        // Perform copy
        memcpy(&wrapper->freqs, &cpu_freqs[i], sizeof(*cpu_freqs));
        PyList_SetItem(list, i, (PyObject*)wrapper);
    }

    // Cleanup
    PyMem_Free(cpu_freqs);

    return list;
}

static PyObject* cpu_freqs_average(PyObject* self, PyObject* args)
{
    // Validate input is a list
    PyObject* list = NULL;
    if (!PyArg_ParseTuple(args, "O", &list))
        return NULL;

    if (!PyList_Check(list))
        return py_procmetrix_error(PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Copy to the lib's format
    size_t count = PyList_Size(list);
    procmetrix_cpu_freq_t* cpu_freqs =
        PyMem_Calloc(count, sizeof(procmetrix_cpu_freq_t));

    if (NULL == cpu_freqs)
        return NULL;

    for (size_t i = 0; i < count; ++i)
    {
        // Get the item and check it's format
        PyObject* item = PyList_GetItem(list, i);
        if (1 != PyObject_IsInstance(item, cpu_freq_type))
        {
            PyMem_Free(cpu_freqs);
            return py_procmetrix_error(PROCMETRIX_ERROR_INVALID_ARGUMENT);
        }

        // Copy the element data
        memcpy(
            &cpu_freqs[i],
            &((cpu_freq_wrapper_t*)item)->freqs,
            sizeof(*cpu_freqs));
    }

    // Allocate the output
    cpu_freq_wrapper_t* wrapper =
        (cpu_freq_wrapper_t*)PyObject_CallNoArgs(cpu_freq_type);
    if (NULL == wrapper)
    {
        PyMem_Free(cpu_freqs);
        return NULL;
    }

    // Calculate the averages
    procmetrix_error_t status =
        procmetrix_cpu_freqs_average(cpu_freqs, count, &wrapper->freqs);

    // Cleanup
    PyMem_Free(cpu_freqs);

    if (PROCMETRIX_ERROR_NONE != status)
    {
        Py_DECREF(wrapper);
        return py_procmetrix_error(status);
    }

    return (PyObject*)wrapper;
}

static PyObject* cpu_freq_system(PyObject* self, PyObject* args)
{
    // Allocate the result
    cpu_freq_wrapper_t* wrapper =
        (cpu_freq_wrapper_t*)PyObject_CallNoArgs(cpu_freq_type);
    if (NULL == wrapper)
        return NULL;

    // Read system frequency
    procmetrix_error_t status = procmetrix_cpu_freq_system(&wrapper->freqs);

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

static PyMethodDef methods[] = {
    // CPU counts
    { "cpu_count_logical", cpu_count_logical, METH_NOARGS },
    { "cpu_count_physical", cpu_count_physical, METH_NOARGS },
    // CPU times
    { "cpu_times_total", system_cpu_times_total, METH_NOARGS },
    { "cpu_times_per_cpu", system_cpu_times_per_cpu, METH_NOARGS },
    // CPU times derived statistics
    { "cpu_times_delta", cpu_times_delta, METH_VARARGS },
    { "cpu_times_sum", cpu_times_sum, METH_VARARGS },
    { "cpu_utilization_ratio", cpu_utilization_ratio, METH_VARARGS },
    // CPU Frequencies
    { "cpu_freqs", cpu_freqs, METH_NOARGS },
    { "cpu_freqs_average", cpu_freqs_average, METH_VARARGS },
    { "cpu_freq_system", cpu_freq_system, METH_NOARGS },
    // End of list
    { NULL, NULL, 0 }
};

void pyprocmetrix_init_cpu(PyObject* module)
{
    // Register types
    cpu_times_type = PyType_FromSpec(&cpu_times_spec);
    PyModule_AddObjectRef(module, "SystemCpuTimes", cpu_times_type);

    cpu_freq_type = PyType_FromSpec(&cpu_freq_spec);
    PyModule_AddObjectRef(module, "SystemCpuFreq", cpu_freq_type);

    // Register methods
    PyModule_AddFunctions(module, methods);
}
