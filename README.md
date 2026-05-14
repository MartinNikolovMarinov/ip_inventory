# IP Inventory

[![Mac Build](https://github.com/MartinNikolovMarinov/ip_inventory/actions/workflows/mac_cmake.yml/badge.svg)](https://github.com/MartinNikolovMarinov/ip_inventory/actions/workflows/mac_cmake.yml)
[![Ubuntu Build](https://github.com/MartinNikolovMarinov/ip_inventory/actions/workflows/ubuntu_cmake.yml/badge.svg)](https://github.com/MartinNikolovMarinov/ip_inventory/actions/workflows/ubuntu_cmake.yml)
[![Windows Build](https://github.com/MartinNikolovMarinov/ip_inventory/actions/workflows/windows_cmake.yml/badge.svg)](https://github.com/MartinNikolovMarinov/ip_inventory/actions/workflows/windows_cmake.yml)
[![All Builds](https://github.com/MartinNikolovMarinov/ip_inventory/actions/workflows/build_cmake.yml/badge.svg)](https://github.com/MartinNikolovMarinov/ip_inventory/actions/workflows/build_cmake.yml)

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

# For Windows MSVC it's required to pass the config option:
cmake --build build --config Debug
cmake --build build --config Release
```

## Tests

```sh
ctest --test-dir build --output-on-failure

# For Windows MSVC it's required to pass the config option:
ctest --test-dir build -C Debug
ctest --test-dir build -C Release
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
