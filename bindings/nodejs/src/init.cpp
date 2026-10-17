#include <napi.h>

Napi::Value Add(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    if (info.Length() != 2)
        throw Napi::TypeError::New(env, "Expected 2 arguments");

    double a = info[0].As<Napi::Number>().DoubleValue();
    double b = info[1].As<Napi::Number>().DoubleValue();

    return Napi::Number::New(env, a + b);
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set("add", Napi::Function::New(env, Add));
    return exports;
}

NODE_API_MODULE(procmetrix, Init)
