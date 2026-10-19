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
    using cpu_time_member_t = double procmetrix_cpu_times_t::*;

    const char* name;
    cpu_time_member_t member;
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
convert_cpu_times(Napi::Env env, procmetrix_cpu_times_t const& cpu_times)
{
    Napi::Object obj = Napi::Object::New(env);

    for (const auto& field : g_fields_cpu_times)
        obj.Set(field.name, cpu_times.*(field.member));

    return obj;
}

static void
convert_cpu_times(Napi::Object const& obj, procmetrix_cpu_times_t& cpu_times)
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
    return convert_cpu_times(env, cpu_times);
}

static Napi::Array cpu_times_per_cpu(const Napi::CallbackInfo& info)
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
        array.Set(i, convert_cpu_times(env, cpu_times[i]));

    return array;
}

static Napi::Object cpu_times_delta(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    // Get arguments
    procmetrix_cpu_times_t before { 0 };
    procmetrix_cpu_times_t after { 0 };

    convert_cpu_times(info[0].As<Napi::Object>(), before);
    convert_cpu_times(info[1].As<Napi::Object>(), after);

    // Calc delta
    procmetrix_cpu_times_t delta { 0 };
    procmetrix_error_t status =
        procmetrix_cpu_times_delta(&before, &after, &delta);

    if (PROCMETRIX_ERROR_NONE != status)
        throw make_error(env, status);

    return convert_cpu_times(env, delta);
}

static Napi::Value cpu_times_sum(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    procmetrix_cpu_times_t cpu_times { 0 };
    convert_cpu_times(info[0].As<Napi::Object>(), cpu_times);

    const auto sum = procmetrix_cpu_times_sum(&cpu_times);
    return Napi::Number::From(env, sum);
}

static Napi::Value cpu_utilization_ratio(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    procmetrix_cpu_times_t cpu_times { 0 };
    convert_cpu_times(info[0].As<Napi::Object>(), cpu_times);

    const auto ratio = procmetrix_cpu_utilization_ratio(&cpu_times);
    return Napi::Number::From(env, ratio);
}

// CPU freqs

struct cpu_freq_field
{
    using cpu_freq_member_t = double procmetrix_cpu_freq_t::*;

    const char* name;
    cpu_freq_member_t member;
};

static constexpr cpu_freq_field g_fields_cpu_freq[] = {
    { "cur", &procmetrix_cpu_freq_t::freq_cur },
    { "min", &procmetrix_cpu_freq_t::freq_min },
    { "max", &procmetrix_cpu_freq_t::freq_max },
};

static Napi::Object
convert_cpu_freq(Napi::Env env, procmetrix_cpu_freq_t const& cpu_freq)
{
    Napi::Object obj = Napi::Object::New(env);

    for (const auto& field : g_fields_cpu_freq)
        obj.Set(field.name, cpu_freq.*(field.member));

    return obj;
}

static void
convert_cpu_freq(Napi::Object const& obj, procmetrix_cpu_freq_t& cpu_freq)
{
    for (const auto& field : g_fields_cpu_freq)
        cpu_freq.*(field.member) =
            obj.Get(field.name).As<Napi::Number>().DoubleValue();
}

static Napi::Array cpu_freqs(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    // Get number of cpus
    size_t ncpus = procmetrix_cpu_count_logical();
    if (0 == ncpus)
        throw make_error(env, PROCMETRIX_ERROR_UNKNOWN);

    // Read the CPU freqs
    std::vector<procmetrix_cpu_freq_t> cpu_freqs(ncpus, { 0 });

    size_t read_count = 0;
    procmetrix_error_t status =
        procmetrix_cpu_freqs(cpu_freqs.data(), cpu_freqs.size(), &read_count);

    if (PROCMETRIX_ERROR_NONE != status)
        throw make_error(env, status);

    // Convert list
    Napi::Array array = Napi::Array::New(env, read_count);

    for (size_t i = 0; i < read_count; ++i)
        array.Set(i, convert_cpu_freq(env, cpu_freqs[i]));

    return array;
}

static Napi::Object cpu_freqs_average(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    // Convert inputs
    Napi::Array input_array = info[0].As<Napi::Array>();
    std::vector<procmetrix_cpu_freq_t> cpu_freqs(input_array.Length(), { 0 });

    for (size_t i = 0; i < input_array.Length(); ++i)
    {
        const auto input_item = input_array[i].AsValue().As<Napi::Object>();
        convert_cpu_freq(input_item, cpu_freqs[i]);
    }

    // Calculate the average
    procmetrix_cpu_freq_t average { 0 };
    procmetrix_error_t status = procmetrix_cpu_freqs_average(
        cpu_freqs.data(), cpu_freqs.size(), &average);

    if (PROCMETRIX_ERROR_NONE != status)
        throw make_error(env, status);

    return convert_cpu_freq(env, average);
}

static Napi::Object cpu_freq_system(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    procmetrix_cpu_freq_t system_freq { 0 };
    procmetrix_error_t status = procmetrix_cpu_freq_system(&system_freq);

    if (PROCMETRIX_ERROR_NONE != status)
        throw make_error(env, status);

    return convert_cpu_freq(env, system_freq);
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

    // CPU frequencies
    exports.Set("cpu_freqs", Napi::Function::New(env, cpu_freqs));

    exports.Set(
        "cpu_freqs_average", Napi::Function::New(env, cpu_freqs_average));

    exports.Set("cpu_freq_system", Napi::Function::New(env, cpu_freq_system));
}
