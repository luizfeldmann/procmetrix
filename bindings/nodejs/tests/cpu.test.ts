import os from "node:os";
import { setTimeout } from "node:timers/promises";
import { beforeAll, describe, expect, it } from "vitest";
import si from "systeminformation";
import procmetrix from "@/index";
import { CpuTimes } from "@/procmetrix";
import { isWindows } from "./helpers";

describe("CPU counts", () => {
  let si_physical_cores: number;
  let si_logical_cores: number;

  beforeAll(async () => {
    const cpu = await si.cpu();
    si_physical_cores = cpu.physicalCores;
    si_logical_cores = cpu.cores;
  });

  it("physical count matches si", () => {
    const ncpu_physical = procmetrix.cpu_count_physical();
    expect(ncpu_physical).toBe(si_physical_cores);
  });

  it("logical count matches si", () => {
    const ncpu_logical = procmetrix.cpu_count_logical();
    expect(ncpu_logical).toBe(si_logical_cores);
  });
});

type OsCpuTimes = os.CpuInfo["times"];

describe("CPU times", () => {
  function check_times(
    cpu_times: CpuTimes,
    cpu_load: OsCpuTimes,
    mode: "total" | "percpu",
  ) {
    const numDigits = 1; // 1 decimal second tolerance
    expect(cpu_times.user).toBeCloseTo(cpu_load.user, numDigits);
    expect(cpu_times.system).toBeCloseTo(cpu_load.sys, numDigits);
    expect(cpu_times.idle).toBeCloseTo(cpu_load.idle, numDigits);
    expect(cpu_times.nice).toBeCloseTo(cpu_load.nice, numDigits);

    // On Windows, SI reports interrupt time as "irq"
    if (isWindows) {
      expect(cpu_times.irq).toBe(0);

      // Procmetrix only returns interrupt times on the per-CPU report
      if (mode == "percpu")
        expect(cpu_times.interrupt).toBeCloseTo(cpu_load.irq, numDigits);
      else expect(cpu_times.interrupt).toBe(0);
    } else {
      expect(cpu_times.irq).toBeCloseTo(cpu_load.irq, numDigits);
    }
  }

  function accumulate_cpu_times(times: OsCpuTimes[]) {
    return times.reduce<OsCpuTimes>(
      (acc, item) => {
        acc.user += item.user / 1000.0;
        acc.idle += item.idle / 1000.0;
        acc.sys += item.sys / 1000.0;
        acc.nice += item.nice / 1000.0;
        acc.irq += item.irq / 1000.0;
        return acc;
      },
      { user: 0, nice: 0, sys: 0, idle: 0, irq: 0 },
    );
  }

  it("matches total load", () => {
    // Read the test values from procmetrix and from node:os
    const cpu_times = procmetrix.cpu_times_total();

    // Accumulate totals from per-cpu array, convert from [ms] to [s]
    const cpu_load = accumulate_cpu_times(os.cpus().map((cpu) => cpu.times));

    check_times(cpu_times, cpu_load, "total");
  });

  it("matches per-cpu load", () => {
    // Read the test values from procmetrix and from node:os
    const cpu_times = procmetrix.cpu_times_per_cpu();

    // Convert from [ms] to [s]
    const cpu_load = os.cpus().map((cpu) => cpu.times);
    cpu_load.forEach((item) => {
      item.user /= 1000.0;
      item.idle /= 1000.0;
      item.sys /= 1000.0;
      item.nice /= 1000.0;
      item.irq /= 1000.0;
    });

    // Compare pairs per-CPU
    expect(cpu_times.length).toBe(cpu_load.length);
    for (let i = 0; i < cpu_times.length; ++i)
      check_times(cpu_times[i], cpu_load[i], "percpu");
  });

  it("matches utilization ratio", async () => {
    // First snapshot
    const before_cpu_times = procmetrix.cpu_times_total();
    const before_cpu_load = accumulate_cpu_times(
      os.cpus().map((cpu) => cpu.times),
    );

    // Introduce delay
    await setTimeout(1000);

    // Second snapshot
    const after_cpu_times = procmetrix.cpu_times_total();
    const after_cpu_load = accumulate_cpu_times(
      os.cpus().map((cpu) => cpu.times),
    );

    // Calculate utilization (procmetrix)
    const delta_cpu_times = procmetrix.cpu_times_delta(
      before_cpu_times,
      after_cpu_times,
    );

    const actual_utilization =
      procmetrix.cpu_utilization_ratio(delta_cpu_times);

    // Calculate utilization (node:os)
    const delta_cpu_load: OsCpuTimes = {
      user: after_cpu_load.user - before_cpu_load.user,
      idle: after_cpu_load.idle - before_cpu_load.idle,
      sys: after_cpu_load.sys - before_cpu_load.sys,
      nice: after_cpu_load.nice - before_cpu_load.nice,
      irq: after_cpu_load.irq - before_cpu_load.irq,
    };

    const expected_utilization =
      1.0 -
      delta_cpu_load.idle /
        (delta_cpu_load.idle +
          delta_cpu_load.user +
          delta_cpu_load.sys +
          delta_cpu_load.nice +
          delta_cpu_load.irq);

    // Compare
    expect(actual_utilization).toBeCloseTo(expected_utilization);
  });
});
