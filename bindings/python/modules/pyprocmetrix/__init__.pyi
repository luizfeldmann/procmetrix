from typing import Final

# Version:

version_major: Final[int]
"""Major version of the library."""

version_minor: Final[int]
"""Minor version of the library."""

version_patch: Final[int]
"""Patch version of the library."""

# Memory:

class SystemVirtualMemory:
    """Statistics about system memory usage."""

    ratio: float
    total: int
    available: int
    used: int
    free: int
    active: int
    inactive: int
    buffers: int
    cached: int
    shared: int
    slab: int

def virtual_memory() -> SystemVirtualMemory:
    """Read statistics about system memory usage in bytes."""

class SystemSwapMemory:
    """Statistics about system swap memory usage."""

    ratio: float
    total: int
    used: int
    free: int
    swap_in: int
    swap_out: int

def swap_memory() -> SystemSwapMemory:
    """Return system swap memory statistics in bytes."""

# CPU:

def cpu_count_logical() -> int:
    """Return the number of logical CPUs in the system."""

def cpu_count_physical() -> int:
    """Return the number of physical cores."""

class SystemCpuTimes:
    """Every attribute represents the seconds the CPU has spent in the given mode."""

    user: float
    system: float
    idle: float
    nice: float
    iowait: float
    irq: float
    softirq: float
    steal: float
    guest: float
    guest_nice: float
    interrupt: float
    dpc: float

def cpu_times_total() -> SystemCpuTimes:
    """Return system total CPU times."""

def cpu_times_per_cpu() -> list[SystemCpuTimes]:
    """Return the CPU times for each logical CPU in the system."""

def cpu_times_delta(before: SystemCpuTimes, after: SystemCpuTimes) -> SystemCpuTimes:
    """Return the time difference between two time snapshots."""

def cpu_times_sum(delta: SystemCpuTimes) -> float:
    """Return the total elapsed time from a delta."""

def cpu_utilization_ratio(delta: SystemCpuTimes) -> float:
    """Return the busy/usage ratio from a times delta."""

class SystemCpuFreq:
    """The current and range of CPU frequency [MHz]."""

    cur: float
    min: float
    max: float

def cpu_freqs() -> list[SystemCpuFreq]:
    """Read the frequencies for each CPU."""

def cpu_freqs_average(freqs: list[SystemCpuFreq]) -> SystemCpuFreq:
    """Calculate the average frequency from the array of per-CPU frequencies."""

def cpu_freq_system() -> SystemCpuFreq:
    """Estimates the system-wide CPU frequencies."""
