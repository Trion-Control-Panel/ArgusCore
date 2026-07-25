# ArgusCore Fix Backlog

Tracks the incremental security/stability/correctness/blizzlike maintenance pass.
One fix per entry. See repo root workflow conventions for process rules.

Legend: `[ ]` open · `[WIP]` in progress · `[DONE]` shipped

---

## P0 — Security / Corruption

### [DONE] Core/Pet - Out-of-bounds write via CMSG_PET_SET_ACTION index

**Subsystem:** Entities/Unit (CharmInfo), Handlers/PetHandler

**Problem:** `HandlePetSetAction` reads `packet.Index` as a raw client-supplied
`uint32` and passes it straight to `CharmInfo::SetActionBar(uint8 index, ...)`,
which indexed the fixed 10-element `PetActionBar` array
(`MAX_UNIT_ACTION_BAR_INDEX = 10`) with zero bounds checking. Any player with
an active pet could send `CMSG_PET_SET_ACTION` with an index outside 0-9,
writing attacker-controlled spell/type data past the end of the array into
adjacent `CharmInfo` memory — a genuine out-of-bounds write / memory-corruption
primitive, not just a crash.

**Files:** `src/server/game/Entities/Unit/CharmInfo.h`

**Reference:** Identical missing bounds check confirmed in DestinyCore — a
shared upstream defect, not ArgusCore-introduced, but live and exploitable
here.

**Fix:** Added a bounds check (`index >= MAX_UNIT_ACTION_BAR_INDEX`) directly
inside `CharmInfo::SetActionBar` itself, rather than only in the one calling
handler — this protects every current and future caller of this low-level
setter, not just `HandlePetSetAction`. Checked `GetActionBarEntry` (the
read-side sibling) separately: all its callers iterate a server-controlled
loop variable, never client data, so it wasn't touched.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [DONE] Core/Pet - HandlePetAbandon missing ownership check

**Subsystem:** Handlers/PetHandler

**Problem:** `HandlePetAbandon` resolves a pet via
`ObjectAccessor::GetCreatureOrPetOrVehicle` — a map-scoped lookup that finds
*any* creature/pet by GUID on the caller's map, not just their own — and calls
`_player->RemovePet(...)` on it with no ownership check. Its sibling
`HandlePetRename`, right above it in the same file, explicitly checks
`pet->GetOwnerGUID() != _player->GetGUID()` before acting; `HandlePetAbandon`
was missing the equivalent check entirely. `Player::RemovePet` then asserts
that the (attacker's own) stable's current-pet number matches the target
pet's charm-info pet number — a mismatch (any other hunter's pet) aborts the
whole world server; in other states it could delete another player's pet from
the database.

**Files:** `src/server/game/Handlers/PetHandler.cpp`

**Reference:** Same gap flagged as the core recurring pattern this session
(shared object looked up by client-supplied ID, acted on without ownership
verification) — every other pet-action handler in this file already has the
equivalent check; this one alone lacked it.

**Fix:** Added `pet->GetOwnerGUID() == _player->GetGUID()` to the existing
condition, matching `HandlePetRename`'s style exactly.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [DONE] Core/Movement - Fall damage evasion via client-controlled fallTime

**Subsystem:** Entities/Player (movement/fall damage)

**Problem:** `Player::UpdateFallInformationIfNeed` re-baselines the fall-start
height (`m_lastFallZ`) whenever `m_lastFallTime >= minfo.jump.fallTime` — but
`jump.fallTime` is entirely client-supplied and this function runs on every
movement packet. A modified client falling from a great height can send
ordinary movement packets with a non-increasing `fallTime` throughout the
actual fall (Z genuinely decreasing), which keeps satisfying this condition
and re-baselines `m_lastFallZ` to follow the player down in real time. By the
time the real landing packet (`CMSG_MOVE_FALL_LAND`) arrives, the recorded
fall-start height is nearly identical to the landing height, producing
near-zero fall damage from any height.

**Files:** `src/server/game/Entities/Player/Player.cpp`

**Reference:** Identical code confirmed in DestinyCore — an inherited
TrinityCore-lineage weakness, not ArgusCore-introduced, but concretely
exploitable in both.

**Why the naive fix (just removing the fallTime-reset clause) would regress
gameplay:** that clause has a legitimate purpose — while walking normally
(not jumping/falling) on downward-sloping terrain, Z decreases gradually and
the client's `fallTime` stays near 0 (no active fall), so the baseline needs
to keep tracking the ground under the player's feet. Without it, walking
downhill would be misread as a fall in progress and could trigger fall
damage later on an unrelated real jump.

**Fix:** Restricted the fallTime-reset clause to only apply when the player
isn't currently flagged as falling (`!minfo.HasMovementFlag(MOVEMENTFLAG_FALLING)`).
This preserves the legitimate "track ground level while walking" behavior
(not falling → clause still fires) while closing the exploit (genuinely
falling with a fabricated fallTime → clause no longer fires, so the baseline
stays anchored to the true fall start until landing or an actual height gain).
The other two re-baseline conditions (height increased, landing packet) are
server-observed and were left untouched.

**Risk:** Moderate — this is a frequently-triggered, core mechanic (every
jump/fall for every player), so it needs real in-game verification across
varied terrain (stairs, slopes, elevators, knockbacks) before being trusted,
despite the change itself being narrowly scoped.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [ ] Core/Movement - No server-side speed/plausibility validation (structural, not fixed)

**Subsystem:** Handlers/MovementHandler, Entities/Player

**Problem:** `HandleMovementOpcode`/`Player::ValidateMovementInfo` validate
movement-flag *consistency* (can't root+move, can't fly without the aura) and
basic position validity (NaN/bounds), then unconditionally accept the
client-reported position. There is no comparison anywhere of position delta
over elapsed time against the unit's known max speed — a total absence of
server-side speed-hack detection, not a narrow bug.

**Files:** `src/server/game/Handlers/MovementHandler.cpp`,
`src/server/game/Entities/Player/Player.cpp` (`ValidateMovementInfo`)

**Reference:** Confirmed identical in DestinyCore (same logic, same design) —
this is a structural, whole-TrinityCore-lineage client-trust model, not an
ArgusCore regression.

**Status:** Deliberately NOT attempted as a quick fix. Building real
speed-hack detection (tracking per-unit expected max speed, elapsed-time
deltas, tolerating legitimate speed-changing effects/knockbacks/flight paths/
teleports without false-positiving) is a genuine subsystem design effort, not
a small isolated patch — it fails the "smallest safe change" test and risks
false-positive rubber-banding/kicks for legitimate players if rushed. Logging
this as a known limitation for a dedicated, deliberate design pass rather than
folding it into this backlog's fix-at-a-time cadence.

### [DONE] Core/BlackMarket - Auction minimum starting bid never enforced

**Subsystem:** BlackMarket/BlackMarketMgr

**Problem:** `BlackMarketEntry::ValidateBid` checked the bid against
`_currentBid + GetMinIncrement()`, but `GetMinIncrement()` (`(_currentBid/20) -
(...)`) evaluates to `0` while `_currentBid` is still `0` (no bids placed yet).
The auction template's actual starting price (`BlackMarketTemplate::MinBid`)
was only ever used for the client-facing display value
(`BlackMarketMgr::BuildItemsResponse`) — never consulted in bid validation. Any
player could bid as little as 1 copper on a fresh, unbid Black Market auction
(potentially a rare item intended to start at thousands of gold) and win it
outright if uncontested.

