#ifndef _PROCMETRIX_API_H_
#define _PROCMETRIX_API_H_

#ifdef _WIN32
    #ifdef PROCMETRIX_SHARED
        #ifdef PROCMETRIX_BUILDING_LIBRARY
            #define PROCMETRIX_API __declspec(dllexport)
        #else
            #define PROCMETRIX_API __declspec(dllimport)
        #endif // PROCMETRIX_BUILDING_LIBRARY
    #else
        #define PROCMETRIX_API
    #endif // PROCMETRIX_SHARED
#else
    #define PROCMETRIX_API
#endif // _WIN32

#endif // _PROCMETRIX_API_H_
