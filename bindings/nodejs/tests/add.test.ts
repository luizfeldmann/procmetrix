import { describe, expect, it } from "vitest";
import procmetrix from "@/index";

describe("add", () => {
  it("adds 2 + 2", () => {
    expect(procmetrix.add(2, 2)).toBe(4);
  });
});
