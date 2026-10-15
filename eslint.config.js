import { defineConfig } from "eslint/config";
import tseslint from "typescript-eslint";
import js from "@eslint/js";

export default defineConfig([
  {
    files: ["bindings/nodejs/**/*.ts"],
    ignores: ["dist/**"],
    extends: [js.configs.recommended, tseslint.configs.recommended],
    rules: {
      "@typescript-eslint/no-unused-vars": [
        "error",
        {
          argsIgnorePattern: "^_",
        },
      ],
    },
  },
]);
