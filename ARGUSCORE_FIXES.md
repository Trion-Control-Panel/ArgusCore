# ArgusCore Fix Backlog

Tracks the incremental security/stability/correctness/blizzlike maintenance pass.
One fix per entry. See repo root workflow conventions for process rules.

Legend: `[ ]` open · `[WIP]` in progress · `[DONE]` shipped

---

## P0 — Security / Corruption

### [DONE] Core/Guild - Arbitrary rank assignment via CMSG_GUILD_ASSIGN_MEMBER_RANK

**Subsystem:** Guilds/Guild, Handlers/GuildHandler

**Problem:** `Guild::HandleSetMemberRank` (invoked from
`WorldSession::HandleGuildAssignRank`, `CMSG_GUILD_ASSIGN_MEMBER_RANK`) only
checked that the acting player's rank has the boolean `GR_RIGHT_PROMOTE`/
`GR_RIGHT_DEMOTE` right and that the target isn't themself. It never compared the
target's current rank or the requested new rank against the acting player's own
rank order. Its sibling function `HandleUpdateMemberRank` (the classic
promote/demote-by-one-step path) correctly enforces both: you can't act on a
member who already outranks or equals you, and you can't grant a rank at or above
your own. `HandleSetMemberRank` (the "set to an arbitrary rank" path used by the
guild rank-management UI, which lets an officer drag a member directly to a
specific rank) had neither check.

**Files:** `src/server/game/Guilds/Guild.cpp` (`HandleSetMemberRank`)

**Exploit:** Any member whose rank has `GR_RIGHT_PROMOTE` (a right commonly given
to trusted-but-non-leader officer ranks) could send
`CMSG_GUILD_ASSIGN_MEMBER_RANK` targeting any other non-self member with the
target rank order set to 0 (Guild Master), instantly making that member (e.g. a
colluding alt) the guild leader. Symmetrically, with `GR_RIGHT_DEMOTE`, an officer
could demote the actual Guild Master or any other higher-ranked member down to an
arbitrary lower rank, since the function never checked that the target's *current*
rank was below the actor's own.

**Reference:** Same missing check exists in DestinyCore's equivalent function —
inherited from upstream, not ArgusCore-specific, but concretely exploitable here.
Fix is precedented in-tree: `HandleUpdateMemberRank` (same file, ~line 1804)
already implements the correct rank-order comparisons for the step-wise
promote/demote path.

**Fix:** Added the same two rank-order checks `HandleUpdateMemberRank` already
uses, generalized for an arbitrary target rank: reject if the target's current
rank is at-or-above the actor's own rank, or if the requested new rank is
at-or-above the actor's own rank.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [DONE] Core/Guild - HandleSetBankTabInfo missing rank-right check

**Subsystem:** Guilds/Guild, Handlers/GuildHandler

**Problem:** `Guild::HandleSetBankTabInfo` (`CMSG_GUILD_BANK_UPDATE_TAB`) has no
rank-right check at all, unlike its siblings `HandleSetRankInfo`/`HandleBuyBankTab`
which gate on `_IsLeader()`. Any guild member within interaction range of the
guild bank object can rename bank tabs and change their icons — no deposit/
withdraw/tab-management rights required.

**Files:** `src/server/game/Guilds/Guild.cpp` (`HandleSetBankTabInfo`)

**Severity:** Low — cosmetic/griefing (tab name/icon), not a theft or data-integrity
issue. Same gap exists in DestinyCore.

**Fix:** Gated `HandleSetBankTabInfo` on `_IsLeader(session->GetPlayer())`,
matching the restriction already used by the closely-related `HandleBuyBankTab`
(tab purchase) and `HandleSetRankInfo` (rank editor) — both leader-only. Chosen
deliberately over guessing at a specific rank-right flag: restricting to
leader-only cannot introduce a new vulnerability, and tab creation/branding is
leader-only in the adjacent, already-correct `HandleBuyBankTab` path.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [DONE] Core/Player - Vendor purchase quantity desync (validate-vs-deliver truncation)

**Subsystem:** Entities/Player, item vendor purchase

