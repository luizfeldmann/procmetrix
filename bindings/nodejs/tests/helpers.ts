import { expect } from "vitest";

export const isWindows = process.platform === "win32";
export const isUnix = !isWindows;
export const isLinux = process.platform === "linux";
export const isMacOS = process.platform === "darwin";
export const isFreeBSD = process.platform === "freebsd";
export const isBSD = isFreeBSD;

//! Relative tolerance compare
export function approx(
  name: string,
  expected: number,
  actual: number,
  rel: number = 1e-2,
  abs: number = 1e-2,
) {
  const limit = Math.max(rel * Math.abs(expected), abs);
  const error = Math.abs(expected - actual);

  const message = `${name}: actual=${actual}, expected=${expected}, error=${error}, tolerance=${limit}`;
  expect(error, message).toBeLessThanOrEqual(limit);
}
