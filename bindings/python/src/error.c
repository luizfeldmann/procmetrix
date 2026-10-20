// Local
#include "error.h"

PyObject* py_procmetrix_error(procmetrix_error_t err)
{
    const char* errmsg = procmetrix_error_message(err);

    // Translate error codes to python errors
    switch (err)
    {
    default:
    case PROCMETRIX_ERROR_UNKNOWN:
    case PROCMETRIX_ERROR_FILE_READ:
    case PROCMETRIX_ERROR_MALFORMED:
        PyErr_SetString(PyExc_RuntimeError, errmsg);
        break;

    case PROCMETRIX_ERROR_INVALID_ARGUMENT:
        PyErr_SetString(PyExc_ValueError, errmsg);
        break;

    case PROCMETRIX_ERROR_MORE_DATA:
        PyErr_SetString(PyExc_OverflowError, errmsg);
        break;

    case PROCMETRIX_ERROR_OUT_OF_MEMORY:
        PyErr_SetString(PyExc_MemoryError, errmsg);
        break;

    case PROCMETRIX_NOT_IMPLEMENTED:
        PyErr_SetString(PyExc_NotImplementedError, errmsg);
        break;

    case PROCMETRIX_ERROR_NONE:
        /** Not an error */
        break;
    }

    return NULL;
}