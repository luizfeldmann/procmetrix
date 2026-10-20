// Bindings
#include "error.h"
#include "init.h"
#include "reflection.h"

// Lib
#include <procmetrix/mem.h>

// Virtual Memory

template <>
struct reflection<procmetrix_virtual_memory_t>
{
    using dbl_field = member_field<procmetrix_virtual_memory_t, double>;
    using u64_field = member_field<procmetrix_virtual_memory_t, uint64_t>;

    template <typename visitor_type>
    static void accept(visitor_type&& visitor)
    {
        visitor(u64_field { "total", &procmetrix_virtual_memory_t::total });
        visitor(
            u64_field { "available", &procmetrix_virtual_memory_t::available });
        visitor(u64_field { "used", &procmetrix_virtual_memory_t::used });
        visitor(u64_field { "free", &procmetrix_virtual_memory_t::free });
        visitor(u64_field { "active", &procmetrix_virtual_memory_t::active });
        visitor(
            u64_field { "inactive", &procmetrix_virtual_memory_t::inactive });
        visitor(u64_field { "buffers", &procmetrix_virtual_memory_t::buffers });
        visitor(u64_field { "cached", &procmetrix_virtual_memory_t::cached });
        visitor(u64_field { "shared", &procmetrix_virtual_memory_t::shared });
        visitor(u64_field { "slab", &procmetrix_virtual_memory_t::slab });
        visitor(dbl_field { "ratio", &procmetrix_virtual_memory_t::ratio });
    }
};

static Napi::Object system_virtual_memory(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    procmetrix_virtual_memory_t virtual_memory { 0 };
    procmetrix_error_t status =
        procmetrix_system_virtual_memory(&virtual_memory);

    if (PROCMETRIX_ERROR_NONE != status)
        throw make_error(env, status);

    return serialize(env, virtual_memory);
}

// Swap Memory

template <>
struct reflection<procmetrix_swap_memory_t>
{
    using dbl_field = member_field<procmetrix_swap_memory_t, double>;
    using u64_field = member_field<procmetrix_swap_memory_t, uint64_t>;

    template <typename visitor_type>
    static void accept(visitor_type&& visitor)
    {
        visitor(u64_field { "total", &procmetrix_swap_memory_t::total });
        visitor(u64_field { "used", &procmetrix_swap_memory_t::used });
        visitor(u64_field { "free", &procmetrix_swap_memory_t::free });
        visitor(u64_field { "swap_in", &procmetrix_swap_memory_t::swap_in });
        visitor(u64_field { "swap_out", &procmetrix_swap_memory_t::swap_out });
        visitor(dbl_field { "ratio", &procmetrix_swap_memory_t::ratio });
    }
};

static Napi::Object system_swap_memory(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    procmetrix_swap_memory_t swap_memory { 0 };
    procmetrix_error_t status = procmetrix_system_swap_memory(&swap_memory);

    if (PROCMETRIX_ERROR_NONE != status)
        throw make_error(env, status);

    return serialize(env, swap_memory);
}

// Init

void init_mem(Napi::Env env, Napi::Object exports)
{
    exports.Set(
        "system_virtual_memory",
        Napi::Function::New(env, system_virtual_memory));

    exports.Set(
        "system_swap_memory", Napi::Function::New(env, system_swap_memory));
}
