#ifndef PROCMETRIX_NODEJS_INIT_H
#define PROCMETRIX_NODEJS_INIT_H

// Node N-API
#include <napi.h>

//! Initializes the CPU functions
void init_cpu(Napi::Env env, Napi::Object exports);

//! Initializes the memory functions
void init_mem(Napi::Env env, Napi::Object exports);

#endif // PROCMETRIX_NODEJS_INIT_H
