"""Process APIs tests."""

import os
from typing import assert_type

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

        if helpers.IS_WINDOWS:
            # psutil converts all env vars names to upper-case in Windows
            # procmetrix' get_proc_environ keeps them as-is
            # in order to test the parity, this test emulates the upper-case 
            environ = {k.upper(): v for k, v in environ.items()}

        assert environ == psutil.Process().environ()

    def test_parity_psutil_get_proc_memory_info(self):
        """Test the current process' memory information matches psutil's."""
        memory_info = pyprocmetrix.get_proc_memory_info(pyprocmetrix.get_pid())
        expected = psutil.Process().memory_info()

        helpers.logger.info("rss=%s", helpers.format_bytes(memory_info.rss))
        helpers.logger.info("vms=%s", helpers.format_bytes(memory_info.vms))

        # Main metrics:
        rel = 0.01
        assert memory_info.rss == pytest.approx(expected.rss, rel)
        assert memory_info.vms == pytest.approx(expected.vms, rel)

        # Other metrics:
        if helpers.IS_LINUX:
            assert memory_info.shared == pytest.approx(expected.shared, rel)
            assert memory_info.lib == pytest.approx(expected.lib, rel)
            assert memory_info.dirty == pytest.approx(expected.dirty, rel)
        else:
            assert memory_info.shared == 0
            assert memory_info.lib == 0
            assert memory_info.dirty == 0

        if helpers.IS_LINUX or helpers.IS_BSD:
            assert memory_info.text == pytest.approx(expected.text, rel)
            assert memory_info.data == pytest.approx(expected.data, rel)
        else:
            assert memory_info.text == 0
            assert memory_info.data == 0

    def test_types_get_proc_memory_info(self):
        """Test the types returned by get_proc_memory_info."""
        memory_info = pyprocmetrix.get_proc_memory_info(pyprocmetrix.get_pid())

        assert isinstance(memory_info, pyprocmetrix.ProcessMemory)
        assert_type(memory_info, pyprocmetrix.ProcessMemory)

        assert isinstance(memory_info.rss, int)
        assert_type(memory_info.rss, int)

        assert isinstance(memory_info.vms, int)
        assert_type(memory_info.vms, int)

        assert isinstance(memory_info.shared, int)
        assert_type(memory_info.shared, int)

        assert isinstance(memory_info.text, int)
        assert_type(memory_info.text, int)

        assert isinstance(memory_info.lib, int)
        assert_type(memory_info.lib, int)

        assert isinstance(memory_info.data, int)
        assert_type(memory_info.data, int)

        assert isinstance(memory_info.dirty, int)
        assert_type(memory_info.dirty, int)

    def test_parity_psutil_get_proc_cpu_times(self):
        """Test the current process' CPU usage matches psutil's."""
        cpu_times = pyprocmetrix.get_proc_cpu_times(pyprocmetrix.get_pid())
        expected = psutil.Process().cpu_times()

        helpers.logger.info("user=%.3f", cpu_times.user)
        helpers.logger.info("system=%.3f", cpu_times.system)

        # Main metrics:
        rel = 0.01
        assert cpu_times.user == pytest.approx(expected.user, rel)
        assert cpu_times.system == pytest.approx(expected.system, rel)

        # Other metrix:
        if helpers.IS_LINUX or helpers.IS_BSD:
            assert cpu_times.children_user == pytest.approx(expected.children_user, rel)
            assert cpu_times.children_system == pytest.approx(expected.children_system, rel)
        else:
            assert cpu_times.children_user == 0
            assert cpu_times.children_system == 0

    def test_types_get_proc_cpu_times(self):
        """Test the types returned by get_proc_cpu_times."""
        cpu_times = pyprocmetrix.get_proc_cpu_times(pyprocmetrix.get_pid())

        assert isinstance(cpu_times, pyprocmetrix.ProcessCpuTimes)
        assert_type(cpu_times, pyprocmetrix.ProcessCpuTimes)

        assert isinstance(cpu_times.user, float)
        assert_type(cpu_times.user, float)

        assert isinstance(cpu_times.system, float)
        assert_type(cpu_times.system, float)

        assert isinstance(cpu_times.children_user, float)
        assert_type(cpu_times.children_user, float)

        assert isinstance(cpu_times.children_system, float)
        assert_type(cpu_times.children_system, float)
