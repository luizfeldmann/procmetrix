// Bindings
#include "error.h"

Napi::Error make_error(Napi::Env env, procmetrix_error_t err)
{
    const char* errmsg = procmetrix_error_message(err);
    return Napi::Error::New(env, errmsg);
}