**Problem:** `Player::BuyItemFromVendorSlot(..., uint32 count, ...)` validates stock
limits, gold price, and extended-cost reagent/currency ownership against the full
client-supplied `count` (an `int32` read directly off `CMSG_BUY_ITEM`'s `Quantity`
field, widened to `uint32`). It then forwards `count` into
`Player::_StoreOrEquipNewItem(..., uint8 count, ...)`, which silently narrows it to
`count % 256`. Actual item delivery, reagent/currency consumption
(`DestroyItemCount`/`RemoveCurrency`), and vendor stock decrement
(`UpdateVendorItemCurrentCount`) all use the truncated value, while the "can you
afford/own this" checks earlier used the untruncated value. A single crafted packet
with `Quantity > 255` desyncs what was validated from what actually executes —
a client-triggerable economic integrity fault.

**Files:** `src/server/game/Entities/Player/Player.cpp`
(`BuyItemFromVendorSlot`, `_StoreOrEquipNewItem`),
`src/server/game/Handlers/ItemHandler.cpp` (`HandleBuyItemOpcode`)

**Reference:** None directly ported; DestinyCore keeps `count` as `uint8` end-to-end
(no widening, so no desync there). ArgusCore widened the outer function to `uint32`
without updating the inner one.

**Fix:** Clamp `count` to `uint8` range immediately after the existing
`if (count < 1) count = 1;` cheat guard in `BuyItemFromVendorSlot`, before any
validation runs, so every downstream check validates the exact quantity that will
be charged and delivered.

**Commit:** `c6313837`

**Test:** Pending manual build/runtime verification.

---

## P1 — Stability

### [DONE] Core/Spells - Null-caster crash in weapon-percent DoT tick

**Subsystem:** Spells/Auras (SpellAuraEffects)

**Problem:** `AuraEffect::HandlePeriodicDamageAurasTick`'s
`SPELL_AURA_PERIODIC_WEAPON_PERCENT_DAMAGE` case dereferenced `caster`
unconditionally (`caster->CalculateDamage(...)`), while the `SPELL_AURA_PERIODIC_DAMAGE`
case immediately above it, and every other `caster` use later in the same function,
correctly guards with `if (caster)`. Traced the call chain: `Aura::UpdateOwner`
assigns `Unit* caster = GetCaster();` and explicitly handles the null case a few
lines later — `GetCaster()` returns null once the original caster is no longer on
the same map (logged out, zoned, disconnected). Non-channeled DoTs are
deliberately *not* removed when the caster leaves. Any weapon-percent-damage DoT
(a bleed-type effect) still ticking on a target after its caster leaves the map
hits this line and crashes the world thread on the next tick — reachable through
ordinary gameplay timing, no exploit needed.

**Files:** `src/server/game/Spells/Auras/SpellAuraEffects.cpp`

**Reference:** Same unguarded line exists in DestinyCore and upstream
TrinityCore-master — a long-standing latent bug in the whole TrinityCore lineage,
not an ArgusCore regression, but a live crash path here regardless.

**Fix:** Guarded the `caster->CalculateDamage(...)` call with the same
`caster ? ... : 0` pattern implied by the sibling case — if the caster is gone,
this tick deals no damage (weapon data is inherently caster-derived, so there's no
way to compute a meaningful value without it) rather than crashing; the aura
continues ticking harmlessly until it expires.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [ ] Core/Spells - Client-triggerable ASSERT in SelectImplicitTargetDestTargets

**Subsystem:** Spells/Spell

**Problem:** `Spell::SelectImplicitTargetDestTargets` (`Spell.cpp:1629-1634`) has
`ASSERT(m_targets.GetObjectTarget() || ...DontFailSpellOnTargetingFailure, ...)`
immediately followed by `if (!target) return;` on the very next line. `ASSERT` in
this codebase is not compiled out in release builds — it aborts the whole
process. The redundant null check right after the assert suggests the null case
is in fact reachable at runtime for some `TARGET_DEST_TARGET_*` spell effect
combinations, which would mean a spell missing the
`DontFailSpellOnTargetingFailure` attribute could abort the server via normal
(or crafted) cast target resolution. Not yet proven to trigger end-to-end against
current spell data — flagged as medium-confidence per the audit that found it.

**Files:** `src/server/game/Spells/Spell.cpp`

**Reference:** Same code exists upstream in TrinityCore-master; not an ArgusCore
regression. Matches the same class of bug as the already-fixed VehicleHandler
seat asserts (client-reachable path asserting on a value instead of gracefully
rejecting it).

