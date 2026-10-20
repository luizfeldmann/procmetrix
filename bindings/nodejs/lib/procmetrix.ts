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
 * The current frequency and scaling range for a CPU, in Mhz
 */
export interface CpuFreq {
  cur: number;
  min: number;
  max: number;
}

/**
 * Statistics about system memory usage.
 */
export interface SystemVirtualMemory {
  total: number;
  available: number;
  used: number;
  free: number;
  active: number;
  inactive: number;
  buffers: number;
  cached: number;
  shared: number;
  slab: number;
  ratio: number;
}

/**
 * System swap memory statistics.
 */
export interface SystemSwapMemory {
  total: number;
  used: number;
  free: number;
  swap_in: number;
  swap_out: number;
  ratio: number;
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

  /**
   * Reads the frequencies for each CPU.
   */
  cpu_freqs(): CpuFreq[];

  /**
   * Calculates the average frequency from the array of per-CPU frequencies.
   */
  cpu_freqs_average(cpu_freqs: CpuFreq[]): CpuFreq;

  /**
   * Estimates the system-wide CPU frequencies in MHz.
   */
  cpu_freq_system(): CpuFreq;

  /**
   * Return statistics about system memory usage in bytes.
   */
  system_virtual_memory(): SystemVirtualMemory;

  /**
   * Return statistics about system swap memory in bytes.
   */
  system_swap_memory(): SystemSwapMemory;
}
