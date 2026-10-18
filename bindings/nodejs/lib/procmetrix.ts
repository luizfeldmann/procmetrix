/**
 * Every attribute represents the time in seconds,
 * that the CPU has spent in the given mode.
 */
export interface CpuTimes {
  user: number;
  system: number;
  idle: number;
  nice: number;
  iowait: number;
  irq: number;
  softirq: number;
  steal: number;
  guest: number;
  guest_nice: number;
  interrupt: number;
  dpc: number;
}

/**
 * Library for retrieving system and process metrics.
 */
export interface Procmetrix {
  /**
   * Gets the number of physical CPU cores.
   */
  cpu_count_physical(): number;

  /**
   * Gets the number of logical CPUs.
   */
  cpu_count_logical(): number;

  /**
   * Return system total CPU times.
   */
  cpu_times_total(): CpuTimes;

  /**
   * Returns the CPU times for each logical CPU in the system.
   */
  cpu_times_per_cpu(): CpuTimes[];

  /**
   * Calculates the delta time between two time counters snapshots.
   */
  cpu_times_delta(before: CpuTimes, after: CpuTimes): CpuTimes;

  /**
   * Calculates the sum of all CPU times.
   */
  cpu_times_sum(cpu_times: CpuTimes): number;

  /**
   * Calculates the utilization ratio of the CPU,
   * from the given time difference.
   */
  cpu_utilization_ratio(cpu_times: CpuTimes): number;
}