**Files:** `src/server/game/BlackMarket/BlackMarketMgr.cpp`

**Reference:** Identical gap confirmed in DestinyCore — inherited upstream
TrinityCore issue, not ArgusCore-introduced, but live and exploitable here.

**Fix:** Added a check against `GetTemplate()->MinBid` (null-checked, since
`GetTemplate()` can theoretically return null for an orphaned market entry) in
`ValidateBid`. Once an auction has at least one valid bid, `_currentBid` is
already at or above `MinBid`, so this only actually changes behavior for the
"no bids yet" case — no change to normal bidding-war behavior.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [DONE] Core/VoidStorage - Unlock cost never charged/verified

**Subsystem:** Handlers/VoidStorageHandler

**Problem:** `HandleVoidStorageUnlock` called
`_player->ModifyMoney(-int64(VOID_STORAGE_UNLOCK_COST))` with no preceding
`HasEnoughMoney` check, unlike the sibling deposit path in
`HandleVoidStorageTransfer`, which does check. `Player::ModifyMoney` clamps to
0 rather than failing when the deduction would go negative, and its return
value was ignored here — so a player with less than the unlock cost got void
storage unlocked for free (or for whatever gold they had).

**Files:** `src/server/game/Handlers/VoidStorageHandler.cpp`

**Reference:** Same gap exists in DestinyCore — inherited, not ArgusCore-specific.

**Fix:** Added the same `HasEnoughMoney` guard already used by the deposit
path, matching its style, before charging the unlock cost.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [DONE] Core/Petition - Missing owner check on three petition opcodes

**Subsystem:** Handlers/PetitionsHandler

**Problem:** Three petition (guild charter) opcode handlers looked up a
petition by client-supplied GUID and acted on it without verifying the caller
is the petition's owner, unlike `HandleTurnInPetition` which already does
(`if (_player->GetGUID() != petition->OwnerGuid) return;`):
- `HandlePetitionShowSignatures` — only checked the petition exists and the
  requester has no guild; any player could view any petition's signer list by
  GUID (minor info disclosure).
- `HandlePetitionRenameGuild` — only checked the caller physically possesses
  *an* item with that GUID (`GetItemByGuid`), not that they own the petition.
  Since a charter is legitimately handed to other players during the signing
  round-trip (trade/mail), any temporary holder could rename the
  soon-to-be-created guild out from under the actual owner.
- `HandleOfferPetition` — no check at all; any player could push the
  sign-petition UI for any petition onto any online target, regardless of
  their relationship to that petition.

**Files:** `src/server/game/Handlers/PetitionsHandler.cpp`

**Reference:** Same gaps exist in DestinyCore/upstream TrinityCore — not an
ArgusCore regression, but live and exploitable here. Verified that the
double-signature exploit, turn-in signature forgery, and petition fee are all
already correctly guarded (account-scoped signature check, server-collected
signature list, fixed config-driven cost) — only these three specific
handlers were missing the ownership check.

**Fix:** Added `if (_player->GetGUID() != petition->OwnerGuid) return;` to all
three handlers, matching `HandleTurnInPetition`'s existing style exactly.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [DONE] Core/Calendar - Missing owner/moderator authorization on event mutation opcodes

**Subsystem:** Handlers/CalendarHandler

**Problem:** Six calendar opcode handlers looked up an event or invite purely by
client-supplied ID and acted on it with no check that the caller owns or
moderates it, even though the file demonstrates the correct pattern elsewhere
(`HandleCalendarCopyEvent` already checks ownership). Event/invite IDs are
small, sequential, trivially-enumerable integers. Affected:
- `HandleCalendarUpdateEvent` — overwrote any event's type/flags/date/title/
  description with no ownership check.
- `HandleCalendarInvite` (existing-event branch) — added an invite to any event
  found by ID with no check the caller owns/moderates it.
- `HandleCalendarRsvp` — mutated any invite's RSVP status found by raw
  `InviteID`, not just the caller's own.
- `HandleCalendarEventRemoveInvite` — only blocked removing the event owner's
  own invite; any other invite on any event could be removed by any player.
- `HandleCalendarStatus` — set any invite's status (the "moderator changes an
  invitee's status" opcode) with no check the caller moderates that event.
- `HandleCalendarModeratorStatus` — granted/revoked moderator rank on any
  invite with no check at all.

**Exploit:** send `CMSG_CALENDAR_STATUS`/`CMSG_CALENDAR_MODERATOR_STATUS` with
guessed small `InviteID`s to flip other players' RSVP status or grant/revoke
moderator rank on events you have no relation to; send
`CMSG_CALENDAR_UPDATE_EVENT` with a guessed `EventID` to rewrite another
player's/guild's raid-schedule event.

**Files:** `src/server/game/Handlers/CalendarHandler.cpp`

**Reference:** Verified identical in DestinyCore — a pre-existing upstream
TrinityCore-class gap, not an ArgusCore regression, but still live and
exploitable here.

**Fix:** Added a shared `CalendarEventEditableBy(event, guid)` helper (true if
`guid` is the event owner, or an invitee holding `CALENDAR_RANK_MODERATOR`+) and
applied it to `HandleCalendarUpdateEvent`, the invite-to-existing-event path,
`HandleCalendarEventRemoveInvite`, and `HandleCalendarStatus`.
`HandleCalendarRsvp` gets a narrower check (`invite->GetInviteeGUID() == guid`
— self-RSVP only, since that opcode is for a player's own accept/decline, not
moderator action). `HandleCalendarModeratorStatus` gets a strict owner-only
check (deliberately *not* the shared helper) so a moderator can't promote
allies or demote other moderators/the owner.

While implementing, found and closed a follow-on gap in `HandleCalendarStatus`/
`HandleCalendarModeratorStatus`: both fetch the target invite via
`CalendarMgr::GetInvite(id)`, which searches *globally* across all events by
raw ID — so authorizing against the packet's `EventID` alone wasn't sufficient;
a moderator of event A could supply an `InviteID` belonging to event B. Added
`invite->GetEventId() == packet.EventID` checks alongside the ownership checks
in both handlers. (`HandleCalendarEventRemoveInvite` doesn't need this — its
underlying `CalendarMgr::RemoveInvite(inviteId, eventId, ...)` already scopes
the lookup to that specific event internally.)

**Risk:** Low-moderate — six call sites touched, but each change is additive
(an early-return guard) with no changes to the legitimate-request logic below
it. Worth in-game verification of normal calendar/raid-invite workflows before
trusting it fully.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [DONE] Core/LFG - HandleLfgLeaveOpcode lets a solo player spoof the leave target

**Subsystem:** Handlers/LFGHandler

**Problem:** `HandleLfgLeaveOpcode` (`CMSG_DF_LEAVE`) correctly restricts grouped
players to only leaving via their own group leader's ticket
(`group->GetLeaderGUID() == dfLeave.Ticket.RequesterGuid`), matching its own
"only leader can leave the queue" comment — but when the *caller* isn't
currently in a group, the code accepted `dfLeave.Ticket.RequesterGuid` as-is
with no check that it's the caller's own GUID: `if (!group || ...)`. Any solo
(non-grouped) player could pass an arbitrary victim's GUID and force that
victim out of their LFG queue/role-check, regardless of the victim's own group
state.

**Files:** `src/server/game/Handlers/LFGHandler.cpp`

**Reference:** Confirmed identical logic in DestinyCore — pre-existing upstream
gap, not ArgusCore-introduced.

