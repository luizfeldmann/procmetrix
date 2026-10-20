/// <reference types="vitest/config" />
import { defineConfig } from "vite";
import dts from "vite-plugin-dts";
import { resolve } from "node:path";

export default defineConfig(({ command }) => ({
  plugins: [
    // Generate declarations (.d.ts)
    dts({
      tsconfigPath: "./tsconfig.build.json",
    }),
  ],
  // Build-time defines
  define: {
    // Selects between "npm run dev" / "npm run build"
    __PROCMETRIX_DEV__: command === "serve",
  },
  build: {
    minify: false,
    sourcemap: true,
    lib: {
      entry: "bindings/nodejs/lib/index.ts",
      // Generate both ESM and CJS builds
      formats: ["es", "cjs"],
      fileName: (format) => {
        return format === "es" ? "index.js" : "index.cjs";
      },
    },
    rollupOptions: {
      // Native addon (.node) is not bundled by vite
      external: [/node:/, /procmetrix\.node$/],
      output: {
        exports: "named",
      },
      platform: "node",
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
}));
