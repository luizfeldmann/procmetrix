// Bindings
#include "init.h"

// Lib
#include <procmetrix/version.h>

// STD
#include <assert.h>

int pyprocmetrix_register_type(
    PyObject* module,
    py_procmetrix_module_state_t* state,
    const char* type_name,
    py_procmetrix_type_t type_id,
    PyType_Spec* type_spec)
{
    // Avoid double registration
    assert(state->types[type_id] == NULL);

    // Create the type from the spec
    PyObject* type = PyType_FromSpec(type_spec);
    if (!type)
        return PY_INIT_FAIL;

    // Register to the module
    if (PyModule_AddObjectRef(module, type_name, type) != PY_INIT_OK)
    {
        Py_DECREF(type);
        return PY_INIT_FAIL;
    }

    // Register in the state
    state->types[type_id] = type;
    return PY_INIT_OK;
}

//! Multi-phase initialization
static int pyprocmetrix_exec(PyObject* module)
{
    // Add version constants
    if (PyModule_AddIntConstant(
            module, "version_major", procmetrix_version_major()) != PY_INIT_OK)
        return PY_INIT_FAIL;

    if (PyModule_AddIntConstant(
            module, "version_minor", procmetrix_version_minor()) != PY_INIT_OK)
        return PY_INIT_FAIL;

    if (PyModule_AddIntConstant(
            module, "version_patch", procmetrix_version_patch()) != PY_INIT_OK)
        return PY_INIT_FAIL;

    // Register each topic
    if (pyprocmetrix_init_mem(module) != PY_INIT_OK)
        return PY_INIT_FAIL;

    if (pyprocmetrix_init_cpu(module) != PY_INIT_OK)
        return PY_INIT_FAIL;

    if (pyprocmetrix_init_proc(module) != PY_INIT_OK)
        return PY_INIT_FAIL;

    // Success initializing the module
    return PY_INIT_OK;
}

//! GC visitation
static int procmetrix_traverse(PyObject* module, visitproc visit, void* arg)
{
    py_procmetrix_module_state_t* state = PyModule_GetState(module);

    // Visit all types
    for (size_t i = 0; i < PROCMETRIX_TYPE_COUNT; ++i)
        Py_VISIT(state->types[i]);

    return PY_INIT_OK;
}

//! Cleanup of module state
static int procmetrix_clear(PyObject* module)
{
    py_procmetrix_module_state_t* state = PyModule_GetState(module);

    // Clear all types
    for (size_t i = 0; i < PROCMETRIX_TYPE_COUNT; ++i)
        Py_CLEAR(state->types[i]);

    return PY_INIT_OK;
}

//! Module slots
static struct PyModuleDef_Slot g_module_slots[] = {
    { Py_mod_exec, pyprocmetrix_exec },
    // Null terminated list
    { 0, NULL },
};

//! Module definition
static PyModuleDef g_module_def = {
    PyModuleDef_HEAD_INIT,                // m_base
    "pyprocmetrix",                       // m_name
    NULL,                                 // m_doc
    sizeof(py_procmetrix_module_state_t), // m_size
    NULL,                                 // m_methods
    g_module_slots,                       // m_slots
    procmetrix_traverse,                  // m_traverse
    procmetrix_clear,                     // m_clear
};

//! Module entry-point
//! NOLINTNEXTLINE(readability-identifier-naming)
PyMODINIT_FUNC PyInit_pyprocmetrix()
{
    return PyModuleDef_Init(&g_module_def);
}
