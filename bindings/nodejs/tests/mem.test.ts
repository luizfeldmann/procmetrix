import { describe, it } from "vitest";
import si from "systeminformation";
import procmetrix from "@/index";
import { approx, isLinux, isWindows } from "./helpers";

describe("System Virtual Memory", () => {
  it("matches system information", async () => {
    const simem = await si.mem();
    const vmem = procmetrix.system_virtual_memory();

    if (isLinux) {
      approx("total", vmem.total, simem.total);
      approx("available", vmem.available, simem.available);
      approx("free", vmem.free, simem.free);
      approx("buffers", vmem.buffers, simem.buffers);
      approx("cached", vmem.cached, simem.cached + simem.reclaimable);
      approx("slab", vmem.slab, simem.slab);
    } else if (isWindows) {
      // TODO
    }
  });
});

describe("System Swap Memory", () => {
  it("matches system information", async () => {
    const simem = await si.mem();
    const swapmem = procmetrix.system_swap_memory();

    if (isLinux) {
      approx("total", swapmem.total, simem.swaptotal);
      approx("free", swapmem.free, simem.swapfree);
      approx("used", swapmem.used, simem.swapused);
    } else if (isWindows) {
      // TODO
    }
  });
});
