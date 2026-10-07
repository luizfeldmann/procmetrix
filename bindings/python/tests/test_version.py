import pyprocmetrix
from importlib.metadata import version

def test_version_matches_metadata():
    assert pyprocmetrix.__version__ == version("pyprocmetrix")
