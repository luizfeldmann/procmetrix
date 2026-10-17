//! Finds the addon in the native build tree
function findAddonInBuildTree() {
  // glob is a dev dependency
  // eslint-disable-next-line @typescript-eslint/no-require-imports
  const { globSync } = require("glob");

  // Find debug/release build
  const matches = globSync("build/**/procmetrix.node", {
    cwd: process.cwd(),
    absolute: true,
  });

  if (matches.length === 0)
    throw new Error("Could not locate build addon in build tree");
  else if (matches.length > 1)
    throw new Error("Multiple builds present in the build tree");

  // Single match
  return matches[0];
}

//! Indicates if this is running as npm run dev or after a build
declare const __PROCMETRIX_DEV__: boolean;

//! Finds and loads the addon (.node) file
function findAddon() {
  let addon_path;
  if (!__PROCMETRIX_DEV__) {
    // This is a built deployment, load the addon from this module
    addon_path = "./native/procmetrix.node";
  } else {
    // Editable mode, find the built file in the source tree
    addon_path = findAddonInBuildTree();
  }

  // eslint-disable-next-line @typescript-eslint/no-require-imports
  return require(addon_path);
}
export interface Procmetrix {
  add(a: number, b: number): number;
}

const procmetrix = findAddon() as Procmetrix;
export default procmetrix;
