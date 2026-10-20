#ifndef PROCMETRIX_NODEJS_ERROR_H
#define PROCMETRIX_NODEJS_ERROR_H

// Node N-API
#include <napi.h>

// Lib
#include <procmetrix/error.h>

//! Converts a library error to Node error
Napi::Error make_error(Napi::Env env, procmetrix_error_t err);

#endif // PROCMETRIX_NODEJS_ERROR_H
