"""Process APIs tests."""

import os

import psutil
import pytest

import pyprocmetrix

from . import helpers


class TestOwnProcess:
    """Test reading data of the current process."""

    def test_get_own_pid(self):
        """Test getting my own PID."""
        own_pid = pyprocmetrix.get_pid()
        helpers.logger.info("own_pid=%u", own_pid)
        assert own_pid == os.getpid()

    def test_pid_exists_self(self):
        """Test my own pid exists."""
        assert pyprocmetrix.pid_exists(pyprocmetrix.get_pid())

    @pytest.mark.flaky(reruns=3)  # list can change between calls
    def test_parity_psutil_list_pids(self):
        """Test the list of running PIDs match psutil's."""
        pids_list = pyprocmetrix.list_pids()
        assert sorted(pids_list) == sorted(psutil.pids())

    def test_parity_psutil_get_proc_parent_pid(self):
        """Test getting the process' parent matches psutil's."""
        parent_pid = pyprocmetrix.get_proc_parent_pid(pyprocmetrix.get_pid())
        helpers.logger.info("parent_pid=%u", parent_pid)

        assert parent_pid == psutil.Process().ppid()

    def test_parity_psutil_get_proc_name(self):
        """Test the current process' name matches psutil's."""
        name = pyprocmetrix.get_proc_name(pyprocmetrix.get_pid())
        helpers.logger.info("name=%s", name)

        assert name == psutil.Process().name()

    def test_parity_psutil_get_proc_exe(self):
        """Test the current process' executable path matches psutil's."""
        exe = pyprocmetrix.get_proc_exe(pyprocmetrix.get_pid())
        helpers.logger.info("exe=%s", exe)

        assert exe == psutil.Process().exe()

    def test_parity_psutil_get_proc_cwd(self):
        """Test the current process' working directory matches psutil's."""
        cwd = pyprocmetrix.get_proc_cwd(pyprocmetrix.get_pid())
        helpers.logger.info("cwd=%s", cwd)

        assert cwd == psutil.Process().cwd()

    def test_parity_psutil_get_proc_cmdline(self):
        """Test the current process' command line args match psutil's."""
        cmdline = pyprocmetrix.get_proc_cmdline(pyprocmetrix.get_pid())
        for i in range(len(cmdline)):
            helpers.logger.info("argv[%i]=%s", i, cmdline[i])

        assert cmdline == psutil.Process().cmdline()

    def test_parity_psutil_get_proc_environ(self):
        """Test the current process' environment variables match psutil's."""
        environ = pyprocmetrix.get_proc_environ(pyprocmetrix.get_pid())
        assert environ == psutil.Process().environ()