**Fix:** Changed the condition so the solo-player branch requires
`GetPlayer()->GetGUID() == dfLeave.Ticket.RequesterGuid`, mirroring the existing
grouped-player check instead of skipping validation entirely. Single-line,
behavior for the grouped-leader case is completely unchanged.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [DONE] Core/Instance - One-packet server crash via CMSG_SET_SAVED_INSTANCE_EXTEND

**Subsystem:** Handlers/CalendarHandler, Instances/InstanceLockMgr

**Problem:** `WorldSession::HandleSetSavedInstanceExtend` (opcode
`CMSG_SET_SAVED_INSTANCE_EXTEND`) reads `MapID` (`int32`) and `DifficultyID`
(`uint32`) directly off the wire and passes them, essentially unvalidated, into
`sInstanceLockMgr.UpdateInstanceLockExtensionForPlayer(...)`, which constructs a
`MapDb2Entries(mapId, difficulty)`. That constructor is:
```cpp
MapDb2Entries::MapDb2Entries(uint32 mapId, Difficulty difficulty)
    : Map(sMapStore.AssertEntry(mapId)), MapDifficulty(ASSERT_NOTNULL(sDB2Manager.GetMapDifficultyData(mapId, difficulty)))
```
`AssertEntry`/`ASSERT_NOTNULL` fatally abort the entire process (confirmed:
`ASSERT` compiles to `Trinity::Assert` in all standard builds, same as the
earlier `Spell::SelectImplicitTargetDestTargets` finding). The only pre-existing
check in the handler (`_player->GetMapId() == uint32(setSavedInstanceExtend.MapID)`)
only rejects "same map as where you currently are" — any other MapID, including
a nonexistent one, or any MapID/DifficultyID combination with no
`MapDifficultyEntry` data, sails straight through.

**Exploit:** any logged-in player sends one `CMSG_SET_SAVED_INSTANCE_EXTEND`
packet with a garbage `MapID` (e.g. 999999) or a real map paired with a
difficulty it doesn't support. The assert fires and the entire world server
process aborts — every player on the server disconnected. No special
permissions, group, or instance state required.

**Files:** `src/server/game/Handlers/CalendarHandler.cpp`

**Reference:** The sibling difficulty-change handlers
(`HandleSetDungeonDifficultyOpcode`/`HandleSetRaidDifficultyOpcode` in
`MiscHandler.cpp`) correctly use the non-asserting `sDifficultyStore.LookupEntry()`
and validate before use — this handler skipped that pattern entirely, apparently
overlooked when the calendar-extend opcode was added. DestinyCore has no
equivalent opcode (no `InstanceLockMgr`/extension system), so no comparison
point there.

**Fix:** Added a validation guard using the same non-asserting lookups already
used elsewhere in the codebase (`sMapStore.LookupEntry`,
`sDB2Manager.GetMapDifficultyData`, both of which return `nullptr` instead of
aborting) — reject negative `MapID`, unknown maps, and unknown
map/difficulty combinations before constructing anything that could assert.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

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

### [DONE] Core/Spells - Client-triggerable ASSERT in SelectImplicitTargetDestTargets

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

**Confirmed:** `ASSERT` in this codebase compiles to `WPAssert`/`Trinity::Assert`
in all non-`PERFORMANCE_PROFILING` builds (`Errors.h:64-70`) — including the
project's standard RelWithDebInfo build — so it does abort a live server, not
just debug builds. Whether the null case is reachable with *current* spell_effect
data wasn't fully proven, but given ArgusCore is actively porting new Legion
spell scripts/effects from LegionCore (see `ROADMAP.md` Phase 12), a
misconfigured `spell_effect` row on any newly-ported spell is a realistic way to
hit this, and the very next line already handled the null case gracefully before
this fix — the assert was strictly more fragile than the code around it.

**Fix:** Removed the `ASSERT`; the null-target case now logs a `TC_LOG_ERROR`
(only when `DontFailSpellOnTargetingFailure` is absent, i.e. exactly the
condition that used to trigger the assert) and returns, skipping destination
target selection for that one effect instead of aborting the process. Behavior
for the normal case (object target present) is unchanged.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [DONE] Core/Spells - EffectUntrainTalents checks caster type instead of target type

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

**Fix:** Added `unitTarget->GetTypeId() != TYPEID_PLAYER` to the existing guard
condition (kept the original `m_caster` check as-is rather than replacing it, to
avoid removing any pre-existing intent behind it). `unitTarget->ToPlayer()` is now
guaranteed non-null when reached.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

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

### [DONE] Core/Arena - Rated arena rating never changes on match completion

**Subsystem:** Battlegrounds (Arena)

**Problem:** `Arena::EndBattleground` and `Arena::RemovePlayerAtLeave` both declared
`ArenaTeam* winnerArenaTeam = nullptr;` / `ArenaTeam* loserArenaTeam = nullptr;`
and never assigned them before the `if (winnerArenaTeam && loserArenaTeam && ...)`
guard around all the rating-adjustment logic — so the entire rating-calculation
block (which is otherwise complete and correct-looking: `WonAgainst`/
`LostAgainst`/`MemberWon`/`MemberLost`/`FinishGame`, achievement criteria, guild
news, postmatch rating for the results screen) was unreachable dead code. Winning
a rated arena match granted no rating; losing — including deliberately
disconnecting/leaving mid-match — lost none either, since `RemovePlayerAtLeave`'s
leave-penalty path had the identical bug.

**Files:** `src/server/game/Battlegrounds/Arena.h`,
`src/server/game/Battlegrounds/Arena.cpp`

**Exploit angle:** a player losing a rated match could leave with zero rating
loss, while winning conferred no gain either — rated arena had no functioning
progression or consequence at all.

**Reference:** DestinyCore's equivalent code is structurally different — it uses
per-match `Group` objects with rating attached directly (`GetBgRaid(team)`),
consistent with a redesigned/non-persistent-team model, not ArgusCore's
`ArenaTeam`/`ArenaTeamMgr` persistent-team architecture (which ArgusCore's own
queue-leave-penalty path in `BattleGroundHandler.cpp` and `BattlegroundQueue.cpp`
already correctly resolves via `sArenaTeamMgr->GetArenaTeamById(...)`). Ported the
fix, not the implementation: kept ArgusCore's own `ArenaTeam` architecture and
found a way to resolve it using infrastructure already present in this codebase.

**Fix:** Added `Arena::GetArenaTeamForTeam(Team team) const`, a small helper that
walks the match's tracked participants (via the already-existing, base-class
`Battleground::_GetPlayerForTeam`, which correctly handles online and
pending-offline-removal players) and resolves the persistent `ArenaTeam*` from
the first participant's registered `GetArenaTeamId(slot)` for the current arena
bracket (`ArenaTeam::GetSlotByType(GetArenaType())`). Used this helper to
correctly populate `winnerArenaTeam`/`loserArenaTeam` in both functions, matching
the exact team-mapping logic (including the `winner == TEAM_OTHER` draw case)
already established by the surrounding, previously-dead code. No new per-match
state or wiring through `BattlegroundMgr.cpp` was needed — this stayed a
2-file, additive change once the right existing accessor
(`_GetPlayerForTeam`) was found.

**Risk:** Moderate — this restores previously-nonfunctional behavior across a
real gameplay system (rated arena rating), so it needs live-match verification
before being trusted, but the diff itself is small and additive (no existing
logic removed or altered, only the two null pointers replaced with real lookups).

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — needs an actual rated
arena match (2v2/3v3/5v5) played to completion and a separate one abandoned
mid-match, on registered arena teams, with rating checked before/after both.

