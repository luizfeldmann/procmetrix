"""Memory metrics tests."""

from typing import assert_type

import psutil
import pytest

import pyprocmetrix

from . import helpers


class TestVirtualMemory:
    """Tests virtual memory metrics."""

    def test_virtual_memory_parity_psutil(self):
        """Compares the virtual memory to the results of psutil."""
        my_vmem = pyprocmetrix.virtual_memory()
        their_vmem = psutil.virtual_memory()

        rel = 0.01
        # Main metrics
        assert my_vmem.total == pytest.approx(their_vmem.total, rel)
        assert my_vmem.available == pytest.approx(their_vmem.available, rel)
        assert my_vmem.used == pytest.approx(their_vmem.used, rel)
        assert my_vmem.free == pytest.approx(their_vmem.free, rel)
        assert my_vmem.ratio == pytest.approx(their_vmem.percent / 100.0, rel)

        # Other metrics
        if helpers.IS_UNIX:
            assert my_vmem.active == pytest.approx(their_vmem.active, rel)
            assert my_vmem.inactive == pytest.approx(their_vmem.inactive, rel)
        if helpers.IS_LINUX:
            assert my_vmem.slab == pytest.approx(their_vmem.slab, rel)
        if helpers.IS_LINUX or helpers.IS_BSD:
            assert my_vmem.buffers == pytest.approx(their_vmem.buffers, rel)
            assert my_vmem.cached == pytest.approx(their_vmem.cached, rel)
            assert my_vmem.shared == pytest.approx(their_vmem.shared, rel)

    def test_virtual_memory_types(self):
        """Check the stub and runtime types for virtual memory."""
        vmem = pyprocmetrix.virtual_memory()
        assert isinstance(vmem, pyprocmetrix.SystemVirtualMemory)
        assert_type(vmem, pyprocmetrix.SystemVirtualMemory)

        assert isinstance(vmem.ratio, float)
        assert_type(vmem.ratio, float)

        assert isinstance(vmem.total, int)
        assert_type(vmem.total, int)

        assert isinstance(vmem.available, int)
        assert_type(vmem.available, int)

        assert isinstance(vmem.used, int)
        assert_type(vmem.used, int)

        assert isinstance(vmem.free, int)
        assert_type(vmem.free, int)

        assert isinstance(vmem.active, int)
        assert_type(vmem.active, int)

        assert isinstance(vmem.inactive, int)
        assert_type(vmem.inactive, int)

        assert isinstance(vmem.buffers, int)
        assert_type(vmem.buffers, int)

        assert isinstance(vmem.cached, int)
        assert_type(vmem.cached, int)

        assert isinstance(vmem.shared, int)
        assert_type(vmem.shared, int)

        assert isinstance(vmem.slab, int)
        assert_type(vmem.slab, int)


class TestSwapMemory:
    """Test swap memory metrics."""

    def test_swap_memory_parity_psutil(self):
        """Compares the swap memory to the results of psutil."""
        my_smem = pyprocmetrix.swap_memory()
        their_smem = psutil.swap_memory()

        rel = 0.01

        assert my_smem.total == pytest.approx(their_smem.total, rel)
        assert my_smem.used == pytest.approx(their_smem.used, rel)
        assert my_smem.free == pytest.approx(their_smem.free, rel)
        assert my_smem.swap_in == pytest.approx(their_smem.sin, rel)
        assert my_smem.swap_out == pytest.approx(their_smem.sout, rel)
        assert my_smem.ratio == pytest.approx(their_smem.percent / 100.0, rel)

    def test_swap_memory_types(self):
        """Check the stub and runtime types for swap memory."""
        smem = pyprocmetrix.swap_memory()
        assert isinstance(smem, pyprocmetrix.SystemSwapMemory)
        assert_type(smem, pyprocmetrix.SystemSwapMemory)

        assert isinstance(smem.ratio, float)
        assert_type(smem.ratio, float)

        assert isinstance(smem.total, int)
        assert_type(smem.total, int)

        assert isinstance(smem.used, int)
        assert_type(smem.used, int)

        assert isinstance(smem.free, int)
        assert_type(smem.free, int)

        assert isinstance(smem.swap_in, int)
        assert_type(smem.swap_in, int)

        assert isinstance(smem.swap_out, int)
        assert_type(smem.swap_out, int)
