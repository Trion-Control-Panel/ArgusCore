# ArgusCore Modernization Roadmap

## Project

Modernize an old Legion-based TrinityCore source into a modern, stable, maintainable emulator framework called ArgusCore.

### Main Goals

- Rebrand to ArgusCore
- Modernize the codebase slowly
- Upgrade dependencies safely
- Improve maintainability
- Add modular architecture similar to AzerothCore
- Improve threading and map handling carefully
- Keep gameplay behavior identical
- **NEVER** break core mechanics, gameplay logic, combat, movement, networking, or scripting behavior

### Critical Rules

- Every change must be **SMALL** and **ISOLATED**
- Every change must **COMPILE** before moving on
- Every change must **BOOT** before moving on
- Every change must be **TESTED IN-GAME** before continuing
- NEVER combine gameplay refactors with threading changes
- NEVER rewrite large systems at once
- NEVER optimize before profiling
- NEVER change behavior unless explicitly intended
- Stability is more important than modernization speed
- Maintain backward compatibility whenever possible

### Development Strategy

- Make tiny incremental commits
- One subsystem at a time
- One dependency at a time
- One architectural improvement at a time
- Keep the game fully playable after every phase

---

## Phase Overview

| # | Phase | Status |
|---|-------|--------|
| 1 | [Safe Rebranding](#phase-1--safe-rebranding) | **Validated** |
| 2 | [Custom Script Integration](#phase-2--custom-script-integration) | **Validated** |
| 3 | [Build System Cleanup](#phase-3--build-system-cleanup) | **Validated** |
| 4 | [Dependency Updates](#phase-4--dependency-updates) | **Validated** |
| 5 | [Code Modernization](#phase-5--code-modernization) | Not started |
| 6 | [CI, Testing, and Profiling](#phase-6--ci-testing-and-profiling) | **In Progress** |
| 7 | [Modular System](#phase-7--modular-system) | **Complete** |
| 8 | [Safe Async Systems](#phase-8--safe-async-systems) | Not started |
| 9 | [Map Threading Research](#phase-9--map-threading-research) | Not started |

---

## Phase 1 — Safe Rebranding

**Goals:** Rename project branding safely. No gameplay or engine logic changes.

### Tasks

- [x] Rename CMake project name (`TrinityCore` → `ArgusCore`)
- [x] Rename config file names and content (`worldserver.conf.dist`, `bnetserver.conf.dist`)
- [x] Rename console output text (`showoptions.cmake`, `Banner.cpp` startup art)
- [x] Add ArgusCore branding assets (`README.md`, `CONTRIBUTING.md`, `ROADMAP.md`)
- [x] Update CI artifact names (`appveyor.yml`, `build-win-x64.yml`)
- [x] Update all CMake/cmake file headers (51 files batch-replaced)
- [x] Update `revision_data.h.in.cmake` header guard and copyright strings
- [ ] Docker/container names — `trinitycore/circle-ci` is an upstream Docker Hub image; needs own CI image build (deferred)

### Do NOT

- Rename namespaces yet
- Rename internal engine classes yet
- Change packet structures
- Change scripting interfaces

### Validation

- [x] Server compiles
- [x] Server boots
- [x] Client connects
- [x] Characters load correctly

---

## Phase 2 — Custom Script Integration

**Goals:** Absorb all `TheLegionPreservationProject` scripts into the proper core script
files and delete the temporary folder. Each script gets moved one at a time,
verified in-game, then committed. The folder is only deleted after every script
has been confirmed working in its new home.

### Why a Dedicated Phase

The TLPP folder was a staging area — scripts dumped in one place so they could
be carried over from the old private server. They do not belong there long-term.
Moving them into the standard script directories makes them visible to anyone
reading the code and removes the implicit "these are second-class citizens" signal.

### Scripts to Integrate

| Script | Current location | Target location | What it does |
|---|---|---|---|
| `custom_lfg.cpp` | `TheLegionPreservationProject/Custom/` | `scripts/World/world_custom_lfg.cpp` | Solo dungeon finder — lets one player queue alone |
| `tlpp_spell_mage.cpp` | `TheLegionPreservationProject/Spells/` | `scripts/Spells/spell_mage.cpp` (merge) | Ice Lance + Thermal Void + Fingers of Frost fixes |
| `tlpp_spell_shaman.cpp` | `TheLegionPreservationProject/Spells/` | `scripts/Spells/spell_shaman.cpp` (merge) | Shaman spell fixes |
| `tlpp_spell_warrior.cpp` | `TheLegionPreservationProject/Spells/` | `scripts/Spells/spell_warrior.cpp` (merge) | Warrior spell fixes |

### Tasks

- [x] Move `tlpp_spell_mage.cpp` content into `scripts/Spells/spell_mage.cpp` — Ice Lance and Thermal Void dropped (core implementation is more complete); Chain Reaction was missing from TLPP
- [x] Move `tlpp_spell_shaman.cpp` content into `scripts/Spells/spell_shaman.cpp` — Maelstrom Weapon dropped (core more complete); Feral Spirit + Healing Surge merged; Stormbringer skipped (spell ID 201845 conflicts with existing `spell_sha_stormsurge` — defer to Phase 5)
- [x] Move `tlpp_spell_warrior.cpp` content into `scripts/Spells/spell_warrior.cpp` — Heroic Leap dropped (TLPP used hack spell ID 94954, core uses correct 178368); Ignore Pain merged
- [x] Move `custom_lfg.cpp` into `scripts/World/world_solo_lfg.cpp`, registered in `world_script_loader.cpp`
- [x] `AddTheLegionPreservationProjectScripts()` — only defined/called within the TLPP folder itself; not wired from any external loader; no edit needed
- [x] Delete `src/server/scripts/TheLegionPreservationProject/` folder
- [ ] Confirm the script tree in CMake output no longer shows the TLPP entry
- [ ] Update DB: `creature_template.ScriptName` for entry 29264 → `npc_sha_feral_spirit`

### Rules

- Move one script at a time — compile and test before touching the next
- Do not change any spell logic while moving — copy behavior exactly as-is
- If a spell fix already exists upstream in the target file, compare carefully before merging to avoid duplicates

### Do NOT

- Fix or improve the spell logic during this phase — that is Phase 5 work
- Move scripts that are not in TLPP
- Touch gameplay systems outside of these four scripts

### Validation

- [ ] Server compiles after each script move
- [ ] Mage Ice Lance / Thermal Void / Fingers of Frost behave correctly (dropped — no change needed)
- [ ] Shaman Feral Spirit / Healing Surge behave correctly
- [ ] Warrior Ignore Pain behaves correctly
- [ ] Solo LFG queues work correctly
- [x] `TheLegionPreservationProject/` folder is gone
- [ ] No regressions in unrelated specs

---

## Phase 3 — Build System Cleanup

**Goals:** Modernize build infrastructure only. No gameplay changes.

### Tasks

- [x] Upgrade minimum CMake version requirement — 3.24 → 3.28
- [x] Remove `if(POLICY ...)` guards — policies CMP0144 and CMP0153 are unconditional now (require 3.28)
- [x] Replace `CMAKE_EXE/SHARED_LINKER_FLAGS` string appending with `add_link_options()` — msvc, unix, clang settings
- [x] Replace `DisableIncrementalLinking` macro with a `foreach()` loop — macros leak caller scope
- [x] Remove duplicate `include(CheckCXXSourceCompiles)` in clang/settings.cmake
- [x] Remove dead commented-out old-style flag code in gcc/settings.cmake
- [x] `target_include_directories` — already correct (no global `include_directories` calls found)
- [x] `target_link_libraries` — already correct (no global `link_libraries` calls found)
- [x] Compiler warning handling — already using interface targets correctly

### Do NOT

- Refactor gameplay code
- Modify threading
- Change map systems

### Validation

- [ ] MSVC build passes (Windows)
- [ ] GCC build passes (Linux)
- [ ] Debug build passes
- [ ] Release build passes

---

## Phase 4 — Dependency Updates

**Goals:** Upgrade dependencies one at a time safely.

### Update Order

1. [x] OpenSSL — installed 4.0.0 (released Apr 2026); floor kept at `3` so Linux builds (still on 3.x) are not broken; 4.0.0 confirmed working on Windows
2. [x] Boost — installed 1.91.0; Windows floor updated 1.83 → 1.91; Linux floor stays 1.74 (Ubuntu 22.04 apt)
3. [x] MySQL — installed 8.4.9 (current LTS); floor updated 8.0.34 → 8.4.0
4. [ ] MariaDB LTS — not installed; floor stays 10.6.3 until tested

### Rules

- Only update **ONE** dependency at a time
- Fix compatibility issues immediately
- Add compatibility wrappers if needed
- Avoid rewriting systems

### Do NOT

- Mix dependency updates together
- Rewrite crypto systems
- Rewrite DB systems yet

### Validation

- [ ] Authentication works
- [ ] Database queries work
- [ ] Characters save/load correctly
- [ ] Worldserver stability confirmed

---

## Phase 5 — Code Modernization

**Goals:** Introduce modern C++ slowly. Keep identical behavior.

### Tasks

- [ ] Replace `NULL` with `nullptr` (2,942 occurrences across src/)
- [ ] Replace `typedef` with `using`
- [ ] Introduce `constexpr` safely
- [ ] Introduce `enum class` safely
- [ ] Replace obvious ownership raw pointers with `unique_ptr`
- [ ] Remove dead code carefully

### Rules

- Refactor only small files at a time
- Keep commits tiny
- Test gameplay after every refactor

### Do NOT

- Rewrite inheritance trees
- Rewrite managers
- Rewrite packet systems
- Rewrite object systems

### Validation

- [ ] No gameplay regressions
- [ ] No combat issues
- [ ] No movement issues
- [ ] No crashes during gameplay

---

## Phase 6 — CI, Testing, and Profiling

**Goals:** Add safety systems before any threading work begins.

### Tasks

- [x] Create `.github/workflows/build-linux.yml` — Ubuntu 24.04, GCC 13, Debug, unit tests, smoke test
- [x] Create `.github/workflows/build-windows.yml` — Windows 2022, MSVC, RelWithDebInfo, artifact upload
- [x] Replace `appveyor.yml` and `.circleci/config.yml` (obsolete, TrinityCore-branded)
- [ ] Add ASAN build job to linux workflow
- [ ] Add UBSAN build job to linux workflow
- [ ] Add profiling tools
- [ ] Add basic performance metrics collection

### Metrics to Track

- Map update times
- Player update times
- Packet queue sizes
- DB latency

### Do NOT

- Start multithreading yet
- Optimize blindly

### Validation

- [ ] Sanitizers clean
- [ ] Stable long runtime testing (24h+)
- [ ] No memory corruption

---

## Phase 7 — Modular System

**Goals:** Introduce modular architecture slowly.

### Tasks

- [x] Create `modules/` directory structure
- [x] Add `modules/cmake/AddModule.cmake` — `add_argus_module()` helper function
- [x] Auto-discover `mod-*` subdirectories via `file(GLOB CONFIGURE_DEPENDS ...)`
- [x] Generate `modules_loader.cpp` at configure time — defines `AddModuleScripts()`
- [x] Build `modules-loader` static library from generated file
- [x] Link `modules-loader` + all module libs into worldserver from root CMakeLists.txt
- [x] Hook `AddModuleScripts()` into `AddScripts()` in `ScriptLoader.cpp.in.cmake` (static builds only)

### How to create a module

```
modules/mod-myfeature/
    CMakeLists.txt       — calls add_argus_module(NAME "MyFeature")
    src/
        mod_myfeature.cpp — defines void AddMyFeatureScripts()
        mod_myfeature.h
    conf/                — optional: .conf.dist files
    sql/                 — optional: database migrations
```

### First Modular Targets (future)

- [ ] Move `world_solo_lfg.cpp` into `modules/mod-solo-lfg/`
- [ ] Custom commands module
- [ ] QoL systems module

### Do NOT

- Modularize maps yet
- Modularize networking yet
- Modularize object manager yet

### Validation

- [ ] Modules load correctly
- [ ] Server remains stable
- [ ] Gameplay unchanged

---

## Phase 8 — Safe Async Systems

**Goals:** Introduce safe asynchronous systems without touching gameplay state.

### Safe Async Targets

- [ ] Logging
- [ ] Database workers
- [ ] Path generation
- [ ] Map loading
- [ ] Compression tasks

### Rules

- No gameplay state sharing between threads
- Use queues/events for cross-thread communication
- Avoid shared mutable state

### Do NOT

- Thread combat systems
- Thread movement systems
- Thread visibility systems

### Validation

- [ ] No deadlocks
- [ ] No race conditions
- [ ] Stable uptime testing (48h+)

---

## Phase 9 — Map Threading Research

**Goals:** Carefully research multithreaded map updates. Experimental only.

### Tasks

- [ ] Add map update profiling
- [ ] Measure update bottlenecks per map type
- [ ] Isolate map update responsibilities
- [ ] Build experimental worker model on a dedicated branch

### Rules

- **Experimental branch only** — never merge unstable threading work to main
- Keep deterministic gameplay behavior

### Critical Systems (must remain single-threaded until proven safe)

- Combat
- Visibility
- Auras
- Movement
- Phasing
- Instances

### Do NOT

- Multithread everything at once
- Allow unsafe shared object access

### Validation

- [ ] No desync
- [ ] No invisible entities
- [ ] No combat corruption
- [ ] No movement glitches

---

## Long-Term Goals

- Cleaner engine architecture
- Reduced global state
- Better APIs
- Improved scripting
- Better maintainability and scalability
- Stable multithreaded world systems

### Final Vision

> **ArgusCore should become: "A stable, modern, maintainable Legion emulator framework."**
>
> **NOT: "A massive rewrite that constantly breaks gameplay."**