### [DONE] Core/Arena - Rated queue MMR hardcoded to 1 for all groups

**Subsystem:** Battlegrounds / Handlers/BattleGroundHandler

**Problem:** `HandleBattlemasterJoinArena` (`BattleGroundHandler.cpp:545-546`) has
`uint32 arenaRating = 1; //at->GetRating();` and
`uint32 matchmakerRating = 1; //at->GetAverageMMR(grp);` — the real lookups are
commented out, so every queued group's MMR is 1 regardless of actual team rating.
This feeds into `BattlegroundQueue`'s matchmaking rating window, making
matchmaking effectively rating-blind (any two teams can be paired regardless of
real skill).

**Files:** `src/server/game/Handlers/BattleGroundHandler.cpp`

**Severity:** Medium — a real Blizzlike-behavior bug (matchmaking should be
rating-aware), but not a security/corruption issue by itself. Interacts with the
rating fix above (now that rating actually changes, feeding it real MMR at queue
time becomes more valuable to fix).

**Fix:** Resolved `ArenaTeam* at` via `sArenaTeamMgr->GetArenaTeamById(_player->GetArenaTeamId(packet.TeamSizeIndex))`
— the exact same pattern already used elsewhere in this same file for the
queue-leave penalty path — and used `at->GetRating()`/`at->GetAverageMMR(grp)`
when a team is found. Deliberately kept the existing `= 1` fallback for the case
where no team resolves, rather than adding a new rejection/error path — minimal,
scoped purely to "use real rating when available"; whether a missing arena team
should block queuing at all is presumably already handled by the existing
`CanJoinBattlegroundQueue` call just below, which this change doesn't touch.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [ ] Core/Currency - Weekly cap unenforced for ComputedWeeklyMaximum currencies

**Subsystem:** Entities/Player (currency)

**Problem:** `Player::GetCurrencyWeeklyCap()` (`Player.cpp:7163-7167`) returns
`currency->MaxEarnablePerWeek` and leaves the
`CurrencyTypeFlags::ComputedWeeklyMaximum` case unimplemented (`// TODO`,
also marked `// NYI` in `DBCEnums.h:844`). `ModifyCurrency`'s weekly-cap
enforcement (`Player.cpp:~6953`) checks `if (weeklyCap && amount > 0 && ...)` —
for a currency relying purely on the computed flag (rather than a flat
`MaxEarnablePerWeek`), `weeklyCap` is `0`, so the whole cap-check block is
skipped even though `HasMaxEarnablePerWeek()` correctly tells the client a cap
exists. Any currency using this flag can be farmed past its intended weekly
ceiling by repeatable content that grants it.

**Files:** `src/server/game/Entities/Player/Player.cpp`

**Severity:** Medium. Requires knowing which Legion currencies (if any) actually
use `ComputedWeeklyMaximum` rather than a flat weekly cap to assess real impact —
not yet confirmed against `currencytypes` DB2 data for this build.

**Status:** Not started. Needs the actual computed-cap formula implemented (not
just a null-check bypass), so this is a small feature-completion fix rather than
a one-line guard — picking it up requires first identifying what the "computed"
formula should be (likely tied to character level or a related DB2 field).

### [DONE] Core/Currency - Negative amount not clamped when WeeklyQuantity exceeds cap

**Subsystem:** Entities/Player (currency)

**Problem:** In `ModifyCurrency` (`Player.cpp:6953-6954`), when clamping a gain to
the remaining weekly allowance, `amount = weeklyCap - itr->second.WeeklyQuantity`
can go negative if `WeeklyQuantity` is already above `weeklyCap` (reachable via
the `ComputedWeeklyMaximum` gap above, or a cap lowered by a content update),
silently converting what should be a currency *gain* into a currency *loss*
instead of clamping to 0.

**Files:** `src/server/game/Entities/Player/Player.cpp`

**Severity:** Low standalone — requires pre-existing corrupted/edge-case state to
trigger. Cheap defensive fix (`std::max(0, ...)`) whenever picked up, ideally
alongside the finding above.

**Note found during fix:** both `weeklyCap` and `WeeklyQuantity` are `uint32`, so
the original `weeklyCap - itr->second.WeeklyQuantity` wasn't just "a small
negative number" when `WeeklyQuantity > weeklyCap` — it was an unsigned
underflow that wraps to a huge value before converting to the `int32 amount`,
producing an arbitrary large negative (in practice), not a bounded-small one.

**Fix:** Compare in unsigned space first (`weeklyCap > itr->second.WeeklyQuantity`)
and only compute the subtraction when it's safe (non-negative), otherwise clamp
to `0`. An `amount` of `0` is already handled safely by the existing
`if (!amount) return;` a few lines below — no gain, no loss, no crash.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [DONE] Core/Battleground - Unguarded PlayerScores lookup in EndBattleground

**Subsystem:** Battlegrounds (Battleground)

**Problem:** `Battleground::EndBattleground` (~line 702-717, only reached when
`isBattleground() && CONFIG_BATTLEGROUND_STORE_STATISTICS_ENABLE`) does
`BattlegroundScoreMap::const_iterator score = PlayerScores.find(player->GetGUID());`
then dereferences `score->second` directly with no `end()` check. `m_Players` and
`PlayerScores` are currently kept 1:1 by `AddPlayer`/`RemovePlayerAtLeave`, so
this isn't trivially reachable today, but the sibling function
`UpdatePlayerScore()` explicitly guards the identical lookup a few lines below
(`if (itr == PlayerScores.end()) return false;`), showing the codebase itself
treats this as an unsafe assumption elsewhere. Worth hardening defensively so a
future desync (modified BG script, offline-queue race) can't crash the world
server for the whole battleground's population.

**Files:** `src/server/game/Battlegrounds/Battleground.cpp`

**Fix:** Wrapped the PvP-stats DB insert in `if (score != PlayerScores.end())`,
matching the guard `UpdatePlayerScore()` already uses for the identical lookup.
No change to the normal (found) path.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

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

### [DONE] Mage/Frost - Chain Reaction used modern retail spell ID/mechanic shape

**Subsystem:** Scripts/Spells (spell_mage)

**Problem:** Follow-up to the Brain Freeze fix above. `spell_mage_ice_lance`
referenced `SPELL_MAGE_CHAIN_REACTION = 278310`, explicitly cast from Ice
Lance's own script when a separate "dummy" marker aura (278309) was present.
Confirmed `278310` is the **modern retail** spell ID for this talent
(verified byte-for-byte match against TrinityCore-master's own `MageSpells`
enum and cast logic) — not Legion 7.3.5's. DestinyCore (also Legion 7.3.5)
uses a completely different spell ID (`195419`) *and* a structurally
different mechanic: a self-stacking passive aura whose own DB2 effects apply
the Ice Lance damage bonus per Frostbolt-built stack, with the script's only
job being a proc-eligibility filter — not something Ice Lance grants/casts at
all. This is a genuine mechanic redesign between Legion and later expansions
sharing the same talent name, not just an ID typo.

**Files:** `src/server/scripts/Spells/spell_mage.cpp`

**Reference:** DestinyCore (Legion 7.3.5) for the correct ID and mechanic
shape; TrinityCore-master for confirming `278310`'s origin as modern retail.

