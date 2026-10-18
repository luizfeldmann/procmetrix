// Lib
#include <procmetrix/error.h>

// STD
#include <assert.h>

const char* procmetrix_error_message(procmetrix_error_t err)
{
    // Translate error codes to python errors
    switch (err)
    {
    default:
    case PROCMETRIX_ERROR_UNKNOWN:
        return "unknown";
        break;

    case PROCMETRIX_ERROR_INVALID_ARGUMENT:
        return "invalid argument";
        break;

    case PROCMETRIX_ERROR_MORE_DATA:
        return "more data";
        break;

    case PROCMETRIX_ERROR_FILE_READ:
        return "unable to read file";
        break;

    case PROCMETRIX_ERROR_MALFORMED:
        return "malformed contents";
        break;

    case PROCMETRIX_ERROR_OUT_OF_MEMORY:
        return "out of memory";
        break;

    case PROCMETRIX_NOT_IMPLEMENTED:
        return "not implemented";
        break;

    case PROCMETRIX_ERROR_NONE:
        return "no error";
        break;
    }

    // unreachable
    assert(0 && "invalid error");
}
