// Bindings
#include "error.h"
#include "init.h"

// Lib
#include <procmetrix/mem.h>

// ============================================================================
// Virtual Memory
// ============================================================================

typedef struct
{
    PyObject_HEAD;
    procmetrix_virtual_memory_t vmem;
} virtual_memory_wrapper_t;

static PyObject* vm_total(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((virtual_memory_wrapper_t*)self)->vmem.total);
}

static PyObject* vm_available(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((virtual_memory_wrapper_t*)self)->vmem.available);
}

static PyObject* vm_used(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((virtual_memory_wrapper_t*)self)->vmem.used);
}

static PyObject* vm_free(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((virtual_memory_wrapper_t*)self)->vmem.free);
}

static PyObject* vm_active(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((virtual_memory_wrapper_t*)self)->vmem.active);
}

static PyObject* vm_inactive(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((virtual_memory_wrapper_t*)self)->vmem.inactive);
}

static PyObject* vm_buffers(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((virtual_memory_wrapper_t*)self)->vmem.buffers);
}

static PyObject* vm_cached(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((virtual_memory_wrapper_t*)self)->vmem.cached);
}

static PyObject* vm_shared(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((virtual_memory_wrapper_t*)self)->vmem.shared);
}

static PyObject* vm_slab(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((virtual_memory_wrapper_t*)self)->vmem.slab);
}

static PyObject* vm_ratio(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(((virtual_memory_wrapper_t*)self)->vmem.ratio);
}

static PyGetSetDef virtual_memory_getters[] = {
    // clang-format off
    { (char*)"total",       vm_total },
    { (char*)"available",   vm_available },
    { (char*)"used",        vm_used },
    { (char*)"free",        vm_free },
    { (char*)"active",      vm_active },
    { (char*)"inactive",    vm_inactive },
    { (char*)"buffers",     vm_buffers },
    { (char*)"cached",      vm_cached },
    { (char*)"shared",      vm_shared },
    { (char*)"slab",        vm_slab },
    { (char*)"ratio",       vm_ratio },
    // clang-format on
    { NULL },
};

static PyType_Slot virtual_memory_slots[] = {
    { Py_tp_getset, virtual_memory_getters },
    { 0, nullptr },
};

static PyType_Spec virtual_memory_spec = {
    "procmetrix.SystemVirtualMemory",
    sizeof(virtual_memory_wrapper_t),
    0,
    Py_TPFLAGS_DEFAULT,
    virtual_memory_slots,
};

// assigned in the registration
static PyObject* virtual_memory_type = NULL;

static PyObject* system_virtual_memory(PyObject* self, PyObject* args)
{
    virtual_memory_wrapper_t* wrapper =
        (virtual_memory_wrapper_t*)PyObject_CallNoArgs(virtual_memory_type);

    if (NULL == wrapper)
        return NULL;

    // Read the virtual memory
    procmetrix_error_t status =
        procmetrix_system_virtual_memory(&wrapper->vmem);

    if (PROCMETRIX_ERROR_NONE != status)
    {
        Py_DECREF(wrapper);
        return py_procmetrix_error(status);
    }

    // Return dict
    return (PyObject*)wrapper;
}

// ============================================================================
// Swap Memory
// ============================================================================

typedef struct
{
    PyObject_HEAD;
    procmetrix_swap_memory_t smem;
} swap_memory_wrapper_t;

static PyObject* swap_total(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((swap_memory_wrapper_t*)self)->smem.total);
}

static PyObject* swap_free(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((swap_memory_wrapper_t*)self)->smem.free);
}

static PyObject* swap_used(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((swap_memory_wrapper_t*)self)->smem.used);
}

static PyObject* swap_in(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((swap_memory_wrapper_t*)self)->smem.swap_in);
}

static PyObject* swap_out(PyObject* self, void* closure)
{
    return PyLong_FromUnsignedLongLong(
        ((swap_memory_wrapper_t*)self)->smem.swap_out);
}

static PyObject* swap_ratio(PyObject* self, void* closure)
{
    return PyFloat_FromDouble(((swap_memory_wrapper_t*)self)->smem.ratio);
}

static PyGetSetDef swap_memory_getters[] = {
    // clang-format off
    { (char*)"total",       swap_total },
    { (char*)"used",        swap_used },
    { (char*)"free",        swap_free },
    { (char*)"swap_in",     swap_in },
    { (char*)"swap_out",    swap_out },
    { (char*)"ratio",       swap_ratio },
    // clang-format on
    { NULL },
};

static PyType_Slot swap_memory_slots[] = {
    { Py_tp_getset, swap_memory_getters },
    { 0, nullptr },
};

static PyType_Spec swap_memory_spec = {
    "procmetrix.SystemSwapMemory",
    sizeof(swap_memory_wrapper_t),
    0,
    Py_TPFLAGS_DEFAULT,
    swap_memory_slots,
};

// assigned in the registration
static PyObject* swap_memory_type = NULL;

static PyObject* system_swap_memory(PyObject* self, PyObject* args)
{
    // Allocate object
    swap_memory_wrapper_t* wrapper =
        (swap_memory_wrapper_t*)PyObject_CallNoArgs(swap_memory_type);

    if (NULL == wrapper)
        return NULL;

    // Read swap memory
    procmetrix_error_t status = procmetrix_system_swap_memory(&wrapper->smem);

    if (PROCMETRIX_ERROR_NONE != status)
    {
        Py_DECREF(wrapper);
        return py_procmetrix_error(status);
    }

    // Return dict
    return (PyObject*)wrapper;
}

// ============================================================================
// Registration
// ============================================================================

static PyMethodDef methods[] = {
    // System memory
    { "virtual_memory", system_virtual_memory, METH_NOARGS },
    { "swap_memory", system_swap_memory, METH_NOARGS },
    // End of list
    { NULL, NULL, 0 }
};

void pyprocmetrix_init_mem(PyObject* module)
{
    // Register types
    virtual_memory_type = PyType_FromSpec(&virtual_memory_spec);
    PyModule_AddObjectRef(module, "SystemVirtualMemory", virtual_memory_type);

    swap_memory_type = PyType_FromSpec(&swap_memory_spec);
    PyModule_AddObjectRef(module, "SystemSwapMemory", swap_memory_type);

    // Register methods
    PyModule_AddFunctions(module, methods);
}
