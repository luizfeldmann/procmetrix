#ifndef PY_PROCMETRIX_INIT_H
#define PY_PROCMETRIX_INIT_H

// Python
#define PY_SSIZE_T_CLEAN
#include <Python.h>

// Statuses
#define PY_INIT_OK (0)
#define PY_INIT_FAIL (-1)

//! Index of types registed by the module
typedef enum py_procmetrix_type
{
    PROCMETRIX_TYPE_VIRTUAL_MEMORY,
    PROCMETRIX_TYPE_SWAP_MEMORY,
    PROCMETRIX_TYPE_CPU_TIMES,
    PROCMETRIX_TYPE_CPU_FREQS,
    // MUST BE LAST:
    PROCMETRIX_TYPE_COUNT,
} py_procmetrix_type_t;

//! Module initialization state
typedef struct py_procmetrix_module_state
{
    PyObject* types[PROCMETRIX_TYPE_COUNT];
} py_procmetrix_module_state_t;

//! Registers a type into the module and into the state
int pyprocmetrix_register_type(
    PyObject* module,
    py_procmetrix_module_state_t* state,
    const char* type_name,
    py_procmetrix_type_t type_id,
    PyType_Spec* type_spec);

//! Initializes the memory-related methods and types
//! @private
int pyprocmetrix_init_mem(PyObject* module);

//! Initializes the CPU-related types and types
//! @private
int pyprocmetrix_init_cpu(PyObject* module);

//! Initializes the process-related types and types
//! @private
int pyprocmetrix_init_proc(PyObject* module);

#endif // PY_PROCMETRIX_INIT_H