**Fix:** Changed `SPELL_MAGE_CHAIN_REACTION` to `195419`; removed the
now-obsolete `SPELL_MAGE_CHAIN_REACTION_DUMMY` (278309) and the explicit
cast-from-Ice-Lance code entirely; added a new, self-contained
`spell_mage_chain_reaction` `AuraScript` (registered on 195419 itself) whose
only job is filtering which spells count as "Frostbolt" for stack-building,
matching DestinyCore's structure. Added `SPELL_MAGE_FROSTBOLT`(116)/
`SPELL_MAGE_FROSTBOLT_TRIGGER`(228597) constants for that filter, matching
the same main-spell/damage-trigger-spell pattern already used for Ice Lance
in this file.

**Risk:** Bounded/asymmetric — if my inference about spell 195419 having
self-sufficient DB2 stacking/damage-bonus data is wrong, the worst case is
Chain Reaction remains as non-functional as it already was (casting 278310,
which likely isn't even a valid Legion spell, was already producing no
effect). It cannot become *more* broken than its prior state. If correct, it
starts working for the first time.

**Database dependency (found retroactively, see the SQL binding entry near
the end of this file):** `spell_mage_chain_reaction` is a brand-new class
name with no `spell_script_names` row anywhere, so it was silently inert
until a binding was added in
`sql/pending/world/2026_07_25_00_world.sql`.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — this one specifically
needs someone to confirm Chain Reaction actually does something in-game
(stacking buff visible, Ice Lance damage scaling with stacks), since I
could not verify spell 195419's DB2 data directly.

### [DONE] Mage/Frost - Brain Freeze mechanic entirely unimplemented

**Subsystem:** Scripts/Spells (spell_mage)

**Problem:** Brain Freeze (190447/190446) — Frost Mage's analogous proc
mechanic to Fire's Hot Streak (Frostbolt has a chance to make the next Ice
Lance guaranteed-crit and free/instant) — had no implementation anywhere in
ArgusCore.

**Files:** `src/server/scripts/Spells/spell_mage.cpp`

**Reference:** DestinyCore's implementation exists but is spread across
Frostbolt's `OnHit` with hardcoded spell-ID checks and several entangled
talent modifiers (Clarity of Thought, Improved Brain Freeze/Flurry
interaction, Frozen Touch). ArgusCore's own file uses a more modern, more
idiomatic pattern for the closely-related Fingers of Frost mechanic
(`spell_mage_fingers_of_frost`, immediately below this fix): a self-contained
`AuraScript` on the talent aura itself, using generic `SpellFamily` classmask
matching (`DoCheckEffectProc`/`AfterEffectProc`) rather than hardcoded spell-ID
checks inside the triggering spell's own script. Modeled Brain Freeze directly
on that existing, already-verified-working sibling pattern instead of
DestinyCore's structure — reusing the exact same Frostbolt classmask bit
Fingers of Frost already relies on (`flag128(0, 0x2000000, 0, 0)`).

**Deliberately deferred (not implemented):** Clarity of Thought (extra proc
chance), Improved Brain Freeze's Flurry interaction, and Frozen Touch's
proc-chance modifier — all real DestinyCore-referenced talents I could not
independently verify are correct for Legion 7.3.5 vs. a later expansion's
version, consistent with the same conservative approach used for Hot
Streak/Combustion.

**Side finding, not fixed:** while integrating with the existing
`spell_mage_ice_lance` class, noticed it has no baseline "always crit against
frozen targets" mechanic either (Ice Lance's well-known innate behavior across
many expansions, independent of any talent) — the existing `HandleOnHit` only
uses the target's frozen state to gate Thermal Void/Chain Reaction bonuses,
never crit chance itself. Didn't investigate or touch this; flagging as a
separate, more foundational question about baseline Ice Lance correctness.

**Also side finding:** `spell_mage_ice_lance` uses
`SPELL_MAGE_CHAIN_REACTION = 278310` (a spell ID that also appeared in the
earlier "broader observation" of post-Legion content in this file), while
DestinyCore's independent Legion 7.3.5 implementation uses `195419` for the
same-named talent. Not resolved here — did not want to conflate a possible ID
correction with the Brain Freeze fix; flagging as its own open question.

