#include "init.h"

Napi::Object init(Napi::Env env, Napi::Object exports)
{
    init_cpu(env, exports);
    return exports;
}

NODE_API_MODULE(procmetrix, init)
