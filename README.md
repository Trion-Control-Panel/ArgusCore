# ArgusCore

**ArgusCore** is a WoW Legion 7.3.5 server emulator — a fork of TrinityCore built and maintained as a modern, stable, maintainable framework.

---

## Build Status

| Platform | Status |
|----------|--------|
| GCC (Linux) | [![GCC Build](../../actions/workflows/gcc-build.yml/badge.svg)](../../actions/workflows/gcc-build.yml) |
| Windows x64 | [![Windows Build](../../actions/workflows/build-win-x64.yml/badge.svg)](../../actions/workflows/build-win-x64.yml) |
| macOS arm64 | [![macOS Build](../../actions/workflows/macos-arm-build.yml/badge.svg)](../../actions/workflows/macos-arm-build.yml) |

---

## About

ArgusCore is a C++ server emulator for World of Warcraft patch 7.3.5 (Legion).

It is derived from the [TrinityCore](https://github.com/TrinityCore/TrinityCore) project and the [Legion Preservation Project](https://github.com/The-Legion-Preservation-Project/TrinityCore), with a focus on:

- Long-term stability over fast iteration
- Incremental modernization of the codebase
- Maintainability and clean architecture
- Fully playable gameplay at every step

See [ROADMAP.md](ROADMAP.md) for the full modernization plan.

---

## Requirements

| Dependency | Version |
|------------|---------|
| CMake | 3.24+ |
| OpenSSL | 3.0+ |
| Boost | 1.74+ (Linux) / 1.78+ (Windows) |
| MySQL | 8.0.34+ |
| MariaDB | 10.6.3+ (alternative to MySQL) |

---

## Building

```bash
# Configure
cmake -S . -B build -DSCRIPTS=static -DTOOLS=ON

# Build
cmake --build build --config RelWithDebInfo
```

Full build guides for Windows, Linux, and macOS are in the project wiki (link TBD).

---

## Configuration

After building, copy the sample config files and edit them for your environment:

```
src/server/worldserver/worldserver.conf.dist  →  worldserver.conf
src/server/bnetserver/bnetserver.conf.dist    →  bnetserver.conf
```

Default database connection expects a MySQL/MariaDB user named `arguscore` with password `arguscore`.

---

## Reporting Issues

Open an issue via the [GitHub issue tracker](../../issues).

Before filing, check for existing issues to avoid duplicates. Include server logs, database version, and steps to reproduce.

---

## Contributing

Pull requests are welcome. Please keep changes small and focused — one fix or feature per PR. See [ROADMAP.md](ROADMAP.md) for the current development priorities.

---

## Copyright

License: GPL 2.0 — see [COPYING](COPYING).

ArgusCore is based on TrinityCore. Original TrinityCore copyright belongs to the TrinityCore developers (2008–present).