**Fix:** Added `spell_mage_brain_freeze` (proc generation, mirrors
`spell_mage_fingers_of_frost`'s structure exactly) and extended the existing
`spell_mage_ice_lance` class with an `OnCalcCritChance` handler (guaranteed
crit while Brain Freeze is active, independent of the frozen-target check used
by other Ice Lance bonuses) and an `AfterCast` handler that consumes Brain
Freeze once per cast — after all targets have already been evaluated for crit,
so a piercing/multi-target Ice Lance doesn't lose the guarantee partway
through its own resolution.

**Risk:** Moderate-high — the most structurally complex of the mechanics
implemented this session (touches an existing, already-functioning script for
a different purpose). Needs careful in-game testing, particularly that Thermal
Void/Chain Reaction/Fingers of Frost interactions with Ice Lance are unaffected.

**Database dependency (found retroactively, see the SQL binding entry near
the end of this file):** `spell_mage_brain_freeze` is a brand-new class name
with no `spell_script_names` row anywhere, so it was silently inert until a
binding was added in `sql/pending/world/2026_07_25_00_world.sql`. (The
extension to the existing `spell_mage_ice_lance` class is unaffected — that
class's own binding was untouched.)

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [DONE] Mage/Fire - Combustion (core damage cooldown) entirely unimplemented

**Subsystem:** Scripts/Spells (spell_mage)

**Problem:** Combustion (190319), Fire Mage's primary damage cooldown, had no
implementation anywhere in ArgusCore — more consequential than the Hot Streak
gap since it's a cooldown used in every burst/rotation window, not a
proc-dependent mechanic.

**Files:** `src/server/scripts/Spells/spell_mage.cpp`

**Reference:** DestinyCore has a Combustion implementation, but written in an
older `SpellScriptLoader`-style pattern inconsistent with the rest of that
file, using a legacy raw-field accessor for crit rating, and referencing an
unexplained `SPELL_INFERNO` removal on aura expiry I couldn't identify or
verify. Discussed with the user rather than guessing at the uncertain parts.

**Decision (explicit user choice):** implement only the well-documented, high-
confidence core mechanic — Combustion doubles the caster's current spell crit
rating for its duration (a distinctive, well-known feature of Legion's
specific version of Combustion, separate from later expansions' versions).
Deliberately did NOT implement two uncertain pieces:
1. The "duration refreshes by 1 sec per crit while active" behavior some
   Legion documentation describes — not confident enough in the exact
   mechanic/values to implement without guessing.
2. DestinyCore's `SPELL_INFERNO` removal-on-expiry line — unidentified spell,
   unclear purpose, not ported.

Both are called out in a code comment at the implementation site and should be
revisited if/when they can be verified (e.g. against real Legion 7.3.5 spell
tooltip/DB2 data).

**Fix:** Added `spell_mage_combustion` (`AuraScript`, `DoEffectCalcAmount` on
`EFFECT_1`/`SPELL_AURA_MOD_RATING`), computing the bonus from
`GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_CRIT_SPELL)` — confirmed
this is still the correct modern accessor in ArgusCore's own engine (already
used identically elsewhere in this codebase), unlike DestinyCore's outdated
equivalent. Set `canBeRecalculated = false` (deviating from DestinyCore's
unset default) to avoid a plausible feedback-loop bug: recalculating from the
same combat-rating field that Combustion's own bonus contributes to could
compound the bonus on every stat recalculation. This matches the existing
`canBeRecalculated = false` convention already used by sibling
`DoEffectCalcAmount` handlers in this same file.

**Risk:** Moderate — a new, partial implementation of a frequently-used
cooldown; needs in-game verification that the crit boost applies/expires
correctly, and the team should be aware the refresh-on-crit behavior is
intentionally absent pending further verification.

**Database dependency (found retroactively, see the SQL binding entry near
the end of this file):** `spell_mage_combustion` is a brand-new class name
with no `spell_script_names` row anywhere, so it was silently inert until a
binding was added in `sql/pending/world/2026_07_25_00_world.sql`.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [DONE] Mage/Fire - Scorch implemented post-Legion "Frenetic Speed" execute mechanic

**Subsystem:** Scripts/Spells (spell_mage)

**Problem:** `spell_mage_scorch` gave Scorch a guaranteed crit against targets
below a health threshold and cast `SPELL_MAGE_FRENETIC_SPEED` (236060, a
movement-speed buff) on the caster whenever that execute condition landed.
Cross-checked against both Legion 7.3.5 reference cores (DestinyCore and
LegionCore-7.3.5-merged) — **neither implements any custom Scorch script at
all**, meaning Legion's actual Scorch is a plain filler spell needing no
special server-side behavior. Checked TrinityCore-master (current retail) and
found this exact mechanic (execute-crit + Frenetic Speed, later versions also
reference `SPELL_MAGE_HEAT_SHIMMER` and other clearly-modern spell IDs like
`SPELL_MAGE_MOLTEN_FURY = 458910`) — confirming `spell_mage_scorch` was copied
from a modern retail TrinityCore baseline and never adapted for Legion. This
is the first confirmed instance of the "broader observation" flagged in the
Hot Streak fix above (non-Legion content present in this file).

**Files:** `src/server/scripts/Spells/spell_mage.cpp`

**Reference:** DestinyCore, LegionCore-7.3.5-merged (both show zero custom
Scorch behavior — the correct Legion baseline); TrinityCore-master (source of
the incorrect, anachronistic mechanic being removed).

**Fix:** Removed the `spell_mage_scorch` class, its registration, and the
now-unused `SPELL_MAGE_FRENETIC_SPEED` enum entry. Left `SPELL_MAGE_SCORCH`
itself in place — still needed by the Hot Streak driver fix above, which
correctly treats Scorch as one of the spells that can trigger Heating Up.

**Risk:** Low — pure removal of code that shouldn't have been running for
this expansion; worst case if I'm wrong about Legion's Scorch being crit-plain
is that Scorch reverts to guaranteed-normal-hit-chance-only behavior, which is
easy to spot and revert.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [DONE] Mage/Fire - Hot Streak mechanic entirely unimplemented

**Subsystem:** Scripts/Spells (spell_mage)

**Problem:** Hot Streak (spell 48108) and Heating Up (48107) — the core Fire
Mage "2 consecutive crits = free instant Pyroblast/Flamestrike" mechanic — had
zero implementation anywhere in ArgusCore: no script class, no reference to
either spell ID in any script or SQL file. Confirmed via `logs/DestinyCore`
(also Legion 7.3.5) that the mechanic works there via a dedicated passive
"driver" aura (44448, "Pyroblast!") that Fire Mage characters carry
innately: `DoCheckProc` gates which Fire spells (Scorch/Fireball/Fire Blast/
Flamestrike/Pyroblast/Phoenix Flames/Dragon's Breath-with-Alexstrasza's-Fury)
can trigger it, and `OnEffectProc` on its `SPELL_AURA_DUMMY` effect escalates
Heating Up -> Hot Streak on a second consecutive crit, or resets on a non-crit.
ArgusCore's own file already uses this exact idiom successfully for Ignite
(`spell_mage_ignite`, a similarly-innate Fire Mage passive), which gave
confidence the same technique (passive dummy-aura proc driver) works correctly
in this codebase/data.

**Files:** `src/server/scripts/Spells/spell_mage.cpp`

**Reference:** DestinyCore's implementation ported for logic; deliberately
omitted one DestinyCore modifier ("Controlled Burn" — a chance for a single
crit to skip straight to Hot Streak) since I could not verify it's genuine
Legion 7.3.5 content rather than a later-expansion mechanic bolted onto
DestinyCore's own codebase — kept the fix to the unambiguous, universally-
documented core 2-crit mechanic only.

**Broader observation (not fixed, flagged for a separate pass):** while
investigating, several *other* existing spell classes in this same file
reference spell IDs/mechanics that postdate Legion 7.3.5 (e.g. Frenetic Speed
236060 and Radiant Spark, both Shadowlands/Dragonflight-era; Winters Chill,
Chain Reaction, Feel the Burn, Ethereal Blink, Master of Time). This suggests
`spell_mage.cpp` was substantially sourced from a modern TrinityCore-master
baseline rather than adapted for Legion, independent of the Hot Streak gap.
Whether these are genuinely harmful (actively firing wrong-expansion behavior)
or inert (present but never triggered because the underlying spell/talent
doesn't exist in Legion's DB2 data) wasn't determined — that requires either
in-game testing per mechanic or DB2 data inspection I don't have tooling for
here. Not attempted as part of this fix; flagging as a candidate for a
dedicated "verify spell_mage.cpp against Legion 7.3.5 talent trees" pass.

**Fix:** Added the `SPELL_MAGE_PYROBLAST_CLEARCASTING_DRIVER` (44448) driver
aura script (`spell_mage_pyroblast_clearcasting_driver`), matching the file's
existing conventions (same idiom as `spell_mage_ignite`), plus the supporting
spell ID constants that weren't already defined (`SPELL_MAGE_FIREBALL`,
`SPELL_MAGE_FLAMESTRIKE`, `SPELL_MAGE_PYROBLAST`, `SPELL_MAGE_PHOENIX_FLAMES`,
`SPELL_MAGE_DRAGONS_BREATH`, `SPELL_MAGE_ALEXSTRASZAS_FURY`,
`SPELL_MAGE_HEATING_UP`, `SPELL_MAGE_HOT_STREAK`, `SPELL_MAGE_SCORCH`).

**Consumption side (caught before shipping, not a separate fix):** the
initial pass only implemented the generation half (Heating Up -> Hot Streak).
Re-checked DestinyCore and found dedicated `OnEffectHit` handlers on Pyroblast
and Flamestrike that remove Hot Streak once it's actually consumed — without
this, Hot Streak would never be cleared and would grant unlimited free
instant Pyroblasts/Flamestrikes instead of the one cast it earned. Added
`spell_mage_pyroblast` and `spell_mage_flamestrike` (both `SpellScript`,
`OnEffectHit` on `EFFECT_0`/`SPELL_EFFECT_SCHOOL_DAMAGE`) to close this.
Omitted DestinyCore's "Pyromaniac" re-proc-on-cast modifier for the same
reason as "Controlled Burn" above — could not confirm it matches Legion
7.3.5's actual talent design rather than a later expansion's version.

**Database dependency (found retroactively, see the SQL binding entry near
the end of this file):** `spell_mage_pyroblast_clearcasting_driver` is a
brand-new class name with no `spell_script_names` row anywhere, so it was
silently inert until a binding was added in
`sql/pending/world/2026_07_25_00_world.sql`. (`spell_mage_pyroblast` and
`spell_mage_flamestrike` keep their original names/bindings — only the new
driver class needed one.)

**Risk:** Moderate — this is new gameplay behavior (not a bug fix to existing
code), so it needs real in-game verification that Fire Mages actually receive
the driver aura and see Heating Up/Hot Streak procs, not just a compile check.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [DONE] Warrior/Arms - Sudden Death missing entirely

**Subsystem:** Scripts/Spells (spell_warrior)

**Problem:** Sudden Death (52437), an Arms proc that resets Colossus Smash's
cooldown when it triggers, had no implementation in ArgusCore at all.

**Reference:** DestinyCore's implementation, revisited properly this time.
Its own author left a "// correct?" doubt-comment about pairing the
`AfterEffectApply` hook with an `AuraEffectRemoveFn`-named macro — checked
ArgusCore's own `SpellScript.h` and found `AuraEffectApplyFn` and
`AuraEffectRemoveFn` are literally the same macro
(`#define ..Fn(F, I, N, M) EffectApplyHandler(&F, I, N, M)` for both), so the
doubt was unfounded — the original code was already correct, just using a
confusingly-named (but functionally identical) macro. This is a good example
of why DestinyCore's Warrior file deserved the "primary reference" treatment
after all, rather than the premature dismissal earlier in this session.

**Files:** `src/server/scripts/Spells/spell_warrior.cpp`

**Fix:** Added `SPELL_WARRIOR_SUDDEN_DEATH = 52437` and a
`spell_warr_sudden_death` `AuraScript` that resets Colossus Smash's cooldown
on apply, ported directly from DestinyCore's logic (using the clearer
`AuraEffectApplyFn` macro name instead of the confusing-but-equivalent
`AuraEffectRemoveFn` DestinyCore used).

