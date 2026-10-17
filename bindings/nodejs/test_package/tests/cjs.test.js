// Vitest can only be imported as ESM
import { describe, expect, test } from "vitest";

// Try importing procmetrix as CJS
import { createRequire } from "node:module";
const require = createRequire(import.meta.url);
const procmetrix = require("procmetrix").default;

describe("CJS import", () => {
  test("add", () => {
    expect(procmetrix.add(1, 2)).toBe(3);
  });
});
