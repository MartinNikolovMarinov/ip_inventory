# IP Inventory

[![CMake Build Matrix](https://github.com/MartinNikolovMarinov/ip_inventory/actions/workflows/build_cmake.yml/badge.svg)](https://github.com/MartinNikolovMarinov/ip_inventory/actions/workflows/build_cmake.yml)

## Build

List available presets:

```sh
cmake --list-presets
```

Configure with one preset:

```sh
cmake --preset gcc-debug
cmake --preset gcc-release
cmake --preset clang-debug
cmake --preset clang-release
cmake --preset msvc-debug
cmake --preset msvc-release
```

Build:

```sh
cmake --build build
```

## Tests

```sh
ctest --test-dir build --output-on-failure
```

## Run

```sh
./build/ip_inventory
```

The server listens on:

```text
http://localhost:8080
```

Swagger UI is available at:

```text
http://localhost:8080/docs/
```
