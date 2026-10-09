"""procmetrix.

A cross-platform library for retrieving information on running processes and system utilization.
"""

# Re-export everything from the extension
from . import pyprocmetrix as _native
from .pyprocmetrix import *  # noqa: F403

# Version string comes from the version segments
__version__ = f"{_native.version_major}.{_native.version_minor}.{_native.version_patch}"
