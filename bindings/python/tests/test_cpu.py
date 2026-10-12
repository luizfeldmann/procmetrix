"""CPU metrics tests."""

import time

import psutil
import pytest
from typing_extensions import assert_type

import pyprocmetrix

from . import helpers


class TestNumCpus:
    """Tests numbers of cpus."""

    def test_parity_psutil_cpu_count_logical(self):
        """Check the reported number of logical CPUs matches psutil."""
        cpu_count_logical = pyprocmetrix.cpu_count_logical()
        helpers.logger.info("cpu_count_logical=%d", cpu_count_logical)

        assert cpu_count_logical == psutil.cpu_count(logical=True)

    def test_parity_psutil_cpu_count_physical(self):
        """Check the reported number of physical CPUs matches psutil."""
        cpu_count_physical = pyprocmetrix.cpu_count_physical()
        helpers.logger.info("cpu_count_physical=%d", cpu_count_physical)

        if helpers.IS_BSD:
            pytest.skip("psutil (7.2.2) does not implement an exact count for BSD")

        assert cpu_count_physical == psutil.cpu_count(logical=False)


class TestCpuTimes:
    """Tests the CPU time spent in various modes."""

    def _compare_cpu_times(self, per_cpu: bool, my_times: pyprocmetrix.SystemCpuTimes, their_times):
        rel = 0.01
        # Main metrics
        assert my_times.user == pytest.approx(their_times.user, rel)
        assert my_times.idle == pytest.approx(their_times.idle, rel)
        assert my_times.system == pytest.approx(their_times.system, rel)

        #  Other metrics
        if helpers.IS_UNIX:
            assert my_times.nice == pytest.approx(their_times.nice, rel)
        else:
            assert my_times.nice == 0

        if helpers.IS_LINUX or helpers.IS_BSD:
            assert my_times.irq == pytest.approx(their_times.irq, rel)
        else:
            assert my_times.irq == 0

        if helpers.IS_LINUX:
            assert my_times.iowait == pytest.approx(their_times.iowait, rel)
            assert my_times.softirq == pytest.approx(their_times.softirq, rel)
            assert my_times.steal == pytest.approx(their_times.steal, rel)
            assert my_times.guest == pytest.approx(their_times.guest, rel)
            assert my_times.guest_nice == pytest.approx(their_times.guest_nice, rel)
        else:
            assert my_times.iowait == 0
            assert my_times.softirq == 0
            assert my_times.steal == 0
            assert my_times.guest == 0
            assert my_times.guest_nice == 0

        if helpers.IS_WINDOWS and per_cpu:
            # when per_cpu=True:
            # psutil gets actual values for 'dpc' and 'interrupt' using 'NtQuerySystemInformation'
            # procmetrix does the same
            assert my_times.dpc == pytest.approx(their_times.dpc, rel)
            assert my_times.interrupt == pytest.approx(their_times.interrupt, rel)
        else:
            # when per_cpu=False:
            # psutil calculates 'dpc' and 'interrupt' by summing the values from the per-CPU API
            # procmetrix just sets them to zero
            assert my_times.dpc == 0
            assert my_times.interrupt == 0

    def test_parity_psutil_cpu_times_total(self):
        """Check the total CPU times match psutil's."""
        my_times = pyprocmetrix.cpu_times_total()
        their_times = psutil.cpu_times(percpu=False)
        self._compare_cpu_times(False, my_times, their_times)

    def test_types_cpu_times_total(self):
        """Check the types returned by cpu_times_total."""
        cpu_times = pyprocmetrix.cpu_times_total()
        assert isinstance(cpu_times, pyprocmetrix.SystemCpuTimes)
        assert_type(cpu_times, pyprocmetrix.SystemCpuTimes)

        assert isinstance(cpu_times.user, float)
        assert_type(cpu_times.user, float)

        assert isinstance(cpu_times.system, float)
        assert_type(cpu_times.system, float)

        assert isinstance(cpu_times.idle, float)
        assert_type(cpu_times.idle, float)

        assert isinstance(cpu_times.nice, float)
        assert_type(cpu_times.nice, float)

        assert isinstance(cpu_times.iowait, float)
        assert_type(cpu_times.iowait, float)

        assert isinstance(cpu_times.irq, float)
        assert_type(cpu_times.irq, float)

        assert isinstance(cpu_times.softirq, float)
        assert_type(cpu_times.softirq, float)

        assert isinstance(cpu_times.steal, float)
        assert_type(cpu_times.steal, float)

        assert isinstance(cpu_times.guest, float)
        assert_type(cpu_times.guest, float)

        assert isinstance(cpu_times.guest_nice, float)
        assert_type(cpu_times.guest_nice, float)

        assert isinstance(cpu_times.interrupt, float)
        assert_type(cpu_times.interrupt, float)

        assert isinstance(cpu_times.dpc, float)
        assert_type(cpu_times.dpc, float)

    def test_parity_psutil_cpu_times_per_cpu(self):
        """Check the per-CPU times match psutil's."""
        my_times = pyprocmetrix.cpu_times_per_cpu()
        their_times = psutil.cpu_times(percpu=True)

        assert len(my_times) == len(their_times)

        for i in range(0, len(my_times)):
            self._compare_cpu_times(True, my_times[i], their_times[i])

    def test_types_cpu_times_per_cpu(self):
        """Check the types returned by cpu_times_per_cpu."""
        cpu_times = pyprocmetrix.cpu_times_per_cpu()

        assert isinstance(cpu_times, list)
        assert_type(cpu_times, list[pyprocmetrix.SystemCpuTimes])

        assert all(isinstance(item, pyprocmetrix.SystemCpuTimes) for item in cpu_times)

    def _burn_cpu(self, seconds: float) -> None:
        end = time.perf_counter() + seconds
        x = 0
        while time.perf_counter() < end:
            x += 1

    @pytest.mark.flaky(reruns=5)
    def test_parity_psutil_cpu_utilization_ratio(self):
        """Checks the total CPU utilization matches psutil."""
        # First sample
        psutil.cpu_percent(interval=None, percpu=False)
        before = pyprocmetrix.cpu_times_total()

        # Delay
        self._burn_cpu(1.0)

        # Second sample
        their_usage = psutil.cpu_percent(interval=None, percpu=False)
        after = pyprocmetrix.cpu_times_total()

        # Calc usage
        delta = pyprocmetrix.cpu_times_delta(before, after)
        my_usage = pyprocmetrix.cpu_utilization_ratio(delta)
        helpers.logger.info("cpu_utilization_ratio=%.2f %%", my_usage * 100.0)

        assert my_usage == pytest.approx(their_usage / 100.0, rel=0.02)


