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
