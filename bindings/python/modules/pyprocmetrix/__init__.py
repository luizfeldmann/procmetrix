# Re-export everything from the extension
from .pyprocmetrix import *

# Version string comes from the version segments
__version__ = f"{version_major}.{version_minor}.{version_patch}"
