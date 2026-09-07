//! @file
//! @ingroup version
//! @brief Library version information.

#ifndef _PROCMETRIX_VERSION_H_
#define _PROCMETRIX_VERSION_H_

// Lib
#include <procmetrix/api.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
    //! @addtogroup version
    //! @{

    //! Reads the major version number
    PROCMETRIX_API unsigned int procmetrix_version_major(void);

    //! Reads the minor version number
    PROCMETRIX_API unsigned int procmetrix_version_minor(void);

    //! Reads the patch version number
    PROCMETRIX_API unsigned int procmetrix_version_patch(void);

    //! @}
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PROCMETRIX_VERSION_H_
