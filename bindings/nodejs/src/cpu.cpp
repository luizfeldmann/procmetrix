// Bindings
#include "error.h"
#include "init.h"

// Lib
#include <procmetrix/cpu.h>

// STD
#include <vector>

// Impl

// CPU counts

static Napi::Value cpu_count_physical(const Napi::CallbackInfo& info)
{
    size_t ncpu = procmetrix_cpu_count_physical();

    Napi::Env env = info.Env();
    return Napi::Number::From(env, ncpu);
}

static Napi::Value cpu_count_logical(const Napi::CallbackInfo& info)
{
    size_t ncpu = procmetrix_cpu_count_logical();

    Napi::Env env = info.Env();
    return Napi::Number::From(env, ncpu);
}

// CPU times

struct cpu_time_field
{
    const char* name;
    double procmetrix_cpu_times_t::* member;
};

static constexpr cpu_time_field g_fields_cpu_times[] = {
    { "user", &procmetrix_cpu_times_t::user },
    { "system", &procmetrix_cpu_times_t::system },
    { "idle", &procmetrix_cpu_times_t::idle },
    { "nice", &procmetrix_cpu_times_t::nice },
    { "iowait", &procmetrix_cpu_times_t::iowait },
    { "irq", &procmetrix_cpu_times_t::irq },
    { "softirq", &procmetrix_cpu_times_t::softirq },
    { "steal", &procmetrix_cpu_times_t::steal },
    { "guest", &procmetrix_cpu_times_t::guest },
    { "guestNice", &procmetrix_cpu_times_t::guest_nice },
    { "interrupt", &procmetrix_cpu_times_t::interrupt },
    { "dpc", &procmetrix_cpu_times_t::dpc },
};

static Napi::Object
to_js_cpu_times(Napi::Env env, procmetrix_cpu_times_t const& cpu_times)
{
    Napi::Object obj = Napi::Object::New(env);

    for (const auto& field : g_fields_cpu_times)
        obj.Set(field.name, cpu_times.*(field.member));

    return obj;
}

static void
from_js_cpu_times(Napi::Object const& obj, procmetrix_cpu_times_t& cpu_times)
{
    for (const auto& field : g_fields_cpu_times)
        cpu_times.*(field.member) =
            obj.Get(field.name).As<Napi::Number>().DoubleValue();
}

static Napi::Object cpu_times_total(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    // Read CPU times
    procmetrix_cpu_times_t cpu_times { 0 };
    procmetrix_error_t status = procmetrix_cpu_times_total(&cpu_times);

    if (PROCMETRIX_ERROR_NONE != status)
        throw make_error(env, status);

    // Convert
    return to_js_cpu_times(env, cpu_times);
}

static Napi::Object cpu_times_per_cpu(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    // Get the number of CPUs
    size_t ncpus = procmetrix_cpu_count_logical();
    if (0 == ncpus)
        throw make_error(env, PROCMETRIX_ERROR_UNKNOWN);

    // Read the CPU times
    std::vector<procmetrix_cpu_times_t> cpu_times(ncpus, { 0 });

    size_t read_count = 0;
    procmetrix_error_t status = procmetrix_cpu_times_per_cpu(
        cpu_times.data(), cpu_times.size(), &read_count);

    if (PROCMETRIX_ERROR_NONE != status)
        throw make_error(env, status);

    // Convert list
    Napi::Array array = Napi::Array::New(env, read_count);

    for (size_t i = 0; i < read_count; ++i)
        array.Set(i, to_js_cpu_times(env, cpu_times[i]));

    return array;
}

static Napi::Object cpu_times_delta(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    // Get arguments
    procmetrix_cpu_times_t before { 0 };
    procmetrix_cpu_times_t after { 0 };

    from_js_cpu_times(info[0].As<Napi::Object>(), before);
    from_js_cpu_times(info[1].As<Napi::Object>(), after);

    // Calc delta
    procmetrix_cpu_times_t delta { 0 };
    procmetrix_error_t status =
        procmetrix_cpu_times_delta(&before, &after, &delta);

    if (PROCMETRIX_ERROR_NONE != status)
        throw make_error(env, status);

    return to_js_cpu_times(env, delta);
}

static Napi::Value cpu_times_sum(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    procmetrix_cpu_times_t cpu_times { 0 };
    from_js_cpu_times(info[0].As<Napi::Object>(), cpu_times);

    const auto sum = procmetrix_cpu_times_sum(&cpu_times);
    return Napi::Number::From(env, sum);
}

static Napi::Value cpu_utilization_ratio(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    procmetrix_cpu_times_t cpu_times { 0 };
    from_js_cpu_times(info[0].As<Napi::Object>(), cpu_times);

    const auto ratio = procmetrix_cpu_utilization_ratio(&cpu_times);
    return Napi::Number::From(env, ratio);
}

// Init

void init_cpu(Napi::Env env, Napi::Object exports)
{
    // CPU counts
    exports.Set(
        "cpu_count_physical", Napi::Function::New(env, cpu_count_physical));

    exports.Set(
        "cpu_count_logical", Napi::Function::New(env, cpu_count_logical));

    // CPU Times
    exports.Set("cpu_times_total", Napi::Function::New(env, cpu_times_total));

    exports.Set(
        "cpu_times_per_cpu", Napi::Function::New(env, cpu_times_per_cpu));

    exports.Set("cpu_times_delta", Napi::Function::New(env, cpu_times_delta));

    exports.Set("cpu_times_sum", Napi::Function::New(env, cpu_times_sum));

    exports.Set(
        "cpu_utilization_ratio",
        Napi::Function::New(env, cpu_utilization_ratio));
}
