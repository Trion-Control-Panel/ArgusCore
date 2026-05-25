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
| 2 | [Custom Script Integration](#phase-2--custom-script-integration) | **Complete** |
| 3 | [Build System Cleanup](#phase-3--build-system-cleanup) | **Validated** |
| 4 | [Dependency Updates](#phase-4--dependency-updates) | **Validated** |
| 5 | [Code Modernization](#phase-5--code-modernization) | **Complete** |
| 6 | [CI, Testing, and Profiling](#phase-6--ci-testing-and-profiling) | **Complete** |
| 7 | [Modular System](#phase-7--modular-system) | **Complete** |
| 8 | [Safe Async Systems](#phase-8--safe-async-systems) | **Complete** |
| 9 | [Map Threading Research](#phase-9--map-threading-research) | **Complete** |
| 10 | [World Layering](#phase-10--world-layering) | **Complete** |
| 11 | [NPC Gameplay Systems](#phase-11--npc-gameplay-systems) | **In progress** |
| 12 | [Class Spell Restoration](#phase-12--class-spell-restoration) | **Not started** |

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
- [x] Confirm the script tree in CMake output no longer shows the TLPP entry
- [x] Update DB: `creature_template.ScriptName` for entry 29264 → `npc_sha_feral_spirit` (sql/updates/world/master/2026_05_19_00_world.sql)

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
3. [x] MySQL — installed 8.4.9 (current LTS); floor kept at 8.0.34 for Linux CI compat (Ubuntu 24.04 apt provides 8.0.x); Windows production uses 8.4.9
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

- [x] Authentication works
- [x] Database queries work
- [x] Characters save/load correctly
- [x] Worldserver stability confirmed

---

## Phase 5 — Code Modernization

**Goals:** Introduce modern C++ slowly. Keep identical behavior.

### Tasks

- [x] Replace `NULL` with `nullptr` (2,942 occurrences across src/)
- [x] Replace `typedef` with `using`
- [x] Introduce `constexpr` safely
- [x] Introduce `enum class` safely — surveyed; all remaining plain enums have wide usage (30+ files), no safe isolated candidates; deferred
- [x] Replace obvious ownership raw pointers with `unique_ptr`
- [x] Remove dead code carefully — codebase is clean; no #if 0 blocks, no dead functions, no large commented sections

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
- [x] Add ASAN build job to linux workflow
- [x] Add UBSAN build job to linux workflow
- [x] Add profiling tools — profile CI job (RelWithDebInfo + -fno-omit-frame-pointer) for offline perf/valgrind use
- [x] Add basic performance metrics collection — DB latency (adhoc_read, adhoc_write, prepared_write) wired into MySQLConnection; map/world/session metrics were already in place

### Metrics to Track

- Map update times
- Player update times
- Packet queue sizes
- DB latency

### Do NOT

- Start multithreading yet
- Optimize blindly

### Outstanding

- [ ] Investigate general server slowness — server is playable but noticeably slow; profile with InfluxDB metrics (map update times, DB latency, session update times) to identify the bottleneck before any optimization work begins

### Validation

- [x] Sanitizers clean
- [ ] Stable long runtime testing (24h+)
- [x] No memory corruption

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

- [x] Move `world_solo_lfg.cpp` into `modules/mod-solo-lfg/` and `modules/mod-solocraft/` — merged AzerothCore mod-solocraft feature set (per-dungeon tables Classic→Legion, class balance, XP balancing, group offset balancing) with ArgusCore TrinityCore API; old core file deleted
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

- [x] Logging — already async via Asio Strand when IoContext is provided
- [x] Database workers — already async via IoContext worker pool; sync path exists as fallback
- [x] Path generation — MMAP tile files warmed by TerrainPreloader (same background I/O as map loading)
- [x] Map loading — TerrainPreloader background page-cache warmer; adjacent tiles prefetched after each grid activation
- [x] Compression tasks — already on isolated network thread (per-socket z_stream, no game state)

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

- [x] Add map update profiling — 8 TC_METRIC_TIMER blocks inside Map::Update(): dynamic_tree, sessions, respawns, player_grid, active_objects, transports, send_objects, relocations
- [x] Measure update bottlenecks per map type — map_type tag (world/dungeon/raid/bg/scenario) added to all 8 subsystem timers and the outer map_update_time_diff metric; collect live data by running the profiling build with InfluxDB enabled
- [x] Isolate map update responsibilities — DelayedUpdate() confirmed per-map isolated (far spell callbacks, object removal, grid state are all local to one map)
- [x] Build experimental worker model on a dedicated branch — MapUpdater refactored to std::function<void()> queue; DelayedUpdate() now runs in parallel across maps via schedule_delayed_update(); Map::Update() path unchanged

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

## Phase 10 — World Layering

**Goals:** Split high-population open-world maps into parallel independent instances ("layers") so a single map tick never has to process thousands of players at once. Players on different layers cannot see each other. Layers are merged when population drops.

**Prerequisites:** Phase 9 map threading must be stable and profiled before this begins.

### Why

The current model runs one `Map` object per open-world zone. At high population (Dalaran, Broken Isles, Argus) this becomes a single-threaded bottleneck. Layering distributes the load across N independent copies of the same map, each updated in its own worker slot.

### How It Works

1. `MapManager` can create multiple `Map*` instances for the same `mapId` — one per layer
2. A `LayerManager` assigns incoming players to the least-populated layer
3. Each layer runs its own spawn state, combat, and visibility — fully isolated
4. When a layer's population falls below a threshold, players are migrated and the layer is destroyed
5. Cross-layer systems (whisper, trade, group, guild) are unaffected — they are player-scoped, not map-scoped

### Tasks

- [ ] Research existing layering implementations (CMaNGOS, MaNGOS Zero)
- [ ] Design `LayerManager` — assignment, rebalancing, merge/split thresholds
- [ ] Extend `MapManager` to allow multiple `Map*` per `mapId` (open world only — dungeons/raids/BGs are already instanced)
- [ ] Implement player layer assignment on zone entry
- [ ] Implement layer migration (teleport-in-place with state handoff)
- [ ] Persist last-known layer in `characters` DB (for reconnects)
- [ ] Add layer ID to visibility system so cross-layer units are invisible
- [ ] Enforce group co-location — on group join, migrate the joining player to the leader's layer; on leader change, migrate group to new leader's layer
- [ ] Stress test with bot population before real players

### Rules

- **Experimental branch only** until fully proven stable
- Open-world maps only — dungeons, raids, battlegrounds, and scenarios are already instanced
- Never merge layers mid-combat
- Layer migration must be invisible to the client (no loading screen)
- Players in the same group are always assigned to the same layer — when a player joins a group, they are migrated to the group leader's layer immediately; when a group disbands, members remain on their current layer

### Critical Systems Affected

- `MapManager` — multi-instance per mapId
- `Map::Update()` — each layer is an independent map update
- `WorldSession` — must track which layer the player is on
- `ObjectVisibility` — cross-layer units must be invisible to each other
- `TransportMgr` — transports that cross zones need layer-aware routing

### Do NOT

- Layer dungeons, raids, battlegrounds, or scenarios — they are already instanced
- Merge layers while players are in combat
- Share mutable spawn or aura state between layers

### Validation

- [ ] No cross-layer visibility leaks
- [ ] No position desyncs on layer migration
- [ ] Spawn state fully isolated per layer
- [ ] Layer merge produces no duplicate NPCs or items
- [ ] Group members always visible to each other (always on same layer)
- [ ] Stable at 500+ players per realm across multiple layers

---

## Phase 11 — NPC Gameplay Systems

**Goals:** Make NPC combat and movement behave closer to blizzlike. Each feature is isolated and tested independently before the next begins.

### Completed

- [x] NPC pathfinding: lower `walkableClimb` in MMAP extractor so fences are impassable walls instead of walkable steps (requires MMAP regeneration)
- [x] NPC obstacle jumping: humanoid NPCs jump over short obstacles (< 1.5y) when chasing instead of routing around — `PathGenerator` detects jumpable direct path, `ChaseMovementGenerator` executes `MoveJump`

### NPC Power Types

Make NPCs use the correct resource bar (mana, rage, energy, focus, etc.) based on their class, matching blizzlike behavior.

**Three levels — implement in order, validate before moving to the next:**

#### Level 1 — Display only (database)
Set `unit_class` in `creature_template` so the correct power bar is shown to players. No code changes.

- [ ] Audit creature_template entries: confirm unit_class is set correctly for named humanoid NPCs (vendors, guards, bosses)
- [ ] Write SQL update for missing or wrong unit_class values
- [ ] Validate in-game: mage NPC shows mana bar, warrior shows rage bar

#### Level 2 — Actual resource consumption (code)
NPC spells consume mana/rage/energy when cast. A mage NPC can run out of mana.

- [ ] Audit where spell cost checks are bypassed for NPCs in the spell system
- [ ] Remove or conditionalize the bypass so NPCs pay spell costs
- [ ] Validate: mage NPC mana depletes under sustained casting pressure
- [ ] Validate: no regression on boss scripts that rely on unlimited resources

#### Level 3 — Full AI resource management (code + AI)
NPCs regenerate resources, conserve energy, build rage on hits, and adjust ability usage based on current resource levels.

- [ ] Implement class-appropriate power regeneration rates for NPCs (mana regen, energy tick, rage generation on hit/damage)
- [ ] Add resource awareness to creature AI base class: expose `GetPowerPercent()` for script conditions
- [ ] Update key humanoid NPC scripts to use resource-gated ability priority (e.g. do not cast if OOM, use filler if low energy)
- [ ] Validate: warrior NPCs generate and spend rage naturally in combat
- [ ] Validate: mage NPCs use filler spells when low on mana

### Rules

- Level 2 must not break any existing boss scripts that depend on NPCs having infinite resources
- Level 3 is opt-in per script — do not apply globally without profiling the AI overhead
- Test every level in-game before starting the next

### Do NOT

- Change player power systems
- Touch battleground or raid boss scripts during Level 2/3 rollout
- Mix power system changes with movement or combat refactors

### Validation

- [ ] Level 1: all major humanoid NPC types show correct power bar
- [ ] Level 2: resource depletion works, no boss script regressions
- [ ] Level 3: NPC combat feels natural, no AI performance regression

---

## Phase 12 — Class Spell Restoration

**Goals:** Port Legion-era spell scripts from `logs/LegionCore-7.3.5-merged` into ArgusCore one spell at a time. Each spell is verified in-game before the next begins.

### Why

ArgusCore's `src/server/scripts/Spells/` has post-Legion (BfA/Shadowlands) implementations for several classes, but is missing a large body of 7.3.5-era implementations that existed in original Legion-era codebases. LegionCore contains those original scripts. Porting them restores correct Legion behavior for spells that currently do nothing or behave incorrectly.

### Source Reference

All source scripts are in:
`C:\Users\Administrator\Documents\GitHub\ArgusCore\logs\LegionCore-7.3.5-merged\src\server\scripts\Spells\`

Target files are in:
`C:\Users\Administrator\Documents\GitHub\ArgusCore\src\server\scripts\Spells\`

### Rules

- Port **one struct at a time** — compile and test before moving to the next
- Do NOT copy blindly — read what the script does and confirm it matches 7.3.5 spell IDs in the DBC
- If ArgusCore already has a newer (more complete) implementation of the same spell, skip it
- Keep LegionCore comments intact where they explain a design decision
- Priest and Shaman are skipped — ArgusCore already has more complete implementations than LegionCore

### Commit format

```
feat(scripts/spell): port spell_monk_chi_wave from LegionCore
```

---

### Death Knight (36 missing)

Source: `spell_dk.cpp` (LegionCore) → `spell_dk.cpp` (ArgusCore)

- [ ] `spell_dk_marrowrend`
- [ ] `spell_dk_bone_shield`
- [ ] `spell_dk_tombstone`
- [ ] `spell_dk_bonestorm`
- [ ] `spell_dk_blood_mirror`
- [ ] `spell_dk_defile`
- [ ] `spell_dk_breath_of_sindragosa`
- [ ] `spell_dk_apocalypse`
- [ ] `spell_dk_festering_strike`
- [ ] `spell_dk_festering_wound`
- [ ] `spell_dk_scourge_strike`
- [ ] `spell_dk_scourge_strike_trigger`
- [ ] `spell_dk_frost_strike`
- [ ] `spell_dk_glacial_advance`
- [ ] `spell_dk_glacial_advance_damage`
- [ ] `spell_dk_pillar_of_frost`
- [ ] `spell_dk_will_of_the_necropolis`
- [ ] `spell_dk_purgatory`
- [ ] `spell_dk_purgatory_absorb`
- [ ] `spell_dk_desecrated_ground`
- [ ] `spell_dk_anti_magic_shell_self`
- [ ] `spell_dk_anti_magic_shell_raid`
- [ ] `spell_dk_anti_magic_zone`
- [ ] `spell_dk_gorefiends_grasp`
- [ ] `spell_dk_death_grip_dummy`
- [ ] `spell_dk_death_gate`
- [ ] `spell_dk_death_gate_teleport`
- [ ] `spell_dk_death_pact`
- [ ] `spell_dk_ghoul_explode`
- [ ] `spell_dk_corpse_explosion`
- [ ] `spell_dk_corpse_shield`
- [ ] `spell_dk_frost_shield`
- [ ] `spell_dk_necrotic_strike`
- [ ] `spell_dk_consumption`
- [ ] `spell_dk_change_duration`
- [ ] `spell_dk_hook`

---

### Monk (30 missing)

Source: `spell_monk.cpp` (LegionCore) → `spell_monk.cpp` (ArgusCore)

- [ ] `spell_monk_enveloping_mist`
- [ ] `spell_monk_guard`
- [ ] `spell_monk_gift_of_the_ox`
- [ ] `spell_monk_sheiluns_gift`
- [ ] `spell_monk_touch_of_karma`
- [ ] `spell_monk_touch_of_death`
- [ ] `spell_monk_dampen_harm`
- [ ] `spell_monk_diffuse_magic`
- [ ] `spell_monk_purifying_brew`
- [ ] `spell_monk_purified_healing`
- [ ] `spell_monk_hurricane_strike`
- [ ] `spell_monk_whirling_dragon_punch_activater`
- [ ] `spell_monk_whirling_dragon_punch_activated`
- [ ] `spell_monk_expel_harm`
- [ ] `spell_monk_zen_pulse`
- [ ] `spell_monk_chi_wave`
- [ ] `spell_monk_chi_wave_dummy`
- [ ] `spell_monk_chi_wave_filter`
- [ ] `spell_monk_storm_earth_and_fire`
- [ ] `spell_monk_storm_earth_and_fire_clone_visual`
- [ ] `spell_monk_clone_cast`
- [ ] `spell_monk_flying_serpent_kick`
- [ ] `spell_monk_disable`
- [ ] `spell_monk_transcendence`
- [ ] `spell_monk_transcendence_transfer`
- [ ] `spell_monk_zen_pilgrimage`
- [ ] `spell_monk_zen_pilgrimage_return`
- [ ] `spell_monk_remove_zen_flight`
- [ ] `spell_monk_zen_flight_check`
- [ ] `spell_monk_power_strikes`

---

### Demon Hunter

Source: `spell_dh.cpp` (LegionCore) → `spell_dh.cpp` (ArgusCore)

- [x] `spell_dh_metamorphosis_main` — covered by `spell_dh_chaotic_transformation` + `spell_dh_demonic` in ArgusCore
- [x] `spell_dh_metamorphosis` (200166 impact) — ported 2026-05-25 as `spell_dh_metamorphosis_impact`
- [x] `spell_dh_blade_dance` — ArgusCore has a more complete version
- [x] `spell_dh_fel_rush_main` — ArgusCore has a more complete version
- [x] `spell_dh_fel_rush` (192611 Loramus talent) — artifact-only, not relevant in 7.3.5 fresh realm
- [ ] `spell_dh_soul_cleave` — blocked: requires `GetAreaObjectList` (LegionCore custom API)
- [ ] `spell_dh_soul_cleave_damage` — blocked: same as above
- [x] `spell_dh_shatter_soul` — ported 2026-05-25
- [ ] `spell_dh_spirit_bomb` — blocked: requires `GetAreaObjectList`
- [ ] `spell_dh_spirit_bomb_damage` — blocked: requires `GetAreaObjectList`
- [x] `spell_dh_anguish` — ported 2026-05-25
- [x] `spell_dh_anguish_damage` — ported 2026-05-25
- [x] `spell_dh_glide` — ArgusCore has a more complete version
- [x] `spell_dh_nemesis` — ported 2026-05-25
- [x] `spell_dh_disable_absorb` — ArgusCore has `spell_dh_last_resort` (more complete)
- [x] `spell_dh_desperate_instincts` — ported 2026-05-25
- [ ] `spell_dh_soul_barrier` — blocked: requires `GetAreaObjectList`
- [x] `spell_dh_darkness` — ArgusCore has a more complete version
- [ ] `spell_dh_empower_wards` — deferred: complex accumulator mechanic (218910/218713/218561 chain)
- [x] `spell_dh_reverse_magic` — ported 2026-05-25
- [ ] `spell_dh_nether_bond` — blocked: requires `GetAura()->GetRndEffectTarget()` (LegionCore custom API)
- [ ] `spell_dh_nether_bond_dummy` — blocked: same as above
- [ ] `spell_dh_illidans_grasp_throw` — blocked: requires `m_whoHasMyAuras` (LegionCore custom API)
- [ ] `spell_dh_illidans_grasp_visual` — blocked: same as above
- [x] `spell_dh_eye_of_leotheras` — ported 2026-05-25
- [x] `spell_dh_mana_break` — ported 2026-05-25
- [x] `spell_dh_mana_rift` — ported 2026-05-25
- [x] `spell_dh_fel_lance` — ported 2026-05-25
- [x] `spell_dh_shattered_souls` (178940/204254) — ported 2026-05-25
- [x] `spell_dh_flaming_soul` (238118) — ported 2026-05-25
- [x] `spell_dh_fueled_by_pain` (213017) — ported 2026-05-25

---

### Paladin (24 missing)

Source: `spell_paladin.cpp` (LegionCore) → `spell_paladin.cpp` (ArgusCore)

- [ ] `spell_pal_shield_of_the_righteous`
- [ ] `spell_pal_divine_storm`
- [ ] `spell_pal_holy_shock`
- [ ] `spell_pal_holy_shield`
- [ ] `spell_pal_holy_prism`
- [ ] `spell_pal_holy_prism_effect`
- [ ] `spell_pal_holy_prism_heal`
- [ ] `spell_pal_holy_prism_damage`
- [ ] `spell_pal_lights_hammer`
- [ ] `spell_pal_ardent_defender`
- [ ] `spell_pal_shield_of_vengeance`
- [ ] `spell_pal_light_of_the_protector`
- [ ] `spell_pal_last_defender`
- [ ] `spell_pal_at_last_defender`
- [ ] `spell_pal_at_aura_of_sacrifice`
- [ ] `spell_pal_at_devotion_aura`
- [ ] `spell_pal_lay_on_hands`
- [ ] `spell_pal_divine_steed`
- [ ] `spell_pal_zeal`
- [ ] `spell_pal_holy_wrath`
- [ ] `spell_pal_greater_blessing_of_kings`
- [ ] `spell_pal_divine_shield`
- [ ] `spell_pal_hand_of_protection`
- [ ] `spell_pal_divine_intervention`

---

### Warlock (16 missing)

Source: `spell_warlock.cpp` (LegionCore) → `spell_warlock.cpp` (ArgusCore)

- [ ] `spell_warl_corruption`
- [ ] `spell_warl_unstable_affliction`
- [ ] `spell_warl_unstable_affliction_R2`
- [ ] `spell_warl_streten_insanity`
- [ ] `spell_warl_seed_of_corruption_dota`
- [ ] `spell_warl_doom_bolt`
- [ ] `spell_warl_soul_harvest`
- [ ] `spell_warl_burning_rush`
- [ ] `spell_warl_banish`
- [ ] `spell_warl_demonic_circle_summon`
- [ ] `spell_warl_demonic_circle_teleport`
- [ ] `spell_warl_demonic_gateway`
- [ ] `spell_warl_demonic_gateway_cast`
- [ ] `spell_warl_demonic_gateway_duration`
- [ ] `spell_warl_demonic_gateway_at`
- [ ] `spell_warl_demon_skin`

---

### Hunter (15 missing)

Source: `spell_hunter.cpp` (LegionCore) → `spell_hunter.cpp` (ArgusCore)

- [ ] `spell_hun_kill_command`
- [ ] `spell_hun_dire_beast`
- [ ] `spell_hun_a_murder_of_crows`
- [ ] `spell_hun_beast_cleave`
- [ ] `spell_hun_flanking_strike`
- [ ] `spell_hun_cobra_shot`
- [ ] `spell_hun_explosive_shot`
- [ ] `spell_hun_explosive_shot_detonate`
- [ ] `spell_hun_explosive_trap`
- [ ] `spell_hun_ancient_hysteria`
- [ ] `spell_hun_masters_call`
- [ ] `spell_hun_pet_heart_of_the_phoenix`
- [ ] `spell_hun_tame_beast`
- [ ] `spell_hun_fetch`
- [ ] `spell_hun_fireworks`

---

### Warrior (12 missing)

Source: `spell_warrior.cpp` (LegionCore) → `spell_warrior.cpp` (ArgusCore)

- [ ] `spell_warr_execute`
- [ ] `spell_warr_bloodthirst`
- [ ] `spell_warr_revenge`
- [ ] `spell_warr_shield_block`
- [ ] `spell_warr_heroic_leap`
- [ ] `spell_warr_ravager`
- [ ] `spell_warr_ravager_visual`
- [ ] `spell_warr_intercept`
- [ ] `spell_war_intervene`
- [ ] `spell_warr_charge_check_cast`
- [ ] `spell_warr_charge_drop_fire`
- [ ] `spell_warr_fervor_of_battle`

---

### Druid (12 missing)

Source: `spell_druid.cpp` (LegionCore) → `spell_druid.cpp` (ArgusCore)

- [ ] `spell_dru_rip`
- [ ] `spell_dru_rake`
- [ ] `spell_dru_shred`
- [ ] `spell_dru_thrash`
- [ ] `spell_dru_ferocious_bite`
- [ ] `spell_dru_ashamanes_frenzy`
- [ ] `spell_dru_lifebloom`
- [ ] `spell_dru_incarnation`
- [ ] `spell_dru_incarnation_tree_of_life`
- [ ] `spell_dru_cat_form`
- [ ] `spell_dru_travel_form`
- [ ] `spell_dru_travel_form_remove`

---

### Mage (8 missing)

Source: `spell_mage.cpp` (LegionCore) → `spell_mage.cpp` (ArgusCore)

- [ ] `spell_mage_cauterize`
- [ ] `spell_mage_arcane_barrage`
- [ ] `spell_mage_flurry`
- [ ] `spell_mage_illusion`
- [ ] `spell_mage_flameglow`
- [ ] `spell_mage_erosion`
- [ ] `spell_mage_displacement`
- [ ] `spell_elem_invisibility`

---

### Rogue (1 missing)

Source: `spell_rogue.cpp` (LegionCore) → `spell_rogue.cpp` (ArgusCore)

- [ ] `spell_rog_shadowy_duel_main`

---

### Do NOT

- Port scripts for spells that do not exist in 7.3.5 DBC data
- Port Priest or Shaman — ArgusCore's implementations are already more complete
- Change spell IDs or aura IDs without verifying against the 7.3.5 DBC
- Port multiple spells in one commit

### Validation

After each spell is ported:
- [ ] Server compiles
- [ ] Spell fires and produces visible effect in-game
- [ ] No crash on cast
- [ ] No regression on related spells of the same spec

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
