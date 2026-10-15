/// <reference types="vitest/config" />
import { defineConfig } from "vite";
import { resolve } from "node:path";

export default defineConfig({
  build: {
    lib: {
      entry: "bindings/nodejs/lib/index.ts",
      formats: ["es", "cjs"],
      fileName: (format) => {
        return format === "es" ? "index.js" : "index.cjs";
      },
    },
  },
  test: {
    environment: "node",
    include: ["bindings/nodejs/tests/**/*.test.ts"],
  },
  resolve: {
    alias: {
      "@": resolve(import.meta.dirname, "bindings/nodejs/lib"),
    },
  },
});
