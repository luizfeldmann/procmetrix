"""General helpers for the tests."""

import os
import sys

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
