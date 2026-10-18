import os from "node:os";

// Vitest can only be imported as ESM
import { describe, expect, test } from "vitest";

// Try importing procmetrix as CJS
import { createRequire } from "node:module";
const require = createRequire(import.meta.url);
const procmetrix = require("procmetrix").default;

describe("CJS import", () => {
  test("CPUs count", () => {
    const ncpu = os.cpus().length;
    expect(procmetrix.cpu_count_logical()).toBe(ncpu);
  });
});