**Status:** Not started. Needs confirming whether the null case is actually
reachable with current spell_effect data before deciding between "replace assert
with early return" vs. "leave as an internal invariant."

### [ ] Core/Spells - EffectUntrainTalents checks caster type instead of target type

**Subsystem:** Spells/SpellEffects

**Problem:** `Spell::EffectUntrainTalents` (`SpellEffects.cpp:2243-2252`) guards
with `if (!unitTarget || m_caster->GetTypeId() == TYPEID_PLAYER) return;` then
unconditionally calls `unitTarget->ToPlayer()->SendRespecWipeConfirm(...)` — the
type check is on `m_caster`, not `unitTarget`, so if `unitTarget` were ever a
non-player unit, `ToPlayer()` would return null and crash.

**Files:** `src/server/game/Spells/SpellEffects.cpp`

**Severity:** Low/likely unreachable — `SPELL_EFFECT_UNTRAIN_TALENTS` is only used
by trainer spells server-cast at the interacting player, so `unitTarget` isn't
normally client-controlled. Same pattern exists in DestinyCore. Flagged for
completeness.

**Status:** Not started. Cheap one-line fix
(`unitTarget->GetTypeId() != TYPEID_PLAYER`) whenever picked up.

### [DONE] Core/Unit - TriggerAuraHeartbeat iterator invalidation

**Subsystem:** Entities/Unit, Spells/Auras

**Problem:** `Unit::TriggerAuraHeartbeat()` (`Unit.cpp:513-519`) iterates
`m_appliedAuras` (a `std::multimap`) with a plain range-based `for` while calling
`Aura::Heartbeat()`, which invokes arbitrary `AuraScript::OnHeartbeat` hooks. If a
heartbeat hook removes the aura currently being visited (a normal aura-scripting
pattern — "cancel self if condition no longer met"), `Unit::_UnapplyAura` erases
that exact map node mid-loop, which is undefined behavior for the range-for's
hidden iterator.

**Files:** `src/server/game/Entities/Unit/Unit.cpp`

**Reference:** Not present in DestinyCore (the heartbeat-aura mechanic is
Legion-specific). Fix is precedented in-tree: `Unit::AtEnterCombat()` /
`Unit::AtExitCombat()` (same file, ~line 8774) invoke the same class of arbitrary
aura-script hook over the same container using an erase-safe idiom (capture the
pointer, advance the iterator, *then* invoke the hook).

**Fix:** Convert `TriggerAuraHeartbeat`'s loop to the same erase-safe idiom already
used by `AtEnterCombat`/`AtExitCombat`.

**Commit:** `6e532be2`

**Test:** Pending manual build/runtime verification.

### [DONE] Core/Vehicle - Null seat dereference in VehicleHandler

**Subsystem:** Handlers/VehicleHandler, Entities/Vehicle

**Problem:** `HandleRequestVehiclePrevSeat`, `HandleRequestVehicleNextSeat`,
`HandleMoveChangeVehicleSeats`, and `HandleRequestVehicleSwitchSeat`
(`VehicleHandler.cpp:45-113`) call `Vehicle::GetSeatForPassenger()`, which is
documented to return null if the passenger isn't currently seated, and dereference
the result without a null check (contrast `CombatHandler.cpp:51-52`, which at
least `ASSERT`s). A rapid double-send of a vehicle-seat opcode during a seat-change
race window is a plausible client-triggerable null deref.

**Files:** `src/server/game/Handlers/VehicleHandler.cpp`

**Reference:** Same missing check exists in DestinyCore — inherited from upstream
TrinityCore, not ArgusCore-specific. Lower priority than the heartbeat bug since
it's long-standing and shared across forks, but still a live crash bug.

**Fix:** Added `if (!seat) return;` immediately after each `GetSeatForPassenger()`
call, before the existing `seat->CanSwitchFromSeat()` dereference, in all four
handlers. `GetVehicleBase()`/`GetVehicle()` share the same underlying `m_vehicle`
pointer (verified via `Unit::GetVehicleBase()`), so the already-present
`vehicle_base` null check does not guarantee `GetSeatForPassenger()` succeeds —
only that the player is attached to *a* vehicle, not that they're currently
registered in its `Seats` map.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [DONE] Core/Loot - Unguarded division in HandleLootMoneyOpcode

