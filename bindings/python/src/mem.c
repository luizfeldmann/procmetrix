// Bindings
#include "error.h"
#include "init.h"

// Lib
#include <procmetrix/mem.h>

// ============================================================================
// Virtual Memory
// ============================================================================

typedef struct virtual_memory_wrapper
{
    PyObject_HEAD
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

static PyGetSetDef g_getters_virtual_memory[] = {
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

static PyType_Slot g_slots_virtual_memory[] = {
    { Py_tp_getset, g_getters_virtual_memory },
    { 0, NULL },
};

static PyType_Spec g_type_spec_virtual_memory = {
    "procmetrix.SystemVirtualMemory",
    sizeof(virtual_memory_wrapper_t),
    0,
    Py_TPFLAGS_DEFAULT,
    g_slots_virtual_memory,
};

static PyObject* system_virtual_memory(PyObject* self, PyObject* args)
{
    py_procmetrix_module_state_t* state = PyModule_GetState(self);

    virtual_memory_wrapper_t* wrapper =
        (virtual_memory_wrapper_t*)PyObject_CallNoArgs(
            state->types[PROCMETRIX_TYPE_VIRTUAL_MEMORY]);

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

    return (PyObject*)wrapper;
}

// ============================================================================
// Swap Memory
// ============================================================================

typedef struct swap_memory_wrapper
{
    PyObject_HEAD
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

static PyGetSetDef g_getters_swap_memory[] = {
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

static PyType_Slot g_slots_swap_memory[] = {
    { Py_tp_getset, g_getters_swap_memory },
    { 0, NULL },
};

static PyType_Spec g_type_spec_swap_memory = {
    "procmetrix.SystemSwapMemory",
    sizeof(swap_memory_wrapper_t),
    0,
    Py_TPFLAGS_DEFAULT,
    g_slots_swap_memory,
};

static PyObject* system_swap_memory(PyObject* self, PyObject* args)
{
    py_procmetrix_module_state_t* state = PyModule_GetState(self);

    // Allocate object
    swap_memory_wrapper_t* wrapper =
        (swap_memory_wrapper_t*)PyObject_CallNoArgs(
            state->types[PROCMETRIX_TYPE_SWAP_MEMORY]);

    if (NULL == wrapper)
        return NULL;

    // Read swap memory
    procmetrix_error_t status = procmetrix_system_swap_memory(&wrapper->smem);

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

static PyMethodDef g_methods_mem[] = {
    // System memory
    { "virtual_memory", system_virtual_memory, METH_NOARGS },
    { "swap_memory", system_swap_memory, METH_NOARGS },
    // End of list
    { NULL, NULL, 0 }
};

int pyprocmetrix_init_mem(PyObject* module)
{
    py_procmetrix_module_state_t* state = PyModule_GetState(module);

    // Register types
    if (pyprocmetrix_register_type(
            module,
            state,
            "SystemVirtualMemory",
            PROCMETRIX_TYPE_VIRTUAL_MEMORY,
            &g_type_spec_virtual_memory) != PY_INIT_OK)
        return PY_INIT_FAIL;

    if (pyprocmetrix_register_type(
            module,
            state,
            "SystemSwapMemory",
            PROCMETRIX_TYPE_SWAP_MEMORY,
            &g_type_spec_swap_memory) != PY_INIT_OK)
        return PY_INIT_FAIL;

    // Register methods
    if (PyModule_AddFunctions(module, g_methods_mem) != PY_INIT_OK)
        return PY_INIT_FAIL;

    return PY_INIT_OK;
}
