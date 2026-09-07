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
- Written in C
- Cross-platform
- No dependencies

## Supported operating systems

- Linux
- ...more soon.

# Requirements

- C99 or newer

- **Build requiremens**
    - Conan
    - CMake

- **Testing requirements**
    - GTest

# Building

Build using [conan](https://conan.io/):

```sh
conan build .
```

# Testing

Run unit tests with:

```sh
cd build/Release
ctest
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