**Subsystem:** Handlers/LootHandler

**Problem:** `uint32 goldPerPlayer = uint32((loot->gold) / (playersNear.size()));`
(`LootHandler.cpp:~168`) has no guard against `playersNear` being empty. In normal
play the looter is always included, so this is low-exploitability today, but it's
an unguarded integer division on a client-triggerable path (`CMSG_LOOT_MONEY`).

**Files:** `src/server/game/Handlers/LootHandler.cpp`

**Reference:** Same pattern exists in DestinyCore; not ArgusCore-specific.

**Evidence this is reachable, not just theoretical:** `_allowedLooters` for
round-robin group loot can rotate to a different player while an earlier looter's
client still has a stale loot window open (round-robin timeout mid-interaction).
If the current round-robin looter has since moved away from the corpse (fails
`IsAtGroupRewardDistance`) and the player sending `CMSG_LOOT_MONEY` has already
rotated out of `_allowedLooters`, `playersNear` ends up empty and the division
crashes the world thread.

**Fix:** Wrapped the `goldPerPlayer` division and payout loop in
`if (!playersNear.empty())`. If nobody currently qualifies, the shared-money
payout is skipped for that loot object; the surrounding code already removes/marks
the loot regardless (`NotifyMoneyRemoved`, `LootMoney()`), so this only affects the
already-rare case where no eligible recipient exists.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

---

## P2 — Core Correctness

### [DONE] Core/Bank - AutoStoreBankReagent reads Slot/PackSlot in wrong wire order

**Subsystem:** Server/Packets (BankPackets), Handlers/BankHandler

**Problem:** `HandleAutoBankReagentOpcode` calls
`GetItemByPos(autoBankReagent.PackSlot, autoBankReagent.Slot)` while
`HandleAutoStoreBankReagentOpcode` calls
`GetItemByPos(autoStoreBankReagent.Slot, autoStoreBankReagent.PackSlot)` — reversed
argument order relative to its sibling. Root cause traced to
`AutoStoreBankReagent::Read()` in `BankPackets.cpp`, which reads the wire fields
`Slot` then `PackSlot`, the opposite order from `AutoBankReagent::Read()`'s
`PackSlot` then `Slot`. The handler itself is internally self-consistent with its
own (wrongly-ordered) `Read()`, which is why this wasn't a crash — just a
functional bug where the reagent-bank auto-store action resolves the wrong
bag/slot pair.

**Files:** `src/server/game/Server/Packets/BankPackets.cpp`

**Reference:** DestinyCore's equivalent packets (`Bag`/`Slot` naming) read both
`AutoBankReagent::Read()` and `AutoStoreBankReagent::Read()` in the identical
`Bag, Slot` order, and both of its handlers call `GetItemByPos(Bag, Slot)`
consistently — corroborating that ArgusCore's `AutoStoreBankReagent::Read()` has
the swapped order, not `AutoBankReagent::Read()`.

**Fix:** Swapped the two lines in `AutoStoreBankReagent::Read()` to read
`PackSlot` before `Slot`, matching `AutoBankReagent::Read()`. This is the single
root-cause fix; no changes needed in `BankHandler.cpp` since its field usage was
already internally consistent — it will now use the correctly-parsed values.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

---

## P3 — Blizzlike Gameplay

(Not populated by this pass — see `ROADMAP.md` / `plans/00-arguscore-analysis-and-roadmap.md`
for the active class-spell-porting and content workstreams, which are tracked
separately from this security/stability backlog.)

---

## P4 — Performance / Cleanup

(Not populated by this pass.)

---

## Notes on scope

This backlog is deliberately shallow on first pass — it reflects two targeted,
read-only audits (item/gold/trade exploit surface; crash/stability surface) rather
than an exhaustive line-by-line review. Areas checked and found solidly guarded
during this pass (not re-audited unless something changes nearby): `TradeHandler`,
`MailHandler` (including mail money/COD overflow guards), `AuctionHouseHandler`,
`GuildHandler`/`Guild.cpp` bank-item moves, `VoidStorageHandler`, movement/teleport
coordinate validation (`MovementHandler.cpp`, `ValidateMovementInfo`), `Spell.cpp`/
`SpellEffects.cpp` target-pointer handling, `LootHandler.cpp` target/master-loot
lookups, `Group.cpp` aura/member iteration.
