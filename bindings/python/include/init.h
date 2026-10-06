#ifndef PY_PROCMETRIX_INIT_H
#define PY_PROCMETRIX_INIT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

//! Initializes the memory-related methods and types
void pyprocmetrix_init_mem(PyObject* module);

//! Initializes the CPU-related types and types
void pyprocmetrix_init_cpu(PyObject* module);

#endif // PY_PROCMETRIX_INIT_H