**Database dependency (found after implementing, applies to this and the
Rampage fix below):** `RegisterSpellScript` only registers a script under its
stringified class name; the actual spell-id-to-script binding happens via the
`spell_script_names` SQL table. Traced ArgusCore's own already-committed
migration history and found the binding for `spell_warr_sudden_death` was
deleted at some point with no replacement (and Rampage's equivalent binding,
under a different old name, was added then deleted again) — meaning neither
fix takes effect without a fresh SQL row. Added both to
`sql/pending/world/2026_07_25_00_world.sql` for review before release.

**Risk:** Low — small, self-contained, matches an already-verified-working
reference exactly. (The SQL dependency above is the only reason this doesn't
take effect purely from the C++ change.)

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification.

### [DONE] Warrior/Fury - Rampage missing entirely (Whirlwind's cleave buff was never consumed)

**Subsystem:** Scripts/Spells (spell_warrior)

**Investigation history (kept for context on how this was reached):**
initially compared registered-script counts and found ArgusCore's
`spell_warrior.cpp` (32 scripts) far short of DestinyCore's (~90+, mostly via
an older `SpellScriptLoader` pattern a naive grep missed at first), and framed
this as a huge gap. Then compared against `logs/LegionCore-7.3.5-merged` (a
smaller, ~1180-line file, closer to ArgusCore's own ~1280) and, seeing several
DestinyCore pieces reference unidentified hardcoded spell IDs, prematurely
concluded DestinyCore's larger file had drifted toward later-expansion
content the way its Mage file had at the edges (Scorch/Chain Reaction).

**That conclusion was wrong, and worth recording why:** "I can't identify
this spell ID" and "this is confirmed wrong-expansion content" are different
confidence levels, and they got conflated. Checked one of the "unidentified"
pieces (spell 248120, "Valarjar Berserkers") against TrinityCore-master and
found **no match at all** — the opposite of what was found for Scorch. The
Valarjar are Odyn's warriors from Legion's Warrior Order Hall (Skyhold)
storyline specifically; that's not drift, it's positive evidence of genuine,
unique Legion content that simply doesn't exist in any later expansion. The
file-size argument was also weaker than treated: LegionCore is an admittedly
rough 2020 community leak per its own README, so it being smaller plausibly
means *incomplete*, not that DestinyCore padded itself with wrong content.

**What was actually found once DestinyCore was properly treated as primary
reference:** `SPELL_WARRIOR_MEAT_CLEAVER_PROC` (85739) in DestinyCore is the
exact same spell ID as ArgusCore's own, already-implemented
`SPELL_WARRIOR_WHIRLWIND_CLEAVE_AURA` — same spell, two different names.
ArgusCore already correctly grants this stacking cleave buff from Whirlwind
(Fury-specific, via `spell_improved_whirlwind`/`ApplyWhirlwindCleaveAura`),
and already correctly generates Enrage from Rampage/Bloodthirst/Bloodbath
crits (`spell_warr_enrage_proc`). What was missing was Rampage itself:
**no spell script or spell ID constant for Rampage existed at all**, so the
cleave buff Whirlwind grants was never consumed by anything — Fury's core
spender ability was entirely absent.

**Files:** `src/server/scripts/Spells/spell_warrior.cpp`

**Fix:** Added `SPELL_WARRIOR_RAMPAGE = 184367` and a new
`spell_warr_rampage` script that consumes the *existing*
`SPELL_WARRIOR_WHIRLWIND_CLEAVE_AURA` on cast (ported DestinyCore's logic,
adapted to reference ArgusCore's own existing constant rather than
introducing a duplicate parallel system) and halves damage to non-primary
targets hit by the cleave (matching DestinyCore's `HandleDamage` exactly).
Deliberately did **not** touch Enrage generation — that's already correctly
handled elsewhere in this file and re-implementing it here risked duplicate/
redundant Enrage application on every Rampage cast.

**Database dependency:** traced ArgusCore's own already-committed SQL
migration history and found spell 184367 had a `spell_script_names` binding
added under the name `spell_warr_rampage_enrage` (presumably from earlier,
never-finished work) and then deleted again with no replacement — so this
fix needs a fresh binding to take effect. Added to
`sql/pending/world/2026_07_25_00_world.sql` (same file as the Sudden Death
fix above) rather than reusing the old dangling name, since that name implied
Enrage-application specifically, which this script doesn't handle.

**Residual uncertainty flagged, not resolved:** `184367` is DestinyCore's
spell ID for Rampage; a comment in LegionCore's Rampage class header lists
different-looking IDs (218617, 184707, 184709, 201364, 201363) for
"Rampage," though the actual LegionCore code never uses any of them directly
(likely rank/hit-variant IDs bound via SQL to the same script, not something
the C++ logic itself needs to branch on). Proceeded with reasonable
confidence in 184367 given it matches general knowledge of this spell's
historical ID stability, but this specific ID was not independently verified
against Legion 7.3.5 client data.

**Risk:** Moderate — new gameplay behavior for Fury's core spender, and the
one open uncertainty (spell ID) would make the whole fix inert (not
crash-prone, just a no-op) if wrong, similar to the pre-fix Chain Reaction
situation.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — see chat for detailed
steps (Whirlwind should grant a stacking cleave buff as a Fury Warrior;
Rampage should consume it and cleave to nearby enemies at half damage on
secondary targets; if Rampage produces no effect at all, the spell ID is
likely the unresolved uncertainty above, not a logic bug).

**Also still open, correctly re-framed:** Sudden Death and a handful of other
DestinyCore Warrior pieces remain unimplemented — not because they're
suspected wrong-expansion content (that assumption was the error above), but
because their specific mechanics weren't yet independently confirmed with
enough confidence in this pass. Worth revisiting with the same "DestinyCore
as primary reference, verify individual pieces against TrinityCore-master
only when something looks actually suspicious" approach that worked here,
rather than the file-size heuristic that led to the wrong conclusion
initially.

### [DONE] Warrior/Arms+Fury - Bladestorm entirely missing

**Subsystem:** Scripts/Spells (spell_warrior)

**Problem:** Bladestorm (227847), the shared Arms/Fury cooldown that spins
the Warrior through nearby enemies dealing periodic weapon damage, had no
implementation in ArgusCore at all — no spell script, no supporting spell ID
constants beyond the base spell and an already-present, already-referenced
periodic-tick spell (`SPELL_WARRIOR_BLADESTORM_PERIODIC_WHIRLWIND` = 50622,
used elsewhere in this file, which is what gave confidence the surrounding
mechanic was legitimate rather than DestinyCore drift).

**Reference:** DestinyCore's three-class implementation
(`spell_warr_bladestorm` on 227847, `spell_warr_bladestorm_new` on 222634,
`spell_warr_bladestorm_offhand` on 95738). The "New Bladestorm" naming and
the fact that the base spell's own hit effects need to be suppressed in
favor of a separate periodic aura reflects a mid-expansion Legion rework of
the ability, not old or forward-drifted content — cross-verified by the
periodic driver spell (50622) already being an independently-used,
named constant in ArgusCore's own file.

**Files:** `src/server/scripts/Spells/spell_warrior.cpp`

**Fix:** Added `SPELL_WARRIOR_BLADESTORM_NEW = 222634` and
`SPELL_WARRIOR_BLADESTORM_OFFHAND = 95738`, and three new classes ported from
DestinyCore, adapted to ArgusCore's modern API where it differs:
- `spell_warr_bladestorm` (`SpellScript` on 227847): suppresses the base
  spell's own aura/damage/heal/default effects on all three hit effects, then
  casts 222634 instead — the base spell becomes a pure trigger.
- `spell_warr_bladestorm_new` (`AuraScript` on 222634): periodic dummy aura
  that casts the existing, already-verified `SPELL_WARRIOR_BLADESTORM_PERIODIC_WHIRLWIND`
  (50622) on each tick — no new periodic-damage system introduced, reuses
  what's already there.
- `spell_warr_bladestorm_offhand` (`SpellScript` on 95738): suppresses
  offhand damage/effects unless the caster is a Fury Warrior, since only
  Fury dual-wields. Used ArgusCore's modern
  `GetPrimarySpecialization() == ChrSpecialization::WarriorFury` check
  (already used elsewhere in this same file) instead of DestinyCore's legacy
  `GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID)` pattern.

**Database dependency:** searched ArgusCore's committed SQL
(`sql/updates/world/master/*.sql`, `sql/base/dev/world_database.sql`) for
existing `spell_script_names` bindings on 227847, 222634, or 95738 — found
none. Same gap as Rampage/Sudden Death: without a fresh binding, none of the
three new scripts would ever fire. Added to
`sql/pending/world/2026_07_25_00_world.sql` (same shared pending file).

**Risk:** Moderate — three interacting scripts rather than one, and it's new
gameplay behavior for a shared Arms/Fury cooldown, so it needs real in-game
verification of all three pieces together (base-spell suppression, periodic
tick damage, and the Fury-only offhand gate), not just a compile check.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — see chat for detailed
steps (cast Bladestorm as both Arms and Fury; confirm the spin/periodic
damage tick fires instead of a single instant hit; confirm only Fury deals
offhand damage during the channel; confirm the base spell produces no direct
aura/damage/heal of its own).

### [DONE] Mage - four earlier scripts found missing spell_script_names bindings retroactively

**Subsystem:** Scripts/Spells (spell_mage), SQL

**Problem:** The `spell_script_names` binding requirement (a `RegisterSpellScript`
call alone does not bind a script to a spell id — a separate SQL row is
required) was only discovered partway through the Warrior work, *after* four
Mage scripts had already been implemented and committed. Never went back to
check them until directly asked "do the mage spells not need SQL files
too?" — checked, and confirmed they do. All four are brand-new class names
introduced this session (never existed under any name before), so none of
them could have an existing binding anywhere (repo SQL or the external TDB):

- `spell_mage_pyroblast_clearcasting_driver` (44448) — Hot Streak generation
- `spell_mage_combustion` (190319) — Combustion crit-rating double
- `spell_mage_brain_freeze` (190447) — Brain Freeze proc generation
- `spell_mage_chain_reaction` (195419) — Chain Reaction stacking

All four have been silently inert since being written. By contrast, the
classes that were only *extended* in the same commits
(`spell_mage_pyroblast`, `spell_mage_flamestrike`, `spell_mage_ice_lance`)
kept their original names, so whatever binding already made them fire is
unaffected.

**Files:** `sql/pending/world/2026_07_25_00_world.sql`

**Fix:** Added `spell_script_names` bindings for all four spell ids to the
same shared pending file used for the Warrior bindings above (per your
preference to keep appending to one file). Each affected Mage entry earlier
in this file now has a "Database dependency" note cross-referencing this one.

**Risk:** Low — pure additive SQL, same idempotent DELETE-then-INSERT pattern
as the Warrior bindings; the actual gameplay risk was already assessed in
each mechanic's own entry above and is unchanged by this fix.

**Commit:** `<pending>`

**Test:** Same as each individual mechanic's own test notes above — none of
Hot Streak, Combustion, Brain Freeze, or Chain Reaction could have shown any
in-game effect before this binding was added, so any earlier "it didn't seem
to do anything" observation is explained by this, not a logic bug in the
scripts themselves.

---

## P4 — Performance / Cleanup

(Not populated by this pass.)

---

## Notes on scope

This backlog is deliberately shallow on first pass — it reflects a series of
targeted, read-only audits rather than an exhaustive line-by-line review. Areas
checked and found solidly guarded (not re-audited unless something changes
nearby): `TradeHandler`, `MailHandler` (including mail money/COD overflow
guards), `AuctionHouseHandler`, `GuildHandler`/`Guild.cpp` bank-item moves,
`VoidStorageHandler`, movement/teleport coordinate validation
(`MovementHandler.cpp`, `ValidateMovementInfo`), `Spell.cpp`/`SpellEffects.cpp`
target-pointer handling, `LootHandler.cpp` target/master-loot lookups,
`Group.cpp` aura/member iteration, Group leader/assistant action gating
(`GroupHandler.cpp`), master-loot distribution, guild bank money
withdrawal/tab-rights, guild rank creation/deletion/editing, and — as of this
pass — the full quest turn-in/reward pipeline (`QuestHandler.cpp`,
`Player::RewardQuest`/`CanRewardQuest`/`SatisfyQuestStatus`): reward-choice
index bounds are checked against the fixed `QUEST_REWARD_CHOICES_COUNT` array
size, all reward amounts (items, gold, XP, currency) are read exclusively from
server-side `Quest` template data with no client-controlled quantities, and
quest state transitions (`m_QuestStatus` removal, `SetRewardedQuest`) happen
synchronously in memory before any reward is granted, closing off any
double-turn-in race — a second turn-in packet always sees already-updated state.

Also checked and found solidly guarded: instance ID/save binding (always
server-computed from the player's/group's own lock record, never client-supplied
— `InstanceLockMgr::CanJoinInstanceLock`/`FindActiveInstanceLock`), instance
reset opcodes (leader-only, rejects LFG groups and non-empty/active instances),
boss-kill/loot-eligibility tracking across repeated zone-in/out
(`CompletedEncountersMask` comparison forces a confirm-or-leave flow rather than
silently granting fresh eligibility), and all `ModifyCurrency`/`AddCurrency`
call sites except the two open findings above (every other caller passes a
server-computed amount from loot/quest/spell/item data, never a raw packet
value). No Mythic Keystone/Challenge Mode system exists in this codebase (flat
Mythic difficulty only), so M+-specific exploit classes don't apply here.
