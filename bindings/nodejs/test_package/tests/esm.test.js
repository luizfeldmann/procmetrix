import { describe, expect, test } from "vitest";
import procmetrix from "procmetrix";

describe("ESM import", () => {
  test("add", () => {
    expect(procmetrix.add(1, 2)).toBe(3);
  });
});
