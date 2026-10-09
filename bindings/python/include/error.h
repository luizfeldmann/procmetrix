#ifndef PY_PROCMETRIX_ERROR_H
#define PY_PROCMETRIX_ERROR_H

//  Python
#define PY_SSIZE_T_CLEAN
#include <Python.h>

// Lib
#include <procmetrix/error.h>

PyObject* py_procmetrix_error(procmetrix_error_t err);

#endif
