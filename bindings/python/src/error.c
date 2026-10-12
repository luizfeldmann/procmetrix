// Local
#include "error.h"

PyObject* py_procmetrix_error(procmetrix_error_t err)
{
    // Translate error codes to python errors
    switch (err)
    {
    default:
    case PROCMETRIX_ERROR_UNKNOWN:
        PyErr_SetString(PyExc_RuntimeError, "unknown");
        break;

    case PROCMETRIX_ERROR_INVALID_ARGUMENT:
        PyErr_SetString(PyExc_ValueError, "invalid argument");
        break;

    case PROCMETRIX_ERROR_MORE_DATA:
        PyErr_SetString(PyExc_OverflowError, "more data");
        break;

    case PROCMETRIX_ERROR_FILE_READ:
        PyErr_SetString(PyExc_RuntimeError, "unable to read file");
        break;

    case PROCMETRIX_ERROR_MALFORMED:
        PyErr_SetString(PyExc_RuntimeError, "malformed contents");
        break;

    case PROCMETRIX_ERROR_OUT_OF_MEMORY:
        PyErr_SetString(PyExc_MemoryError, "out of memory");
        break;

    case PROCMETRIX_NOT_IMPLEMENTED:
        PyErr_SetString(PyExc_NotImplementedError, "not implemented");
        break;

    case PROCMETRIX_ERROR_NONE:
        /** Not an error */
        break;
    }

    return NULL;
}