class TestCpuFreqs:
    """Tests the CPU frequency metrics."""

    def _compare_cpu_freqs(self, my_freq: pyprocmetrix.SystemCpuFreq, their_freq):
        rel = 0.01
        assert my_freq.cur == pytest.approx(their_freq.current, rel)
        assert my_freq.min == pytest.approx(their_freq.min, rel)
        assert my_freq.max == pytest.approx(their_freq.max, rel)

    def test_parity_psutil_cpu_freqs_per_cpu(self):
        """Compare per-CPU frequency with psutil."""
        my_freqs = pyprocmetrix.cpu_freqs()
        their_freqs = psutil.cpu_freq(percpu=True)

        if helpers.IS_FREEBSD and len(their_freqs) == 0:
            # psutil (7.2.2) tries to sysctl dev.cpu.X.freq, which doesnt exist in VMs
            # procmetrix can fallback to sysctl hw.clockrate
            pytest.skip("psutil couldn't read CPU freq")

        if helpers.IS_WINDOWS:
            # On Windows, psutil returns only CPU 0
            my_freqs = [my_freqs[0]]

        assert len(my_freqs) == len(their_freqs)

        for i in range(0, len(my_freqs)):
            self._compare_cpu_freqs(my_freqs[i], their_freqs[i])

    def test_parity_psutil_cpu_freqs_system(self):
        """Compare system-wide CPU frequency with psutil."""
        their_freq = psutil.cpu_freq(percpu=False)

        if helpers.IS_FREEBSD and their_freq is None:
            # psutil has no fallback for virtualized systems not exposing sysctl dev.cpu.X.freq
            pytest.skip("psutil couldn't read CPU freq")

        if helpers.IS_WINDOWS:
            # in Windows psutil returns the freqs of CPU 0
            my_freq = pyprocmetrix.cpu_freqs()[0]
        else:
            # cpu_freq_system gives the average of all cores
            my_freq = pyprocmetrix.cpu_freq_system()

        self._compare_cpu_freqs(my_freq, their_freq)

    def test_types_cpu_freqs(self):
        """Check the types of the CPU frequency object."""
        cpu_freqs = pyprocmetrix.cpu_freqs()

        # The method returns a list
        assert isinstance(cpu_freqs, list)
        assert_type(cpu_freqs, list[pyprocmetrix.SystemCpuFreq])

        # Every element in the list is a CPU frequency
        assert all(isinstance(freq, pyprocmetrix.SystemCpuFreq) for freq in cpu_freqs)

        # Get the average of the list
        average_freq = pyprocmetrix.cpu_freqs_average(cpu_freqs)
        assert isinstance(average_freq, pyprocmetrix.SystemCpuFreq)
        assert_type(average_freq, pyprocmetrix.SystemCpuFreq)

        # The types of the members of the frequency struct
        assert isinstance(average_freq.cur, float)
        assert_type(average_freq.cur, float)

        assert isinstance(average_freq.min, float)
        assert_type(average_freq.min, float)

        assert isinstance(average_freq.max, float)
        assert_type(average_freq.max, float)
