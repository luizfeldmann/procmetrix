// Bindings
#include "error.h"
#include "init.h"
#include "reflection.h"

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

template <>
struct reflection<procmetrix_cpu_times_t>
{
    using field = member_field<procmetrix_cpu_times_t, double>;

    template <typename visitor_type>
    static void accept(visitor_type&& visitor)
    {
        visitor(field { "user", &procmetrix_cpu_times_t::user });
        visitor(field { "system", &procmetrix_cpu_times_t::system });
        visitor(field { "idle", &procmetrix_cpu_times_t::idle });
        visitor(field { "nice", &procmetrix_cpu_times_t::nice });
        visitor(field { "iowait", &procmetrix_cpu_times_t::iowait });
        visitor(field { "irq", &procmetrix_cpu_times_t::irq });
        visitor(field { "softirq", &procmetrix_cpu_times_t::softirq });
        visitor(field { "steal", &procmetrix_cpu_times_t::steal });
        visitor(field { "guest", &procmetrix_cpu_times_t::guest });
        visitor(field { "guestNice", &procmetrix_cpu_times_t::guest_nice });
        visitor(field { "interrupt", &procmetrix_cpu_times_t::interrupt });
        visitor(field { "dpc", &procmetrix_cpu_times_t::dpc });
    }
};

static Napi::Object cpu_times_total(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    // Read CPU times
    procmetrix_cpu_times_t cpu_times { 0 };
    procmetrix_error_t status = procmetrix_cpu_times_total(&cpu_times);

    if (PROCMETRIX_ERROR_NONE != status)
        throw make_error(env, status);

    // Convert
    return serialize(env, cpu_times);
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
        array.Set(i, serialize(env, cpu_times[i]));

    return array;
}

static Napi::Object cpu_times_delta(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    // Get arguments
    procmetrix_cpu_times_t before { 0 };
    procmetrix_cpu_times_t after { 0 };

    parse(info[0].As<Napi::Object>(), before);
    parse(info[1].As<Napi::Object>(), after);

    // Calc delta
    procmetrix_cpu_times_t delta { 0 };
    procmetrix_error_t status =
        procmetrix_cpu_times_delta(&before, &after, &delta);

    if (PROCMETRIX_ERROR_NONE != status)
        throw make_error(env, status);

    return serialize(env, delta);
}

static Napi::Value cpu_times_sum(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    procmetrix_cpu_times_t cpu_times { 0 };
    parse(info[0].As<Napi::Object>(), cpu_times);

    const auto sum = procmetrix_cpu_times_sum(&cpu_times);
    return Napi::Number::From(env, sum);
}

static Napi::Value cpu_utilization_ratio(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    procmetrix_cpu_times_t cpu_times { 0 };
    parse(info[0].As<Napi::Object>(), cpu_times);

    const auto ratio = procmetrix_cpu_utilization_ratio(&cpu_times);
    return Napi::Number::From(env, ratio);
}

// CPU freqs

template <>
struct reflection<procmetrix_cpu_freq_t>
{
    using field = member_field<procmetrix_cpu_freq_t, double>;

    template <typename visitor_type>
    static void accept(visitor_type&& visitor)
    {
        visitor(field { "cur", &procmetrix_cpu_freq_t::freq_cur });
        visitor(field { "min", &procmetrix_cpu_freq_t::freq_min });
        visitor(field { "max", &procmetrix_cpu_freq_t::freq_max });
    }
};

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
        array.Set(i, serialize(env, cpu_freqs[i]));

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
        parse(input_item, cpu_freqs[i]);
    }

    // Calculate the average
    procmetrix_cpu_freq_t average { 0 };
    procmetrix_error_t status = procmetrix_cpu_freqs_average(
        cpu_freqs.data(), cpu_freqs.size(), &average);

    if (PROCMETRIX_ERROR_NONE != status)
        throw make_error(env, status);

    return serialize(env, average);
}

static Napi::Object cpu_freq_system(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    procmetrix_cpu_freq_t system_freq { 0 };
    procmetrix_error_t status = procmetrix_cpu_freq_system(&system_freq);

    if (PROCMETRIX_ERROR_NONE != status)
        throw make_error(env, status);

    return serialize(env, system_freq);
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
