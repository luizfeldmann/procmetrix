import os from "node:os";
import { describe, expect, test } from "vitest";
import procmetrix from "procmetrix";

describe("ESM import", () => {
  test("CPUs count", () => {
    const ncpu = os.cpus().length;
    expect(procmetrix.cpu_count_logical()).toBe(ncpu);
  });
});
