// Lib
#include "procmetrix/error.h"
#include <procmetrix/mem.h>

// Internal
#include <internal/freebsd/common_freebsd_internal.h>

// STD
#include <fcntl.h>
#include <paths.h>
#include <string.h>
#include <unistd.h>

// System
#include <kvm.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/vmmeter.h>
#include <vm/vm.h>
#include <vm/vm_param.h>

// Impl

procmetrix_error_t
procmetrix_system_virtual_memory(procmetrix_virtual_memory_t* virtual_memory)
{
    // Sanity
    if (NULL == virtual_memory)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    memset(virtual_memory, 0, sizeof(*virtual_memory));

    // Read page size
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0)
        return PROCMETRIX_ERROR_UNKNOWN;

    // Total
    unsigned long total = 0;
    procmetrix_error_t status =
        procmetrix_impl_bsd_sysctlbyname("hw.physmem", &total, sizeof(total));
    if (PROCMETRIX_ERROR_NONE != status)
        return status;

    virtual_memory->total = (uint64_t)total;

    // Buffers
    long buffers = 0;
    status = procmetrix_impl_bsd_sysctlbyname(
        "vfs.bufspace", &buffers, sizeof(buffers));
    if (PROCMETRIX_ERROR_NONE != status)
        return status;

    virtual_memory->buffers = (uint64_t)buffers;

    // Free
    unsigned int free = 0;
    status = procmetrix_impl_bsd_sysctlbyname(
        "vm.stats.vm.v_free_count", &free, sizeof(free));
    if (PROCMETRIX_ERROR_NONE != status)
        return status;

    virtual_memory->free = (uint64_t)page_size * (uint64_t)free;

    // Active
    unsigned int active = 0;
    status = procmetrix_impl_bsd_sysctlbyname(
        "vm.stats.vm.v_active_count", &active, sizeof(active));
    if (PROCMETRIX_ERROR_NONE != status)
        return status;

    virtual_memory->active = (uint64_t)page_size * (uint64_t)active;

    // Inactive
    unsigned int inactive = 0;
    status = procmetrix_impl_bsd_sysctlbyname(
        "vm.stats.vm.v_inactive_count", &inactive, sizeof(inactive));
    if (PROCMETRIX_ERROR_NONE != status)
        return status;

    virtual_memory->inactive = (uint64_t)page_size * (uint64_t)inactive;

    // Wired
    unsigned int pages_wired = 0;
    status = procmetrix_impl_bsd_sysctlbyname(
        "vm.stats.vm.v_wire_count", &pages_wired, sizeof(pages_wired));
    if (PROCMETRIX_ERROR_NONE != status)
        return status;

    uint64_t wired = (uint64_t)page_size * (uint64_t)pages_wired;

    // Optional; ignore error if not avail
    unsigned int cached = 0;
    if (PROCMETRIX_ERROR_NONE ==
        procmetrix_impl_bsd_sysctlbyname(
            "vm.stats.vm.v_cache_count", &cached, sizeof(cached)))
        virtual_memory->cached = (uint64_t)page_size * (uint64_t)cached;

    // Dependant variables
    virtual_memory->available = virtual_memory->inactive +
                                virtual_memory->cached + virtual_memory->free;

    virtual_memory->used =
        virtual_memory->active + wired + virtual_memory->cached;

    virtual_memory->ratio =
        (double)(virtual_memory->total - virtual_memory->available) /
        (double)(virtual_memory->total);

    // Shared memory
    struct vmtotal virtmem;
    int mib[] = { CTL_VM, VM_METER };
    status = procmetrix_impl_bsd_sysctl(mib, 2, &virtmem, sizeof(virtmem));
    if (PROCMETRIX_ERROR_NONE != status)
        return status;

    virtual_memory->shared =
        (uint64_t)page_size * (virtmem.t_vmshr + virtmem.t_rmshr);

    return status;
}

procmetrix_error_t
procmetrix_system_swap_memory(procmetrix_swap_memory_t* swap_memory)
{
    // Sanity
    if (NULL == swap_memory)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    memset(swap_memory, 0, sizeof(*swap_memory));

    // Read page size
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0)
        return PROCMETRIX_ERROR_UNKNOWN;

    // Read KVM
    kvm_t* fd;
    struct kvm_swap kvmsw[1];

    fd = kvm_open(NULL, _PATH_DEVNULL, NULL, O_RDONLY, "kvm_open failed");
    if (NULL == fd)
        return PROCMETRIX_ERROR_FILE_READ;

    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;
    if (kvm_getswapinfo(fd, kvmsw, 1, 0) < 0)
        status = PROCMETRIX_ERROR_UNKNOWN;

    // Cleanup KVM file open
    kvm_close(fd);

    if (PROCMETRIX_ERROR_NONE != status)
        return status;

    // Basic swap memory
    swap_memory->total = (uint64_t)kvmsw[0].ksw_total * (uint64_t)page_size;
    swap_memory->used = (uint64_t)kvmsw[0].ksw_used * (uint64_t)page_size;
    swap_memory->free = swap_memory->total - swap_memory->used;
    swap_memory->ratio = (double)swap_memory->used / (double)swap_memory->total;

    // Swap-in & Swap-out
    unsigned int swapin = 0;
    unsigned int swapout = 0;
    unsigned int nodein = 0;
    unsigned int nodeout = 0;

    status = procmetrix_impl_bsd_sysctlbyname(
        "vm.stats.vm.v_swapin", &swapin, sizeof(swapin));
    if (PROCMETRIX_ERROR_NONE != status)
        return status;

    status = procmetrix_impl_bsd_sysctlbyname(
        "vm.stats.vm.v_swapout", &swapout, sizeof(swapout));
    if (PROCMETRIX_ERROR_NONE != status)
        return status;

    // Node-in & Node-out
    status = procmetrix_impl_bsd_sysctlbyname(
        "vm.stats.vm.v_vnodein", &nodein, sizeof(nodein));
    if (PROCMETRIX_ERROR_NONE != status)
        return status;

    status = procmetrix_impl_bsd_sysctlbyname(
        "vm.stats.vm.v_vnodeout", &nodeout, sizeof(nodeout));
    if (PROCMETRIX_ERROR_NONE != status)
        return status;

    swap_memory->swap_in = swapin + swapout;
    swap_memory->swap_out = nodein + nodeout;

    return status;
}
