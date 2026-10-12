"""General helpers for the tests."""

import logging
import os
import sys

# Utils
logger = logging.getLogger("tests")


def format_bytes(size: int) -> str:
    """Format a number of bytes."""
    units = ["B", "KB", "MB", "GB", "TB", "PB"]

    value = float(size)

    for unit in units:
        if value < 1024.0 or unit == units[-1]:
            return f"{value:.2f} {unit}"

        value /= 1024.0

    raise AssertionError("unreachable")


# Platform detection:

IS_WINDOWS = os.name == "nt"
"""Is this a Windows system"""

IS_UNIX = not IS_WINDOWS
"""Is this a UNIX system (i.e. not Windows)"""

IS_LINUX = sys.platform.startswith("linux")
"""Is this a Linux system"""

IS_MACOS = sys.platform.startswith("darwin")
"""Is this a MacOS system"""

IS_FREEBSD = sys.platform.startswith("freebsd")
"""Is this a FreeBSD system"""

IS_BSD = IS_FREEBSD
"""Is this a BSD system"""
