# procmetrix

A lightweight C library for retrieving system metrics such as CPU utilization, frequency, and memory usage information.

[![.github/workflows/ci.yml](https://github.com/luizfeldmann/procmetrix/actions/workflows/ci.yml/badge.svg?branch=main&event=push)](https://github.com/luizfeldmann/procmetrix/actions/workflows/ci.yml)

---

# Features

- Logical and physical CPU counts
- CPU utilization percentages
- CPU frequency information
- Memory usage information
- Current process metrics
- Cross-platform
- No dependencies
- Written in C
- Bindings for Python available
- Bindings for NodeJS available

## Supported operating systems

- FreeBSD
- Linux
- Windows
- ...more soon.

# Requirements

- C99 or newer

- **Build requiremens**
  - Conan
  - CMake

- **Testing requirements**
  - GTest

# Building

Build the _C_ library using [conan](https://conan.io/):

```sh
# Build only the C library
conan build .
```

To build the language bindings as well:

```sh
# Build also the CPython extension
# (This only compiles the .so/.pyd, see below how to build a wheel)
conan build . --profile=default --profile=conan/profiles/python-bindings

# Build the NodeJS addon
# (This only compiles the .node addon, see below how to build an npm package)
conan build . --profile=default --profile=conan/profiles/nodejs-bindings
```

To build the _Python_ wheels:

```sh
pip install build
python -m build
```

To build the _npm_ package

```sh
# Build the native part of the bindings:
npm install
# Build the JavaScript part of the bindings:
npm run build
# Package to a .tgz
npm pack
```

# Testing

Run unit tests of the _C_ library with:

```sh
cd build/Release
ctest
```

Run tests of the _Python_ module with:

```sh
pip install ".[test]"
pytest
```

Run tests of the npm package (in development mode) with:

```sh
npm install
npm run conan:build
npm run test
```

# Documentation

The latest documentation is [published here](https://luizfeldmann.github.io/procmetrix/).

You can also build the docs yourself by running:

```sh
doxygen
```

The output files will be under `docs/html`.

# License

This project is licensed under the [MIT License](./LICENSE).
