# IP Inventory

[![Mac Build](https://github.com/MartinNikolovMarinov/ip_inventory/actions/workflows/mac_cmake.yml/badge.svg)](https://github.com/MartinNikolovMarinov/ip_inventory/actions/workflows/mac_cmake.yml)
[![Ubuntu Build](https://github.com/MartinNikolovMarinov/ip_inventory/actions/workflows/ubuntu_cmake.yml/badge.svg)](https://github.com/MartinNikolovMarinov/ip_inventory/actions/workflows/ubuntu_cmake.yml)
[![Windows Build](https://github.com/MartinNikolovMarinov/ip_inventory/actions/workflows/windows_cmake.yml/badge.svg)](https://github.com/MartinNikolovMarinov/ip_inventory/actions/workflows/windows_cmake.yml)
[![All Builds](https://github.com/MartinNikolovMarinov/ip_inventory/actions/workflows/build_cmake.yml/badge.svg)](https://github.com/MartinNikolovMarinov/ip_inventory/actions/workflows/build_cmake.yml)

## Table of content

- [Overview](#overview)
- [Platforms](#platofrms)
- [Building and running the Project](#building-and-running-the-project)
    - [Build](#build)
    - [Tests](#tests)
    - [Run](#run)
- [Folder Structure](#folder-structure)
- [Architecture](#architecture)
    - [Design Diagram](#design-diagram)
    - [Database schema](#database-schema)
- [Continues Integration](#continues-integration)

## Overview

IP Inventory is a service for managing a shared pool of IPv4 and IPv6 addresses. It keeps track of which addresses are available, temporarily reserved, or permanently assigned to a service.

Clients use the API to add IP addresses to the pool, reserve addresses for a service, confirm a reservation by assigning the addresses, release assigned addresses, rename a service id, and query the current state of reservations or assignments. Reservations expire automatically if they are not assigned in time, so unused addresses return to the available pool.

The service exposes an HTTP/JSON API on `http://localhost:8080` by default. The current implementation stores data in SQLite, but persistence is behind a repository interface, so the same business logic can be used with another storage backend that implements the repository contract.

## Platforms

The project is designed for cross-platform and cross-architecture compatibility, supporting major desktop operating systems (Linux, macOS, Windows), compiler toolchains (GCC, Clang, MSVC), and CPU architectures (x86_64, ARM64).

Tested on:

1. Windows 11 x86_64
2. MacOS Tahoe (Version 26.5)
3. Ubuntu Linux x86_64

## Building and running the Project

The project does not have any runtime **dependencies**, but there are exactly 3 build **dependencies**:

* Git
* CMake - a version above or equal to `3.20.0` (released March 23, 2021)
* A Compiler capable of compiling C++ 20 code. MSVC, Clang and Gcc have been tested.

### Build

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
cmake --build build --parallel

# For Windows MSVC it's required to pass the config option:
cmake --build build --config Debug --parallel
cmake --build build --config Release --parallel
```

### Tests

To run the tests:
```sh
ctest --test-dir build --output-on-failure

# For Windows MSVC it's required to pass the config option:
ctest --test-dir build -C Debug
ctest --test-dir build -C Release
```

### Run

To start the server:
```sh
./build/ip_inventory

# For Windows MSVC the executables are likely in a subfolder:
./build/Debug/ip_inventory.exe
./build/Release/ip_inventory.exe
```

The server listens on:

```text
http://localhost:8080
```

Swagger UI is available at:

```text
http://localhost:8080/docs/
```

## Folder Structure

TODO: explain briefly the folder structure and the contents of the project

## Architecture

TODO: explain the basic classes and system components as well as the used dependencies (maybe that first).

### Design Diagram
![Design Diagram](./docs/Ip_invetory_design_diagram.png)

### Database schema

![Database Schema](./docs/db_schema.png)

The schema stores service ids separately from the IP pool and reservation rows, so pool entries can point either to an assigned service or to a temporary reservation. IP addresses use `(ip_type, ip_bytes)` as the primary key: the type distinguishes IPv4 from IPv6, and the binary bytes provide a canonical value independent of display formatting.

Indexes are created on `ip_pool.assigned_id`, `ip_pool.reserved_id`, and `reserved_ips.expiration_time`. They support the common assignment/reservation lookups and make reservation cleanup efficient when expired rows are removed.

## Continues Integration

The project uses GitHub Actions to run the same CMake configure, build, and test flow that is used locally. The workflows are triggered manually with `workflow_dispatch` so CI minutes are not spent on every commit while this is a personal project. For a production project, the same checks would normally run on pull requests, commits to `master`, and still remain available as manual runs.

There are separate workflows for Ubuntu, macOS, and Windows, plus a combined build matrix. Together they exercise the project across the main supported platform and compiler combinations: Linux with GCC and Clang, macOS with Clang, and Windows with MSVC. The workflows build both Debug and Release presets and run the test suite after each build.

This gives coverage across the main operating systems, compiler toolchains, and runner architectures used by the project, including x86 and ARM64 environments where they are available from GitHub-hosted runners.
