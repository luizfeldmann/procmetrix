"""Version-related tests."""

from importlib.metadata import version
from typing import assert_type

import pyprocmetrix


def test_version_matches_metadata():
    """Checks the imported version matches what's returned by the built library."""
    assert pyprocmetrix.__version__ == version("pyprocmetrix")


def test_version_types():
    """Checks the correct types for the version constants."""
    assert isinstance(pyprocmetrix.version_major, int)
    assert_type(pyprocmetrix.version_major, int)

    assert isinstance(pyprocmetrix.version_minor, int)
    assert type(pyprocmetrix.version_minor) is int

    assert isinstance(pyprocmetrix.version_patch, int)
    assert type(pyprocmetrix.version_patch) is int
