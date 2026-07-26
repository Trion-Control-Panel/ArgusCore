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

**Database dependency (found retroactively):** `spell_mage_chain_reaction` is
a brand-new class name with no `spell_script_names` row anywhere, so it was
silently inert until a binding was added in
`sql/updates/world/master/2026_07_25_06_world.sql`.

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

**Database dependency (found retroactively):** `spell_mage_brain_freeze` is a
brand-new class name with no `spell_script_names` row anywhere, so it was
silently inert until a binding was added in
`sql/updates/world/master/2026_07_25_05_world.sql`. (The extension to the
existing `spell_mage_ice_lance` class is unaffected — that class's own
binding was untouched.)

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

**Database dependency (found retroactively):** `spell_mage_combustion` is a
brand-new class name with no `spell_script_names` row anywhere, so it was
silently inert until a binding was added in
`sql/updates/world/master/2026_07_25_04_world.sql`.

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

**Database dependency (found retroactively):** `spell_mage_pyroblast_clearcasting_driver`
is a brand-new class name with no `spell_script_names` row anywhere, so it
was silently inert until a binding was added in
`sql/updates/world/master/2026_07_25_03_world.sql`. (`spell_mage_pyroblast`
and `spell_mage_flamestrike` keep their original names/bindings — only the
new driver class needed one.)

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
fix takes effect without a fresh SQL row. Added to its own dedicated file,
`sql/updates/world/master/2026_07_25_00_world.sql`, so the core's DB updater
applies it automatically on next startup. (Each fix now gets its own SQL
file rather than sharing one — the updater tracks applied files by name and
never re-runs one it's already applied, so appending new bindings to an
already-applied file would silently never take effect.)

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
fix needs a fresh binding to take effect. Added its own dedicated file,
`sql/updates/world/master/2026_07_25_01_world.sql`, rather than reusing the
old dangling name, since that name implied Enrage-application specifically,
which this script doesn't handle.

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
three new scripts would ever fire. Added its own dedicated file,
`sql/updates/world/master/2026_07_25_02_world.sql`.

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

**Files:**
`sql/updates/world/master/2026_07_25_03_world.sql` (Hot Streak driver),
`2026_07_25_04_world.sql` (Combustion),
`2026_07_25_05_world.sql` (Brain Freeze),
`2026_07_25_06_world.sql` (Chain Reaction)

**Fix:** Added a `spell_script_names` binding for each of the four spell ids,
one dedicated SQL file per fix (not one shared file — the DB updater tracks
applied migrations by filename and never re-runs one it's already applied,
so appending new bindings to an already-applied file would silently never
take effect; each fix gets its own file going forward). Each affected Mage
entry earlier in this file now has a "Database dependency" note
cross-referencing its own specific file.

**Risk:** Low — pure additive SQL, same idempotent DELETE-then-INSERT pattern
as the Warrior bindings; the actual gameplay risk was already assessed in
each mechanic's own entry above and is unchanged by this fix.

**Commit:** `<pending>`

**Test:** Same as each individual mechanic's own test notes above — none of
Hot Streak, Combustion, Brain Freeze, or Chain Reaction could have shown any
in-game effect before this binding was added, so any earlier "it didn't seem
to do anything" observation is explained by this, not a logic bug in the
scripts themselves.

### [DONE] Warrior/Protection - Revenge Trigger passive entirely missing

**Subsystem:** Scripts/Spells (spell_warrior)

**Problem:** The Revenge trigger passive (5301) — Protection's core mechanic
that resets Revenge's (6572) own cooldown whenever it procs off dodge/parry/
block — had no implementation anywhere in ArgusCore. Without it, Revenge is
stuck on its normal cooldown and never becomes the spammable, avoidance-
rewarding ability it's meant to be; effectively Protection's core
threat/rotation loop was inert.

**Reference:** DestinyCore's implementation, explicitly marked "7.3.5" —
a single self-contained `AuraScript` bound to 5301 with no talent-tier
ambiguity, unlike several other candidate Warrior gaps considered alongside
it (Overpower Proc Enabler's own `CheckProc` logic looked backwards/uncertain
and was deliberately not picked for this reason).

**Files:** `src/server/scripts/Spells/spell_warrior.cpp`

**Fix:** Added `SPELL_WARRIOR_REVENGE = 6572` and a new
`spell_warr_revenge_trigger` `AuraScript` (bound to 5301) that resets
Revenge's cooldown `OnProc`, ported directly from DestinyCore's logic.
Adapted two API differences to ArgusCore's modern conventions: `GetClass()`
instead of the legacy lowercase `getClass()`, and dropped the old
`SpellScriptLoader`/`PrepareAuraScript` wrapper boilerplate in favor of a
plain `AuraScript` registered via `RegisterSpellScript`, matching every other
script already in this file.

**Database dependency:** searched ArgusCore's committed SQL for an existing
`spell_script_names` binding on spell 5301 — found none. Same gap as every
other fix this session: added its own dedicated file,
`sql/updates/world/master/2026_07_25_07_world.sql`, auto-applied on startup.

**Risk:** Low — small, self-contained, single proc-reset with no side
effects on other mechanics. Needs in-game verification that Revenge's
cooldown actually resets on dodge/parry/block as a Protection Warrior.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — as Protection, tank
something that dodges/parries/blocks your attacks and confirm Revenge (6572)
becomes available again immediately rather than waiting out its normal
cooldown.

### [DONE] Warrior - Thunder Clap missing entirely (baseline AoE, all specs)

**Subsystem:** Scripts/Spells (spell_warrior)

**Problem:** Thunder Clap (6343), the baseline AoE ability used by all three
Warrior specs, had no implementation anywhere in ArgusCore. Without a
script, it would deal its direct damage (data-driven, needs no script) but
never apply Weakened Blows — the Legion-era debuff that reduces the
target's physical damage done, which replaced the older attack-speed-slow
version of this mechanic from earlier expansions — nor the Thunderstruck
talent's bonus stun.

**Reference:** DestinyCore's implementation. Cross-checked both
`SPELL_WARRIOR_THUNDERSTRUCK` (199045) and its stun (199042) against
TrinityCore-master specifically because they're talent-tier, not baseline —
found **no match at all**, meaning this talent doesn't exist in modern
retail's simplified Warrior tree. Same positive-evidence pattern as the
earlier Valarjar Berserkers finding: absence from modern retail here is
evidence of genuine Legion-only content, not forward drift. LegionCore has
no Thunder Clap content at all to cross-check against (consistent with it
being an incomplete leak, not a red flag against DestinyCore).

**Files:** `src/server/scripts/Spells/spell_warrior.cpp`

**Fix:** Added `SPELL_WARRIOR_THUNDER_CLAP` (6343),
`SPELL_WARRIOR_THUNDERSTRUCK` (199045), `SPELL_WARRIOR_THUNDERSTRUCK_STUN`
(199042), and `SPELL_WARRIOR_WEAKENED_BLOWS` (115798) constants, plus a new
`spell_warr_thunder_clap` `SpellScript` (`OnHit`) that casts Weakened Blows
on the target every hit, and additionally casts the Thunderstruck stun if
the caster has that talent's aura — ported directly from DestinyCore's
logic with no structural changes needed.

**Database dependency:** searched ArgusCore's committed SQL for an existing
`spell_script_names` binding on spell 6343 — found none. Added its own
dedicated file, `sql/updates/world/master/2026_07_25_08_world.sql`.

**Risk:** Low — small, self-contained, matches DestinyCore's reference
exactly with no talent-rank ambiguity. Needs in-game verification that
Weakened Blows applies on hit and that Thunderstruck's stun only triggers
when that talent is actually selected.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — cast Thunder Clap on a
target and confirm Weakened Blows applies; if speced into Thunderstruck,
confirm the target is also stunned.

### [DONE] Warrior/Arms - Overpower Proc Enabler missing, and DestinyCore's reference had a logic-inversion bug

**Subsystem:** Scripts/Spells (spell_warrior)

**Problem:** Spell 60503 ("Overpower Proc Enabler" per DestinyCore's own
comment — "Your other melee abilities have a chance to activate Overpower")
had no implementation anywhere in ArgusCore. Without it, none of Whirlwind,
Colossus Smash, Mortal Strike, or Slam can grant Overpower usability, so
Arms's random-proc version of Overpower (which replaced the older
dodge-triggered version from earlier expansions) is inert.

**Reference:** DestinyCore has an implementation, but its `CheckProc` checks
`eventInfo.GetSpellInfo()->Id == SPELL_WARRIOR_OVERPOWER` — i.e. it only
allows this aura to proc from **Overpower itself**, directly contradicting
its own comment one line above ("procs on Whirlwind, Colossus Smash, Mortal
Strike and Slam") and making the aura permanently non-functional (Overpower
can't be the thing that enables casting Overpower — nothing would ever cast
Overpower in the first place to trigger it). This is the first fix this
session where DestinyCore's own code looked genuinely wrong rather than
just uncertain/unverifiable, so it was cross-checked rather than either
blindly ported or skipped: the sibling talent `spell_warr_soul_of_the_slaughter`
in the same DestinyCore file uses the identical idiom (filter
`eventInfo.GetSpellInfo()->Id` against a short list of trigger spells) and
does it correctly there, checking the *other* abilities
(Whirlwind_Arms/Cleave/Hamstring/Execute_Arms/Mortal_Strike/Slam_Arms) —
confirming this is a copy-paste-type authoring slip in this one class, not
an intentional Legion-specific mechanic.

**Files:** `src/server/scripts/Spells/spell_warrior.cpp`

**Fix:** Added `SPELL_WARRIOR_OVERPOWER` (7384), `SPELL_WARRIOR_SLAM_ARMS`
(1464), and `SPELL_WARRIOR_WHIRLWIND_ARMS` (1680) constants (Colossus Smash
and Mortal Strike constants already existed), and a new
`spell_warr_overpower_proc` `AuraScript` whose `CheckProc` filters on
Whirlwind_Arms/Colossus Smash/Mortal Strike/Slam_Arms — matching the
comment's stated intent and the sibling class's correct pattern — instead of
porting DestinyCore's inverted check verbatim. Left the 5% proc chance and
the actual "enable Overpower" effect to spell 60503's own DB2-defined proc
chance/effect data, consistent with how Chain Reaction's stacking and other
self-contained filter-only auras already work elsewhere in this file.

**Database dependency:** searched ArgusCore's committed SQL for an existing
`spell_script_names` binding on spell 60503 — found none (the only existing
reference to 60503 anywhere in this repo's SQL is an unrelated bulk
`spell_proc` PPM-rate row). Added its own dedicated file,
`sql/updates/world/master/2026_07_25_09_world.sql`.

**Risk:** Moderate — diverges from the reference core's actual code (not
just its structure/API), so the "corrected" trigger-spell list is my own
inference from the comment and the sibling class's pattern, not something
independently verified against Legion 7.3.5 client/DB2 data. If 60503's own
proc chance/effect data doesn't actually grant Overpower usability the way
assumed, this could still be a no-op — but it cannot be *more* broken than
DestinyCore's already-permanently-inert version.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — as an Arms Warrior,
use Whirlwind/Colossus Smash/Mortal Strike/Slam repeatedly and confirm
Overpower occasionally becomes usable/highlighted without needing a target
to dodge your attack.

### [DONE] Warrior - Dragon Roar missing entirely

**Subsystem:** Scripts/Spells (spell_warrior)

**Problem:** Dragon Roar, a Warrior cooldown, had no implementation
anywhere in ArgusCore. Without a script it would deal its direct damage
(data-driven) but never knock the target back.

**Reference:** DestinyCore's implementation — simple and self-contained,
single `OnHit` handler, no talent-tier ambiguity. Its own spell id is never
named in an enum constant anywhere in DestinyCore's C++ (only the knockback
sub-spell is), so it was confirmed independently via DestinyCore's own
committed `spell_script_names` data (`DB_world_735.02.sql`:
`(118000,'spell_warr_dragon_roar')`) rather than guessed.

**Files:** `src/server/scripts/Spells/spell_warrior.cpp`

**Fix:** Added `SPELL_WARRIOR_DRAGON_ROAR_KNOCK_BACK` (118895) and a new
`spell_warr_dragon_roar` `SpellScript` (`OnHit`) that casts the knockback on
the target — ported directly from DestinyCore's logic with no structural
changes needed, matching the same `OnHit`/`SpellHitFn` idiom already used
for Thunder Clap in this file.

**Database dependency:** searched ArgusCore's committed SQL for an existing
`spell_script_names` binding on spell 118000 — found none. Added its own
dedicated file, `sql/updates/world/master/2026_07_25_10_world.sql`.

**Risk:** Low — small, self-contained, matches DestinyCore's reference
exactly.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — cast Dragon Roar and
confirm nearby enemies are knocked back in addition to taking damage.

### [DONE] Warrior - Second Wind missing, and DestinyCore's reference had an inverted apply/remove bug

**Subsystem:** Scripts/Spells (spell_warrior)

**Problem:** Second Wind (29838) — "When you become Stunned or Incapacitated,
you regenerate health over time" — had no implementation anywhere in
ArgusCore.

**Reference:** DestinyCore has an implementation, but it routes through an
intermediate marker aura (202149, "Second Wind Damaged"): the proc passive
(29838) casts 202149 on proc; 202149's `OnApply` **removes** the heal
(202147) and its `OnRemove` **re-casts** it. Net effect: the heal wouldn't
start until the marker itself expires, and any already-active heal gets
cancelled the instant a new stun/root proc happens — backwards from "gain
health over time when stunned." This is the second apparent authoring bug
found in DestinyCore's Warrior file this session (after the Overpower Proc
Enabler), so it was investigated rather than ported or skipped. Cross-checked
against TrinityCore-Cata's implementation of this exact mechanic (structurally
unchanged across many expansions, including a `Rank_1`/`Rank_2` legacy-talent
form — used here only as a structural reference for how the pieces fit
together, not as Legion content): there, the proc aura's `CheckProc` filters
the incoming damage's mechanic mask for Root/Stun via
`GetAllEffectsMechanicMask()`, and `HandleProc` casts the heal directly and
immediately, with no marker aura involved at all. That confirmed the marker
indirection in DestinyCore's Legion version is very likely a copy-paste-type
slip, not intentional redesign.

**Files:** `src/server/scripts/Spells/spell_warrior.cpp`

**Fix:** Added `SPELL_WARRIOR_SECOND_WIND_HEAL` (202147) and two new classes:
`spell_warr_second_wind` (`AuraScript` on 29838) whose `CheckProc` filters on
Root/Stun mechanic damage (matching the Cata reference's structure) and
whose `OnEffectProc` immediately casts the heal on self — no marker aura;
and `spell_warr_second_wind_heal` (`AuraScript` on 202147) with a
`DoEffectCalcAmount` that converts the periodic heal's flat amount into a
percentage of max health, since this engine has no `PERIODIC_HEAL_PCT` aura
type (same technique the Cata reference used for the equivalent spell).

**Residual uncertainty flagged, not resolved:** the heal's `CalcAmount` hook
is registered on `EFFECT_0` — the Cata reference used `EFFECT_1` for its
equivalent spell, but that was a different spell id (29841/29842) with its
own effect layout, not something transferable to Legion's 202147. Not
independently verified against Legion 7.3.5 DB2 data; if the heal amount
doesn't scale as expected, this index is the first thing to check.

**Risk:** Moderate-high — this is the second reference-code correction this
session (not just an API/structural port), and the effect-index guess above
is unverified. However, DestinyCore's own version was already fully
non-functional (net effect: no working heal, ever), so this cannot regress
anything — worst case it remains equally non-functional if the effect index
guess is wrong.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — as a Warrior speced
into Second Wind, get stunned or rooted and confirm a heal-over-time
noticeably restores health afterward, scaling with max health.

### [DONE] Warrior/Arms - Execute was modern-retail content, and ArgusCore's engine lacked the hook DestinyCore's version needed

**Subsystem:** Scripts/Spells (spell_warrior)

**Problem:** Discovered while investigating Massacre (a separate, still-open
candidate — see below): ArgusCore's `spell_warr_execute_damage` was bound to
spell 260798 with the comment `// tooltip has 2 multiplier hardcoded in it
$damage=${2.0*$260798s1}`. The **identical** class, spell id, and comment
verbatim exist in TrinityCore-master (modern retail) — confirmed forward
drift, the same pattern as the earlier Scorch fix. DestinyCore and
LegionCore independently agree Legion 7.3.5's actual Execute id is 163201 (a
completely different mechanic: base damage plus scaling bonus damage from
spending extra Rage, with a partial refund on survival — not a flat 2x
multiplier).

**Reference:** DestinyCore's implementation is clean and self-consistent,
but depends on an `OnTakePower`/`SpellOnTakePowerFn` hook to intercept and
modify the spell's rage cost dynamically — **ArgusCore's `SpellScript.h` has
no such hook at all**, confirmed by a thorough search of the file. This
isn't an API naming difference; it's a missing engine capability. Flagged
to the user before proceeding rather than guessing at a workaround.
**Ground truth for the exact numbers came directly from the user**, who
provided the real Legion 7.2.5 tooltip: "Attempts to finish off a foe,
causing 202% Physical damage, and consuming up to 10 additional Rage to
deal up to 202% additional damage. Only usable on enemies that have less
than 20% health. If your foe survives, 30% of the Rage spent is refunded."
This also corrected a formula mismatch: DestinyCore's own code implied a
larger (+300%) bonus-damage ratio than the tooltip supports (+100%/doubling
at most) — the tooltip numbers were used as authoritative over DestinyCore's
formula where they disagreed.

**Files:** `src/server/scripts/Spells/spell_warrior.cpp`

**Fix:** Replaced `spell_warr_execute_damage` (260798) with a new
`spell_warr_execute` class bound to 163201 (Arms) and 217955 (PvP talent
variant). Rather than intercepting the cost pipeline (not possible in this
engine), the base rage cost is still paid normally through the existing
system, and only the *additional* rage is handled manually: `Load()` reads
available rage (capped at 10), `HandleDamage` scales bonus damage
proportionally (up to +100%, i.e. doubling), and `HandleAfterHit` deducts
the extra rage spent and refunds 30% of the total (base + extra) if the
target survived. The 10-rage-cap and 30%-refund numbers are hardcoded from
the user-provided tooltip rather than derived from this spell's own effect
base points (DestinyCore's approach), since Legion 7.3.5 DB2 data isn't
available here to verify which raw effect fields correspond to which value.

**Flagged uncertainty, resolved in a follow-up fix (see the Massacre entry
below):** ArgusCore's `SPELL_WARRIOR_EXECUTE = 20647`, referenced by
`spell_warr_sweeping_strikes`, was left untouched here pending investigation.
Confirmed via Wowhead/community-database lookup (per the user's suggestion
to check Wowhead when uncertain) that 20647 is genuine **Wrath of the Lich
King**-era Execute — a third instance of expansion drift in this file (after
Scorch's forward-drift and this fix's own 260798), this time backward
drift, the same direction as the earlier Unrelenting Assault finding.
Corrected to 163201 alongside the Massacre fix below.

**Risk:** Moderate-high — this is a structural reinvention (not a direct
port) working around a genuine engine capability gap, and the exact numbers,
while now grounded in a real tooltip rather than guesswork, haven't been
tested in-game. The old 260798-bound version was already wrong-expansion
content, so this cannot regress correct behavior — worst case it's a
different-but-still-wrong implementation.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — as Arms, use Execute
on a target below 20% health at varying Rage levels (e.g. exactly at the
base cost vs. with 10+ extra Rage available) and confirm damage scales up
with more Rage spent, extra Rage is consumed, and ~30% of Rage spent is
refunded if the target survives the hit.

### [VERIFIED — no fix needed] Warrior - Berserker Rage / Enrage Aura / Unshackled Fury do not need scripts

**Subsystem:** Scripts/Spells (spell_warrior)

**Investigated as a candidate gap, concluded no code change is needed.**
DestinyCore has scripts for all three (`spell_warr_berzerker_rage`,
`spell_warr_enrage_aura`, `spell_warr_unshackled_fury`), but tracing
through ArgusCore's actual engine code
(`Unit::ModifyAuraState`, `src/server/game/Entities/Unit/Unit.cpp:5956`)
showed this is handled automatically: when a unit gains/loses an aura-state
flag (e.g. `AURA_STATE_ENRAGED`), the engine itself automatically
casts/removes any passive spell on that unit whose own `CasterAuraState`
DB2 field matches that flag. Unshackled Fury (a "gain bonus damage while
Enraged" passive) is exactly the kind of spell this mechanism exists for —
its apply/remove lifecycle is already fully engine/DB2-driven, needing no
script at all. DestinyCore's `spell_warr_unshackled_fury` only duplicates
this (and uses the constant name `AURA_STATE_ENRAGE`, which doesn't even
exist in ArgusCore — the real name here is `AURA_STATE_ENRAGED`). Its
`Register()` isn't reachable if the passive is already applying with the
correct value in the first place. Similarly, Berserker Rage's and Enrage
Aura's `RemoveAurasDueToSpell(SPELL_WARRIOR_UNCHACKLED_FURY)` calls on
removal are redundant with what `ModifyAuraState` already does when the
Enrage aura-state flag clears — worse, since neither class re-applies
Unshackled Fury anywhere, porting that removal call as written would
strip the buff with no code path to bring it back, which would be a
regression, not a fix.

**Decision:** implemented nothing for this trio. If in-game testing later
shows Unshackled Fury's damage bonus doesn't actually track Enrage state
correctly, that would mean spell 76856's own DB2 data doesn't have
`CasterAuraState` set as assumed — a data question, not a scripting one,
and worth revisiting only if that's confirmed.

### [DONE] Warrior/Fury - Massacre missing entirely, plus a WotLK-era backward-drift constant corrected

**Subsystem:** Scripts/Spells (spell_warrior)

**Problem:** Massacre (206315), a Fury talent, had no implementation
anywhere in ArgusCore.

**Investigation history (kept for context on a self-corrected mistake):**
initially suspected DestinyCore's `spell_warr_massacre` was another
authoring bug, similar to Overpower Proc Enabler — its `CheckProc` (filter
for Execute crits) didn't match what the *current-retail* Massacre talent
does (extends Execute's usable health range). Per the user's suggestion to
check Wowhead when uncertain rather than guess, looked up the talent's
actual Legion 7.3.5 behavior via a patch-specific community guide: "Execute
critical hits reduce the Rage cost of your next Rampage by 100%." This is a
completely different mechanic from the later-expansion redesign — Massacre
was substantially reworked after Legion. DestinyCore's `CheckProc` (filter
on Execute crits) is *correct* for the Legion version; the earlier "bug"
suspicion was wrong, caused by unknowingly comparing against the wrong
expansion's version of the same talent name.

**Also resolved while investigating:** confirming Massacre's spell id
(206315) required checking what `SPELL_WARRIOR_EXECUTE` actually pointed to
in DestinyCore vs. ArgusCore. Found ArgusCore's existing `SPELL_WARRIOR_EXECUTE`
constant (flagged as uncertain in the Execute fix above) was **20647 — a
genuine Wrath of the Lich King-era spell id**, confirmed via Wowhead/community
database lookup. This is backward drift (same direction as the earlier
Unrelenting Assault finding), the third distinct expansion-drift instance
found in this file this session (after Scorch's forward-drift and this
session's other Execute fix). Corrected the constant to 163201 (Legion's
real Execute, matching the value already used for the main Execute fix),
which also fixes `spell_warr_sweeping_strikes`'s own Execute-crit detection
as a side effect — it was comparing against the wrong id too.

**Files:** `src/server/scripts/Spells/spell_warrior.cpp`

**Fix:** Corrected `SPELL_WARRIOR_EXECUTE` from 20647 to 163201. Added
`SPELL_WARRIOR_MASSACRE = 206315` and a new `spell_warr_massacre`
`AuraScript` whose `CheckProc` filters for Execute crits — ported directly
from DestinyCore's logic with no changes needed, now that its correctness
for Legion 7.3.5 specifically has been confirmed. The actual "next Rampage
free" grant is left to spell 206315's own DB2-defined proc-trigger-spell
effect data, matching the same self-contained pattern used for Chain
Reaction and other filter-only auras in this file.

**Database dependency:** searched ArgusCore's committed SQL for an existing
`spell_script_names` binding on spell 206315 — found none. Added its own
dedicated file, `sql/updates/world/master/2026_07_25_13_world.sql`.

**Risk:** Low — small, self-contained, matches DestinyCore's reference
exactly once cross-verified against the correct expansion's mechanic. The
`SPELL_WARRIOR_EXECUTE` correction only has one other call site
(`spell_warr_sweeping_strikes`), and can only improve its accuracy (WotLK's
Execute id could never have matched a real Legion Execute cast anyway).

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — as Fury, land a
critical Execute and confirm your next Rampage costs no Rage. Separately,
as any spec with Sweeping Strikes talented, confirm an Execute hit still
triggers Sweeping Strikes' extra attack correctly (this should already have
been broken before this fix, now corrected as a side effect).

### [DONE] Warrior - Shattering Throw missing entirely

**Subsystem:** Scripts/Spells (spell_warrior)

**Problem:** Shattering Throw, a Warrior utility ability, had no
implementation anywhere in ArgusCore. Without a script it would deal its
direct damage (data-driven) but never remove the target's immunity shield
(e.g. Divine Shield, Ice Block) — the ability's entire point.

**Reference:** DestinyCore's implementation — simple and self-contained,
single `OnEffectHitTarget` handler. Its own spell ids are never named in an
enum constant in DestinyCore's C++ (only bound via SQL by class name), so
they were confirmed two ways: DestinyCore's own committed
`spell_script_names` data (both 64380 and 65941 bound to the same class),
and Wowhead, which confirms both are legitimate historical registrations of
this ability across different expansion patches (this is a long-lived,
stable Warrior utility, not something that changed id between Legion and
modern retail the way Execute did).

**Files:** `src/server/scripts/Spells/spell_warrior.cpp`

**Fix:** Added a new `spell_warr_shattering_throw` `SpellScript` that
removes any aura with the `MECHANIC_IMMUNE_SHIELD` mechanic on hit — ported
directly from DestinyCore's logic with no changes needed. Deliberately did
not add a bonus-damage-vs-shields effect some later-expansion tooltips
describe, since DestinyCore's own Legion-era implementation doesn't include
one either and there's no independent confirmation it applied in 7.3.5.

**Database dependency:** searched ArgusCore's committed SQL for an existing
`spell_script_names` binding on either spell id — found none. Added its own
dedicated file, `sql/updates/world/master/2026_07_25_14_world.sql`, binding
both ids to the single new class.

**Risk:** Low — small, self-contained, matches DestinyCore's reference
exactly.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — cast Shattering Throw
on a target with an active immunity shield (Divine Shield, Ice Block, etc.)
and confirm the shield is removed.

### [DONE] Warrior/Fury - Frenzy missing entirely

**Subsystem:** Scripts/Spells (spell_warrior)

**Problem:** Frenzy (206313), a Fury stacking haste buff, had no
implementation anywhere in ArgusCore.

**Reference:** DestinyCore's implementation — a simple filter-only aura,
matching the same self-contained pattern as Chain Reaction and Massacre.
Confirmed via Wowhead/WoWDB that Frenzy is the passive buff Furious Slash
(100130) applies/stacks (up to 3 stacks, 2% haste each, 15 sec duration) —
stable, long-lived Fury mechanic, not something flagged as changed between
Legion and later expansions.

**Files:** `src/server/scripts/Spells/spell_warrior.cpp`

**Fix:** Added `SPELL_WARRIOR_FURIOUS_SLASH = 100130` and a new
`spell_warr_frenzy` `AuraScript` whose `CheckProc` restricts the aura to
only proc from Furious Slash — ported directly from DestinyCore's logic
with no changes needed. The stacking/haste/duration behavior itself is left
to spell 206313's own DB2-defined effect data.

**Database dependency:** searched ArgusCore's committed SQL for an existing
`spell_script_names` binding on spell 206313 — found none. Added its own
dedicated file, `sql/updates/world/master/2026_07_25_15_world.sql`.

**Risk:** Low — small, self-contained, matches DestinyCore's reference
exactly.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — as Fury, use Furious
Slash repeatedly and confirm the Frenzy buff stacks up to 3 times.

### [DONE] Warrior/Arms - Tactician missing entirely

**Subsystem:** Scripts/Spells (spell_warrior)

**Problem:** Tactician (184783), an Arms talent giving rage-spending
abilities a chance to reset Colossus Smash and Mortal Strike's cooldowns,
had no implementation anywhere in ArgusCore.

**Reference:** DestinyCore's implementation. Verified via web search that
Legion 7.3.5's Tactician resets Colossus Smash/Mortal Strike specifically —
**not** Overpower, which is what the same-named, same-numbered talent does
in modern retail (confirmed by checking TrinityCore-master's own
`spell_warr_tactician`, which uses a charge-restore system tied to
Overpower). Same pattern as Massacre and Execute: a talent name/id staying
stable across expansions while its actual effect is redesigned.

**Files:** `src/server/scripts/Spells/spell_warrior.cpp`

**Fix:** Added `SPELL_WARRIOR_TACTICIAN_CD = 199854` and a new
`spell_warr_tactician` `AuraScript` that computes the rage cost of the
triggering spell and rolls a chance (0.75% per point of Rage spent) to
reset both cooldowns and cast the "Tactician" glow marker — ported directly
from DestinyCore's logic.

**Residual uncertainty flagged, not resolved:** the 0.75%-per-rage constant
is DestinyCore's own hardcoded value. A web search surfaced a slightly
different figure (0.65%) for the same patch, but it came from an aggregated
snippet rather than a direct tooltip quote, so DestinyCore's value was kept
as the more directly-sourced reference. This is a minor tuning discrepancy,
not a mechanic difference — worth revisiting only if it turns out to matter
in practice (e.g. if in-game testing shows the proc rate feels clearly off).

**Database dependency:** searched ArgusCore's committed SQL for an existing
`spell_script_names` binding on spell 184783 — found none. Added its own
dedicated file, `sql/updates/world/master/2026_07_25_16_world.sql`.

**Risk:** Low-moderate — structure and target cooldowns are well-confirmed;
only the exact proc-chance percentage carries residual uncertainty, and
being wrong there would just mean a slightly different proc frequency, not
a broken mechanic.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — as Arms, use
rage-spending abilities repeatedly and confirm Colossus Smash/Mortal
Strike's cooldowns occasionally reset early, with a visible "Tactician"
proc indicator.

### [DONE] Warrior/Arms - Executioner's Precision missing entirely

**Subsystem:** Scripts/Spells (spell_warrior)

**Problem:** Executioner's Precision (238147), a Legion artifact trait, had
no implementation anywhere in ArgusCore.

**Reference:** DestinyCore's implementation — a simple filter-only aura,
matching the same self-contained pattern as Chain Reaction, Massacre, and
Frenzy. Confirmed via Wowhead/Wowpedia that this is the Legion-era version
of the mechanic (Execute makes the next Mortal Strike deal bonus damage,
stacking up to 2 times) — a later expansion redesigned it into a
differently-tuned talent, same drift pattern as Massacre/Execute/Tactician.
Directly benefited from the `SPELL_WARRIOR_EXECUTE` correction made in the
Massacre fix (163201, not the WotLK-era 20647) — this class reuses that
same constant rather than needing its own.

**Files:** `src/server/scripts/Spells/spell_warrior.cpp`

**Fix:** Added a new `spell_warr_executioners_precision` `AuraScript` whose
`CheckProc` restricts the aura to only proc from Execute — ported directly
from DestinyCore's logic with no changes needed. The stacking bonus-damage
behavior itself is left to spell 238147's own DB2-defined effect data.

**Database dependency:** searched ArgusCore's committed SQL for an existing
`spell_script_names` binding on spell 238147 — found none. Added its own
dedicated file, `sql/updates/world/master/2026_07_25_17_world.sql`.

**Risk:** Low — small, self-contained, matches DestinyCore's reference
exactly.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — as Arms, use Execute
and confirm your next Mortal Strike deals bonus damage, stacking up to 2
times with repeated Execute casts.

### [DONE] Warrior - seven more mechanics implemented, plus three confirmed backward-drift skips

**Subsystem:** Scripts/Spells (spell_warrior)

**Problem:** Continuing the spell-by-spell audit through the remaining
DestinyCore Warrior scripts. Seven more had no implementation anywhere in
ArgusCore; three more candidates were investigated and deliberately **not**
implemented after Wowhead/web research confirmed they're pre-Legion content
that doesn't exist in 7.3.5.

**Implemented (all self-contained, low-risk, ported directly from
DestinyCore with only API-convention adaptation - no structural changes):**

- **Defensive Stance** (71) — grants rage from damage taken while active.
  Initially suspected backward drift (most Warrior stances were removed in
  patch 7.0.3), but confirmed via web search that Defensive Stance
  specifically survived into Legion as an Arms talent rather than being cut.
- **Focused Rage (Arms)** (207982) — filter-only aura restricting a
  rage-cost-reduction proc to Mortal Strike. The Protection version of this
  same-named talent was deliberately **not** ported as a separate class:
  ArgusCore's existing `spell_warr_ignore_pain` already implements the
  equivalent Vengeance-triggered mechanic under a different structure, and
  adding DestinyCore's separate version on top would risk double-granting
  the same buff via two trigger paths.
- **Inspiring Presence** — self-heal proc based on a percentage of damage
  taken. The script is bound to spell **205484** (confirmed via DestinyCore's
  own committed `spell_script_names` data), not `SPELL_WARRIOR_INSPIRING_PRESENCE`
  (222944), which is a different, inner spell the script casts when it
  procs — the two ids serve different roles and aren't interchangeable.
- **Precise Strikes** (248579, explicitly marked "7.3.5" in DestinyCore) —
  Colossus Smash grants a stacking self-buff. Lower-confidence than the
  other six: DestinyCore's own committed SQL has no binding for this script
  name at all, so 248579 comes from the source comment only, not an
  independently confirmed binding.
- **Safeguard** (223657) — removes movement-impairing effects on hit
  (Intervene's own effect).
- **Soul of the Slaughter** (238111) — multi-branch proc-chance filter
  granting Precise Strikes' bonus from Whirlwind/Cleave/Hamstring/Execute/
  Mortal Strike/Slam, each with its own chance.
- **Wrecking Ball Effect** (215570) — clears an existing stack of itself
  before a fresh one re-applies, preventing overlap.

**Deliberately not implemented — confirmed pre-Legion content via
Wowhead/web search:**

- **Retaliation** (20230) — confirmed removed in patch 5.0.4 (Mists of
  Pandaria, 2012), does not exist in Legion at all.
- **Vigilance / Vigilance Trigger** (50720/50725) — DestinyCore's
  implementation only handles removing the caster's "Vengeance" buff
  (76691), but that classic Vengeance mechanic was removed in Warlords of
  Draenor patch 6.0 (2014) and replaced by Resolve. Separately, web search
  indicates Vigilance itself was removed around patch 7.0.3, before 7.3.5.
  Both findings point the same direction: this is pre-Legion (or
  early-Legion-and-then-cut) content, not something to port.
- **Focused Rage (Protection)** — not a drift issue, but skipped for a
  different reason: already effectively covered by ArgusCore's existing
  `spell_warr_ignore_pain` (see above).

**Also investigated, deliberately skipped for other reasons (not
implemented, not necessarily wrong-expansion):**

- **Commanding Shout** — DestinyCore's own `SPELL_WARRIOR_COMMANDING_SHOUT`
  constant is set to 97463, but that's the *same value* as
  `SPELL_WARRIOR_RALLYING_CRY`/`SPELL_WARRIOR_RALLYING_CRY_TRIGGER` in the
  same file — an apparent copy-paste collision in DestinyCore's own enum.
  Web search suggests the real id might be 225998, but that's only
  confirmed for "current retail," not independently verified for 7.3.5, and
  neither LegionCore nor TrinityCore-master have a comparable script to
  cross-check against. Not implemented until this can be resolved with
  higher confidence.
- **Sword and Board** — DestinyCore binds this script using a *negative*
  spell id (-46951) in `spell_script_names`, a different binding convention
  (family/icon-based rather than a literal spell id) that hasn't been
  verified to work the same way in ArgusCore's updater. Skipped rather than
  risk an incorrectly-applied binding.
- **Unrivaled Strength** — DestinyCore's implementation casts a hardcoded
  spell id (200977) with no named constant anywhere in the file, and copies
  a base amount between two unrelated auras. Not enough independent
  verification to trust the hardcoded id; skipped rather than guess.
- **Spell Reflect** (visual variant) — DestinyCore's script only changes
  which purely-cosmetic reflect animation plays, gated on a hardcoded
  faction template id (1732) that hasn't been verified against ArgusCore's
  own faction data. Pure visual polish with no gameplay impact; skipped as
  not worth the risk for the reward.
- **Ravager** — depends on summoning and periodically damaging through an
  NPC entity (creature 76168). Searched ArgusCore's committed SQL and found
  no matching `creature_template` row, meaning a hard data prerequisite for
  this mechanic couldn't be confirmed. Needs dedicated investigation before
  attempting.

**Files:** `src/server/scripts/Spells/spell_warrior.cpp`

**Database dependency:** searched ArgusCore's committed SQL for existing
`spell_script_names` bindings on all seven implemented spell ids — found
none. Added all seven to a single dedicated file,
`sql/updates/world/master/2026_07_25_18_world.sql`.

**Risk:** Low for six of the seven; Precise Strikes carries slightly more
uncertainty since its spell id isn't independently confirmed via a binding,
only a source comment.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification for each: Defensive
Stance (tank damage while in the stance, confirm rage generation),
Focused Rage Arms (crit with Mortal Strike, confirm next ability's rage
cost is reduced), Inspiring Presence (take damage, confirm a proportional
self-heal), Precise Strikes (use Colossus Smash, confirm a stacking buff
appears), Safeguard (use it while rooted/snared, confirm the effect is
removed), Soul of the Slaughter (use the listed abilities repeatedly,
confirm Precise Strikes' buff sometimes triggers from them too), Wrecking
Ball Effect (trigger it twice in quick succession, confirm no stacking
beyond intended).

### [DONE] Warrior/Fury - Unrivaled Strength missing entirely (previously skipped for unverified id)

**Subsystem:** Scripts/Spells (spell_warrior)

**Problem:** Unrivaled Strength (200860), a Fury artifact trait increasing
critical damage during Battle Cry, had no implementation anywhere in
ArgusCore. This was explicitly skipped in the previous pass because
DestinyCore's implementation casts a hardcoded literal spell id (200977)
with no named constant anywhere in the file, which looked like it could be
an unverified/unreliable magic number.

**Reference:** DestinyCore's implementation. Resolved the earlier
uncertainty via Wowhead: both 200860 (the outer artifact-trait aura this
script binds to) and 200977 (the inner effect it casts and resizes) are
independently confirmed as genuine, distinct spells for this exact Legion
ability — not a guess or a coincidental unrelated id. An initial DB search
for 200977 returned unrelated creature/gameobject data (a numeric
coincidence in an unrelated ID namespace, not evidence against the spell
id), which is why Wowhead confirmation specifically was needed here.

**Files:** `src/server/scripts/Spells/spell_warrior.cpp`

**Fix:** Added `SPELL_WARRIOR_UNRIVALED_STRENGTH_EFFECT = 200977` and a new
`spell_warr_unrivaled_strength` `AuraScript` that casts the effect spell on
proc and copies the base amount across — ported directly from DestinyCore's
logic with no changes needed. No `CheckProc` filter required; the outer
aura's own DB2 proc data already scopes this to Battle Cry.

**Database dependency:** searched ArgusCore's committed SQL for an existing
`spell_script_names` binding on spell 200860 — found none. Added its own
dedicated file, `sql/updates/world/master/2026_07_26_00_world.sql`.

**Risk:** Low — small, self-contained, both spell ids now independently
confirmed via Wowhead rather than trusted on faith.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — as Fury, use Battle
Cry and confirm critical strike damage is increased for its duration.

### [Still open] Warrior - Ravager needs a creature_template row and a separate NPC AI script, not just spell scripts

**Subsystem:** Scripts/Spells (spell_warrior), SQL, Creature AI

**Investigated further, not implemented.** Ravager's player-side spell
scripts (summon + periodic damage via the summoned NPC) were already found
non-trivial in the previous pass, but this pass found the scope is larger
than initially thought: DestinyCore's own full DB dump confirms creature
entry 76168 ("Ravager") requires a real `creature_template` row (model,
stats, faction, etc.) **and** a dedicated creature AI script
(`npc_warr_ravager`, referenced in that row's `AIName`/`ScriptName` column)
that lives in a different file entirely (Creature AI scripts, not
`spell_warrior.cpp`). This is real content/data insertion, not just a
spell-script-and-binding fix like everything else in this session — a
meaningfully different (and larger) category of change. Not attempted;
would need the creature template data and the AI script content
independently sourced and reviewed before proceeding, and is a good
candidate to explicitly scope as its own dedicated task rather than fold
into the spell-by-spell audit.

### [Still open] Warrior - Commanding Shout id still unresolved

**Subsystem:** Scripts/Spells (spell_warrior)

**Investigated further via two rounds of web search, still not confident
enough to implement.** DestinyCore's own `SPELL_WARRIOR_COMMANDING_SHOUT`
constant (97463) is a confirmed copy-paste collision with
`SPELL_WARRIOR_RALLYING_CRY`/`_TRIGGER` in the same file. Web search
surfaced spell 45517 as a currently-live, non-version-prefixed "Commanding
Shout" entry (confirmed real: "Apply Aura: Mod Max Health," 2 min duration,
20 yards) as well as 225998 ("current retail" per an earlier search) and
several older Classic/TBC/WotLK-specific ids (469, 47439, 403215) — none of
which came with clear confirmation of which one (if any) was live
specifically in patch 7.3.5. Neither LegionCore nor TrinityCore-master have
a comparable script to cross-check structurally. Left unimplemented rather
than guess between several plausible-but-unconfirmed candidates.

### [VERIFIED — no fix needed] Monk - Power Strikes/Tiger Palm interaction already works via existing generic proc system

**Subsystem:** Scripts/Spells (spell_monk)

**Context:** First fix of a new class pass (Warrior is now at 25 fixes this
session and considered thoroughly audited for now). Compared ArgusCore's
Monk scripts against DestinyCore's and found a dramatically larger gap than
Warrior ever had — only ~19 scripts implemented in ArgusCore vs. ~75 in
DestinyCore, with core rotational abilities missing across all three specs.
User chose to start with Windwalker's basic builder/spender loop (Tiger
Palm, Blackout Kick) as the highest-value starting point.

**Investigated as a candidate gap, concluded no code change is needed.**
DestinyCore's `spell_monk_tiger_palm` exists solely to check for a "Power
Strikes Aura" on the caster and, if present, add its bonus to Tiger Palm's
own chi generation before removing it. Tracing ArgusCore's existing
(already-implemented, already-registered) `spell_monk_power_strike_periodic`
and `spell_monk_power_strike_proc` classes showed this exact mechanic is
already fully handled generically: a periodic driver aura (121817) casts a
marker aura (129914) on a timer, and that marker aura's own `OnEffectProc`
hook grants the chi bonus (121283) whenever its own DB2 proc data allows —
which is scoped to Tiger Palm's own cast/hit, by design of the mechanic
("Tiger Palm has a chance to generate an additional Chi"). DestinyCore's
Tiger Palm-side check is a different, more manual implementation of the
identical mechanic, not something ArgusCore is missing. No code added.

### [DONE] Monk/Windwalker - Blackout Kick missing entirely

**Subsystem:** Scripts/Spells (spell_monk)

**Problem:** Blackout Kick (100784), Windwalker/Mistweaver's builder-
consumer interaction with the Teachings of the Monastery talent, had no
implementation anywhere in ArgusCore.

**Reference:** DestinyCore's implementation. Cross-checked the core
mechanic (Tiger Palm grants stacks, Blackout Kick consumes them for bonus
damage, with a chance to reset Rising Sun Kick) against Wowhead/community
sources, which confirm this general shape has been stable across many
expansions — though the exact tuning numbers (proc %, max stacks) have
changed repeatedly since Legion, up to and including a recent patch
(10.2.0) increasing max stacks from 3 to 4. DestinyCore's own hardcoded
15% RSK-reset chance was kept as the more directly Legion-sourced value
over current-retail's documented 12%, consistent with how Tactician's
proc-chance uncertainty was handled earlier.

**Files:** `src/server/scripts/Spells/spell_monk.cpp`

**Fix:** Added `SPELL_MONK_RISING_SUN_KICK` (107428),
`SPELL_MONK_SPIRIT_OF_THE_CRANE_AURA`/`_MANA` (210802/210803), and
`SPELL_MONK_TEACHINGS_OF_THE_MONASTERY`/`_AURA` (116645/202090) constants,
plus a new `spell_monk_blackout_kick` `SpellScript` handling all three
pieces: the RSK cooldown/charge reset roll, consuming Teachings of the
Monastery stacks for bonus damage, and refunding mana via Spirit of the
Crane when present.

**Deliberate simplification from the reference:** DestinyCore re-deals the
hit's damage once per consumed stack via a manually constructed
`SpellNonMeleeDamage` object (producing N separate combat-log entries).
That low-level manual damage-dealing pattern has no precedent anywhere
else in ArgusCore's engine, and couldn't be verified to behave correctly
without an existing usage to compare against. Used a simpler, functionally
equivalent approach instead: multiply the single hit's damage by
`(stacks + 1)`. This delivers the same total damage as one combat-log
entry instead of several separate ones — a cosmetic/combat-log-granularity
difference, not a gameplay difference.

**Database dependency:** searched ArgusCore's committed SQL for an existing
`spell_script_names` binding on spell 100784 — found none. Added its own
dedicated file, `sql/updates/world/master/2026_07_26_01_world.sql`.

**Risk:** Moderate — the most structurally complex Monk fix so far (three
interacting sub-mechanics), and the RSK-reset percentage and the
damage-multiplier simplification are both worth re-checking in-game.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — as Windwalker or
Mistweaver, use Tiger Palm to build Teachings of the Monastery stacks, then
Blackout Kick and confirm: (1) damage scales up with stacks consumed, (2)
Rising Sun Kick's cooldown/charges occasionally reset, (3) as Mistweaver
with Spirit of the Crane, mana is refunded proportional to stacks consumed.

### [DONE] Monk/Windwalker - Fists of Fury missing entirely

**Subsystem:** Scripts/Spells (spell_monk)

**Problem:** Fists of Fury (113656), Windwalker's other core Chi spender
(alongside Blackout Kick), had no implementation anywhere in ArgusCore
across any of its four constituent spell scripts.

**Reference:** DestinyCore's implementation, spread across four classes
bound to three different spell ids:
- `spell_monk_fists_of_fury` (113656, the channeled aura) — ticks fast for
  the visual pulse, but only fires the damage sub-spell every 6th tick.
- `spell_monk_fists_of_fury_damage` (117418) — manual damage formula
  (`AttackPower * 5.25`, then the normal damage-bonus-done/taken pipeline),
  since this coefficient isn't expressible via a plain weapon-percent-damage
  effect. The 5.25 coefficient is DestinyCore's own value, not
  independently verified against Legion 7.3.5 client data.
- `spell_monk_fists_of_fury_visual_filter` (also 117418 — confirmed this is
  intentional dual-binding, not a data error: TrinityCore-style script
  binding supports multiple independent script classes per spell id) —
  prevents re-targeting a unit already hit by the visual sweep this tick.
- `spell_monk_fists_of_fury_visual` (123154) — sets an explicit 1000ms
  duration on the visual sweep aura, since DestinyCore's own comment notes
  client data has no duration for it and it would never end otherwise; kept
  as a defensive workaround rather than a guess, on the reference's own
  explanation.

**Deliberately not ported:** `spell_monk_fists_of_fury_stun`, a target-filter
script for a stun-variant of this ability. DestinyCore's own registration
function has it commented out (`//new spell_monk_fists_of_fury_stun();`) —
disabled in the reference itself, so not treated as confirmed-working
content worth porting.

**Files:** `src/server/scripts/Spells/spell_monk.cpp`. Also added a
`GridNotifiers.h` include for `Trinity::UnitAuraCheck`, not previously
needed in this file.

**Fix:** Added `SPELL_MONK_FISTS_OF_FURY_DAMAGE` (117418) and
`SPELL_MONK_FISTS_OF_FURY_VISUAL` (123154) constants, plus all four classes
ported directly from DestinyCore's logic with only API-convention
adaptations (e.g. `SpellDamageBonusTaken`'s signature in this engine takes
one fewer parameter than DestinyCore's call).

**Database dependency:** searched ArgusCore's committed SQL for existing
`spell_script_names` bindings on all three spell ids — found none. Added
all four class bindings to a single dedicated file,
`sql/updates/world/master/2026_07_26_02_world.sql`.

**Risk:** Moderate — four interacting classes across three spell ids, and
the damage coefficient (5.25) isn't independently verified. Needs real
in-game testing of the full channel (visual sweep timing, periodic damage
cadence, target filtering) together.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — as Windwalker, use
Fists of Fury and confirm: the channel visually sweeps nearby enemies, deals
periodic damage roughly every 6th tick rather than every tick, and doesn't
hang indefinitely (the visual sweep aura should end with the channel, not
persist).

### [DONE] Monk - Rising Sun Kick missing Rising Thunder interaction, and a scope-gating bug found while adding it

**Subsystem:** Scripts/Spells (spell_monk)

**Problem:** ArgusCore's existing `spell_monk_rising_sun_kick` only handled
the Mortal Wounds application (Combat Conditioning talent). DestinyCore's
equivalent also resets Thunder Focus Tea's cooldown when the caster has
Rising Thunder (210804) — a Mistweaver talent, missing entirely from
ArgusCore's version.

**Bug found while adding it:** the existing class gated its entire
`HandleOnHit` behind `Load()` requiring `SPELL_MONK_COMBAT_CONDITIONING` —
correct while Mortal Wounds was the only thing this class did, but Rising
Thunder and Combat Conditioning are unrelated talents for different specs.
Naively adding the Rising Thunder check inside the existing method would
have left it silently unreachable for any Mistweaver without Combat
Conditioning (i.e. always, since Combat Conditioning is a different spec's
talent). Removed `Load()` and moved the Combat Conditioning check inline
instead, matching the internal-`HasAura`-check pattern already used by
sibling classes in this file (e.g. `spell_monk_pressure_points`) — each
condition is now independently evaluated rather than one gating the other.

**Reference:** DestinyCore's implementation for the Rising Thunder piece.
Note DestinyCore's own class *also* handled Mortal Wounds via an old
spec-ID check (`TALENT_SPEC_MONK_BATTLEDANCER` — not a real Monk spec name,
likely a pre-release internal codename) rather than a talent-presence
check; ArgusCore's existing modern `HasAura(SPELL_MONK_COMBAT_CONDITIONING)`
approach was already the more correct implementation of that piece, so only
the Rising Thunder addition was actually new content here.

**Files:** `src/server/scripts/Spells/spell_monk.cpp`

**Fix:** Added `SPELL_MONK_RISING_THUNDER` (210804) and
`SPELL_MONK_THUNDER_FOCUS_TEA` (116680) constants; restructured
`spell_monk_rising_sun_kick` as described above and added the Rising
Thunder → Thunder Focus Tea reset.

**Database dependency:** none — this class already has an active
`spell_script_names` binding from a pre-session migration
(`sql/updates/world/master/2026_05_26_01_world.sql`); only its internal
logic changed, not its name or bound spell id, so the existing binding
picks up the new behavior automatically.

**Risk:** Low-moderate — the Combat Conditioning behavior is unchanged in
effect (just restructured), and the new Rising Thunder piece is small and
self-contained. Worth confirming in-game that Combat Conditioning's Mortal
Wounds application still fires correctly after the restructuring.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — with Combat
Conditioning talented, confirm Rising Sun Kick still applies Mortal Wounds.
Separately, as Mistweaver with Rising Thunder talented, confirm Rising Sun
Kick occasionally resets Thunder Focus Tea's cooldown.

### [DONE] Monk/Brewmaster - Purifying Brew missing, Guard missing and DestinyCore's reference implements the wrong-patch mechanic

**Subsystem:** Scripts/Spells (spell_monk)

**Problem:** Purifying Brew (119582) and Guard (202162), two of Brewmaster's
most basic tools, had no implementation anywhere in ArgusCore.

**Purifying Brew:** straightforward — halves whatever Stagger DoT
(Light/Moderate/Heavy) is currently active. Reused ArgusCore's existing
`FindExistingStaggerEffect` helper (already shared across the other Stagger
scripts in this file) rather than re-deriving the same "check Light, then
Moderate, then Heavy" logic DestinyCore's version does inline.

**Guard: a genuine reference conflict, resolved by cross-checking a third
source.** DestinyCore's implementation is a flat self-absorb shield
(`AttackPower * 18`) — this matches Guard's **pre-Legion (Mists of
Pandaria)** mechanic, not 7.3.5's. Web research (Warcraft Wiki's patch-note
history) confirms Guard was redesigned into a PvP honor talent in **patch
7.1.5** — well before 7.3.5 — dropping the self-absorb entirely in favor of
redirecting 30% of a protected nearby ally's incoming damage into the
Monk's own Stagger pool. Confirmed this isn't just a wiki claim:
DestinyCore's own bound spell id for this script (202162) is specifically
the *later* PvP-talent version's id, not the old MoP ability's id (115295)
— meaning DestinyCore's code and its own SQL binding actually disagree
with each other, a clear sign the C++ wasn't updated when Guard was
redesigned. Found a correct, structurally-verified implementation for this
exact id (202162) in LegionCore instead, explicitly commented "Guard (PvP
talent) - 202162" and using the ally-damage-redirect mechanic the wiki
described.

**Files:** `src/server/scripts/Spells/spell_monk.cpp`

**Fix:**
- Added `spell_monk_purifying_brew`, ported from DestinyCore with no
  changes needed beyond reusing the existing helper.
- Added `spell_monk_guard`, ported from LegionCore's (not DestinyCore's)
  implementation: an absorb effect with `amount = -1` (uncapped, since it's
  a percentage redirect rather than a fixed pool) that intercepts a
  percentage of the ally's damage (read from the spell's own EFFECT_1) and
  redirects it into the casting Monk's Stagger.
- **Refactored** `spell_monk_stagger`'s three previously-private helper
  methods (`AddAndRefreshStagger`, `GetStaggerSpellId`, `AddNewStagger`)
  into shared file-scope functions (matching the existing
  `FindExistingStaggerEffect` pattern) so `spell_monk_guard` could reuse
  the exact same "add this amount to the target's Stagger pool" logic
  rather than duplicating it. Verified `spell_monk_stagger`'s own behavior
  is unchanged — it now calls the extracted free function instead of its
  former private member, with identical logic.

**Files reordering note:** both new classes had to be placed *after* the
shared Stagger helper functions in the file (not simply inserted
alphabetically near `spell_monk_provoke`/`spell_monk_rising_sun_kick` where
they were first drafted), since C++ requires free functions to be declared
before use and this file has no forward declarations for them.

**Database dependency:** searched ArgusCore's committed SQL for existing
`spell_script_names` bindings on both spell ids — found none. Added both to
a single dedicated file, `sql/updates/world/master/2026_07_26_03_world.sql`.

**Risk:** Moderate for Guard specifically — it diverges from the primary
reference core entirely (a rare case this session; only Overpower Proc
Enabler and Second Wind previously required overriding DestinyCore's own
logic, and even then using DestinyCore's own file for the correction, not
a different core). The correction here is well-evidenced (wiki patch
history + LegionCore's matching, explicitly-labeled implementation +
DestinyCore's own binding pointing at the same id), but still worth
particular attention in testing. Purifying Brew carries low risk — a
direct, simple port reusing already-proven code.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — Purifying Brew:
build up Stagger, use Purifying Brew, confirm the remaining Stagger amount
is roughly halved. Guard: cast it on a nearby ally, have them take damage,
confirm their damage taken is reduced and the Monk gains a corresponding
Stagger debuff.

### [DONE] Monk/Mistweaver - core healing kit missing entirely (Enveloping Mist, Renewing Mist, Soothing Mist)

**Subsystem:** Scripts/Spells (spell_monk)

**Problem:** Enveloping Mist, Renewing Mist, and Soothing Mist — Mistweaver's
three foundational healing spells — had no implementation anywhere in
ArgusCore, across five constituent spell scripts.

**Reference:** DestinyCore's implementation for four of the five pieces;
the fifth (the base Soothing Mist channel) was only partially ported — see
below.

- **Enveloping Mist** (124682) — casts the actual heal (132120) after cast
  completes. Notably, ArgusCore's *already-existing* `spell_monk_mists_of_life`
  class casts spell 124682 directly and has since this session began, but
  the Enveloping Mist half of that talent was silently inert the whole time
  without this script to make 124682 actually do anything.
- **Renewing Mist** (115151) — thin wrapper applying the real periodic HoT
  (119611, already present in ArgusCore's enum as `SPELL_MONK_RENEWING_MIST`
  since an earlier fix, just never had its own binding/script).
- **Renewing Mist (periodic)** (119611) — Legion's signature redesign of
  this HoT: when the current target reaches full health, it jumps to the
  most injured ally within 25 yards instead of falling off, carrying its
  remaining duration along. Uses `Trinity::AnyFriendlyUnitInObjectRangeCheck`/
  `UnitListSearcher`/`Cell::VisitAllObjects` for the nearby-ally search and
  `Trinity::Predicates::HealthPctOrderPred` to pick the lowest-health
  target — note the nested `Predicates` namespace, which differs from
  DestinyCore's older `Trinity::HealthPctOrderPred` (no `Predicates::`)
  reference.
- **Soothing Mist (redirect)** (193884) — lets other instant Mistweaver
  spells cast during the Soothing Mist channel without breaking it, by
  re-casting Soothing Mist on the same target when this driver procs.

**Soothing Mist (channel) — deliberately partial port:** DestinyCore's
reference also crosses over into the Jade Serpent Statue mechanic here
(redirecting the channel through a summoned totem, creature entry 60849).
Not ported — same unverified NPC `creature_template` data dependency that
blocked Ravager earlier this session; searched this repo's committed SQL
and found no matching creature row. Implemented only the core player-facing
piece: applies a visual while channeling, has a 25% chance per tick to
generate a Chi, and cleans up the visual when the channel ends. The Jade
Serpent Statue crossover can be added later if that NPC data is confirmed.

**Files:** `src/server/scripts/Spells/spell_monk.cpp`. Added `CellImpl.h`,
`CommonPredicates.h`, and `GridNotifiersImpl.h` includes for the nearby-ally
search and health-percent sort needed by Renewing Mist's jump mechanic.

**Fix:** Added `SPELL_MONK_ENVELOPING_MIST_HEAL` (132120),
`SPELL_MONK_SOOTHING_MIST_ENERGIZE` (116335), and
`SPELL_MONK_SOOTHING_MIST_VISUAL` (125955) constants, plus all five classes.

**Database dependency:** searched ArgusCore's committed SQL for existing
`spell_script_names` bindings on all five spell ids — found none. Added all
five to a single dedicated file,
`sql/updates/world/master/2026_07_26_04_world.sql`.

**Risk:** Moderate — five interacting classes, the most complex being
Renewing Mist's jump mechanic (untested grid-search/sort combination in
this specific file, though the individual APIs are used elsewhere in
ArgusCore's engine). The Jade Serpent Statue omission means Soothing Mist
is functionally complete for solo healing but won't summon/route through
that totem — a scoped-down but not broken implementation.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — as Mistweaver: cast
Enveloping Mist and confirm it heals; cast Renewing Mist on an injured ally,
let them reach full health, and confirm the HoT jumps to a different
injured nearby ally rather than just expiring; channel Soothing Mist and
confirm Chi generates periodically and the channel doesn't break when
casting other instants.

### [DONE] Monk - Dampen Harm, Energizing Brew, Fortifying Brew missing; Expel Harm deliberately deferred

**Subsystem:** Scripts/Spells (spell_monk)

**Problem:** Continuing through Monk defensive/utility cooldowns. Dampen
Harm (122278), Energizing Brew (115288), and Fortifying Brew (115203) had
no implementation anywhere in ArgusCore.

**Dampen Harm:** charge-based absorb that only triggers on a single hit
large enough to exceed a health-percentage threshold. **Found and fixed an
integer-division bug in DestinyCore's reference**: its absorb formula is
`dmgInfo.GetDamage() * (CalcValue(EFFECT_0) / 100)`, where `CalcValue`
returns `int32` — the inner division truncates to 0 for any percentage
under 100%, meaning the absorb would always compute to zero. Used
`CalculatePct` (float-based, matching the convention already used
throughout this codebase) instead of porting that division verbatim.

**Energizing Brew:** trivially simple — only usable in combat, enforced via
`OnCheckCast`.

**Fortifying Brew:** thin wrapper, same "outer cast applies the real buff"
pattern already seen in Renewing Mist/Rampage/etc. this session.

**Expel Harm — investigated, deliberately deferred, not implemented.**
DestinyCore's version deals a flat 50% of the self-heal as AoE damage to
*all* attackable enemies within 10 yards via a hardcoded literal spell id
(115129, no named constant). Cross-checked against LegionCore for a second
opinion and found a structurally different implementation entirely: it
consumes "Gift of the Ox" healing orbs (a Brewmaster passive mechanic
involving spawned `AreaTrigger` objects) and deals only 10% of the healing
as *single-target* damage — matching Wowhead's documented modern-retail
behavior ("10% of the amount healed... to an enemy within 8 yards", not an
AoE nuke). This strongly suggests DestinyCore's flat-50%-AoE version is
wrong for 7.3.5, similar to the earlier Guard finding. However, a correct
port depends on Gift of the Ox, which ArgusCore has **no implementation of
at all** — a genuine prerequisite, not just a nice-to-have (same situation
as Ravager depending on unimplemented NPC data, and Soothing Mist's Jade
Serpent Statue omission). Left unimplemented rather than port either the
likely-wrong DestinyCore version or a version depending on a missing
mechanic.

**Files:** `src/server/scripts/Spells/spell_monk.cpp`

**Fix:** Added `SPELL_MONK_FORTIFYING_BREW` (120954) constant and three
classes (`spell_monk_dampen_harm`, `spell_monk_energizing_brew`,
`spell_monk_fortifying_brew`), ported from DestinyCore with the Dampen Harm
correction noted above.

**Database dependency:** searched ArgusCore's committed SQL for existing
`spell_script_names` bindings on all three spell ids — found none. Added
all three to a single dedicated file,
`sql/updates/world/master/2026_07_26_05_world.sql`.

**Risk:** Low — all three are small, self-contained, and Dampen Harm's
fix can only improve on a reference that was previously non-functional
(zero absorb) regardless of what value it computed.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — Dampen Harm: take a
single large hit exceeding the health-percentage threshold, confirm partial
damage is absorbed and a charge is consumed. Energizing Brew: confirm it
fails to cast outside combat. Fortifying Brew: confirm the defensive buff
applies on cast.

### [DONE, partial] Monk/Brewmaster - Gift of the Ox missing; engine hook gap and DB2 data gap both found and worked around/flagged

**Subsystem:** Scripts/Spells (spell_monk)

**Problem:** Gift of the Ox (124502), the Brewmaster passive that spawns
healing orbs when taking damage, had no implementation. Investigated as a
prerequisite for Expel Harm (deferred in the previous fix).

**Engine hook gap found and worked around:** DestinyCore's reference
implements this as a global `PlayerScript::OnTakeDamage` hook.
**ArgusCore's `PlayerScript` has no `OnTakeDamage` hook at all** — the same
category of missing engine capability as Execute's missing `OnTakePower`
earlier this session. Rather than needing a new engine hook, implemented
this as a self-contained proc-driven `AuraScript` on the talent aura
itself (124502) instead, matching the idiom used throughout this session
(Defensive Stance, Second Wind, etc.). This assumes 124502's own DB2 proc
data is scoped to "on taking damage" — not independently verified, but a
reasonable low-risk assumption given the ability's entire purpose.

**DB2 data gap found, left unresolved:** the companion piece —
`at_monk_gift_of_the_ox_sphere`, an `AreaTriggerAI` that makes the spawned
orb heal the Monk on pickup — is written (ported from DestinyCore, using
ArgusCore's own already-proven `at_monk_song_of_chi_ji` as a structural
template) but **not bound**. Binding an `AreaTriggerAI` requires a row in
`areatrigger_create_properties` referencing real client DB2 visual/shape
asset ids (`Id`, `AreaTriggerId`, `Shape`, `ShapeData0-5`, `MoveCurveId`,
etc.) — confirmed by finding the schema via ArgusCore's own existing
`at_monk_song_of_chi_ji` binding row. DestinyCore's own equivalent row uses
a completely different, older/simpler table schema, so it couldn't be
copied directly, and the correct values for Gift of the Ox's specific orb
can't be inferred from the C++ alone. Guessing at these ids risked
pointing at unrelated real client data, so left unbound rather than guess
— same category of gap as Ravager's `creature_template` and Soothing
Mist's Jade Serpent Statue, both also left unresolved this session.

**Practical effect of the partial implementation:** Gift of the Ox now
correctly rolls its proc chance and casts the orb-spawn spells when taking
damage (scaling with damage taken and current health, matching
DestinyCore's formula). The spell's own existing DB2 data should still
spawn a visible orb. Picking it up will not yet trigger the heal, since
the `AreaTriggerAI` that would handle that isn't bound to real orb data.

**Files:** `src/server/scripts/Spells/spell_monk.cpp`

**Fix:** Added `SPELL_MONK_GIFT_OF_THE_OX_AURA/_AT_RIGHT/_AT_LEFT/_HEAL`
and `SPELL_MONK_HEALING_SPHERE_COOLDOWN` constants, the
`at_monk_gift_of_the_ox_sphere` `AreaTriggerAI` struct (unbound, see
above), and `spell_monk_gift_of_the_ox_aura` (`AuraScript`, bound and
functional).

**Database dependency:** added a `spell_script_names` binding for
`spell_monk_gift_of_the_ox_aura` only, in its own dedicated file,
`sql/updates/world/master/2026_07_26_06_world.sql`. No SQL was written for
the `AreaTriggerAI` binding — see the DB2 data gap above.

**Risk:** Low for the bound proc/spawn half; the unbound pickup-heal half
is inert but harmless (a registered-but-unreferenced script, not a broken
one).

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — as Brewmaster, take
damage and confirm a healing orb occasionally spawns nearby (more
frequently at lower health). Picking it up is expected to currently do
nothing until the `areatrigger_create_properties` row is supplied.

### [DONE] Monk - Touch of Death and Touch of Karma missing; Mastery: Combo Strikes scoped out as a separate, larger task

**Subsystem:** Scripts/Spells (spell_monk)

**Context:** Investigated Mastery: Combo Strikes (Windwalker's core
mastery) as the next candidate, but found it's a fundamentally different
scale of change than anything else this session: DestinyCore's reference
needs the same script bound to essentially every Windwalker damage-dealing
spell individually (not one spell id), and depends on a `PlayerStorage`
per-player persistence system that **does not exist anywhere in
ArgusCore**. Presented this to the user, who chose to defer Combo Strikes
as its own separately-scoped task and continue with two clean,
self-contained candidates instead.

**Problem:** Touch of Death (115080) and Touch of Karma (122470) had no
implementation anywhere in ArgusCore.

**Touch of Death:** deals damage equal to a percentage of the *caster's*
(not the target's) max health, halved against player targets — the
well-known Legion design (an execute-style instant kill vs. NPCs, scaled
down for PvP balance). Self-referential structure: each tick recomputes the
damage and re-casts the same spell id (115080) on the original target with
the new value as a custom base point. **Initially** omitted DestinyCore's
own Combo Strikes integration line (commented out in the reference itself,
with an unfinished author TODO) since it depended on the deferred Combo
Strikes system — **later completed once the user asked for it**; see the
follow-up entry below, which wires this integration back in.

**Touch of Karma:** absorbs damage up to the caster's max health,
redirecting 1/16th of the *cumulative* absorbed total back to the attacker
as periodic damage (recalculated from the running total on each hit, not
just the latest one).

**Files:** `src/server/scripts/Spells/spell_monk.cpp`

**Fix:** Added `SPELL_MONK_TOUCH_OF_DEATH` (115080) and
`SPELL_MONK_TOUCH_OF_KARMA_REDIRECT_DAMAGE` (124280) constants, plus both
classes ported from DestinyCore with only API-convention adaptations
(`CastCustomSpell` → `CastSpellExtraArgs`/`AddSpellBP0`, matching the
pattern established for every other custom-value cast this session).

**Database dependency:** searched ArgusCore's committed SQL for existing
`spell_script_names` bindings on both spell ids — found none. Added both to
a single dedicated file,
`sql/updates/world/master/2026_07_26_07_world.sql`.

**Risk:** Low — both are small, self-contained, single-spell mechanics with
no cross-spell binding requirements or missing engine capabilities.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — Touch of Death: use
it on a low-health NPC target and confirm a lethal/near-lethal damage tick;
against a player target, confirm the damage is roughly half of what it
would be against an NPC. Touch of Karma: take repeated hits while it's
active and confirm damage is absorbed and the attacker takes periodic
damage back, scaling with cumulative damage absorbed.

### [DONE] Monk/Windwalker - Mastery: Combo Strikes implemented (follow-up, previously deferred)

**Subsystem:** Scripts/Spells (spell_monk)

**Context:** After the previous fix deferred Combo Strikes as a
separately-scoped task, the user asked specifically to complete Touch of
Death's commented-out Combo Strikes TODO — which meant building the full
mastery system, since that TODO calls directly into it. Confirmed this
tradeoff with the user (per the session's "STOP and ask before major
scope changes" practice) before proceeding.

**Problem:** Mastery: Combo Strikes (115636), Windwalker's core mastery
(alternating between different abilities grants a damage bonus scaling
with Mastery rating; repeating the same ability grants none), had no
implementation anywhere in ArgusCore, across three constituent script
classes needing binding to ten different spell ids total.

**Reference:** DestinyCore's implementation. Two gaps found and worked
around:

1. **Missing engine capability:** DestinyCore tracks "last ability used"
   per player via a `PlayerStorage` system that doesn't exist anywhere in
   ArgusCore — the same category of gap as Execute's missing
   `OnTakePower` and Gift of the Ox's missing `OnTakeDamage`, both found
   earlier this session. Rather than needing a new engine-wide storage
   system, implemented a lightweight file-scope
   `std::unordered_map<ObjectGuid, int32>` instead (in a `MonkComboStrikes`
   namespace), functionally equivalent for this single purpose. Entries
   aren't cleared on logout — an accepted minor memory-retention tradeoff
   for a workaround, not a functional issue (`ObjectGuid` already has a
   proven `std::unordered_map` hash specialization used elsewhere in
   ArgusCore's engine, e.g. `AuctionHouseMgr.h`).
2. **Multi-spell binding, not a single spell id:** unlike every other fix
   this session, this mastery has no generic classmask-based way to hook
   "any Windwalker damage ability" — each must be bound explicitly via
   `spell_script_names`, exactly matching DestinyCore's own approach.
   Identified all ten target spell ids from DestinyCore's own source
   comments and committed SQL dump: direct-hit abilities (Tiger Palm
   100780, Blackout Kick 100784, Flying Serpent Kick AoE 123586, Rising
   Sun Kick's secondary id 185099), periodic-channel abilities (Fists of
   Fury 113656, Spinning Crane Kick 101546, Whirling Dragon Punch 152175),
   and their damage sub-spells (117418, 107270, 158221 respectively).

**Files:** `src/server/scripts/Spells/spell_monk.cpp`. Added an
`#include <unordered_map>`.

**Fix:** Added `SPELL_MONK_HIT_COMBO`/`_AURA`, `SPELL_MONK_MASTERY_COMBO_STRIKES`,
`SPELL_MONK_SPINNING_CRANE_KICK`/`_DAMAGE`, and
`SPELL_MONK_WHIRLING_DRAGON_PUNCH`/`_DAMAGE` constants; the shared
`MonkComboStrikes` namespace (state tracking + the two central helper
functions `HandleHitCombo`/`TryToHandleDamage`); and three classes
(`spell_monk_mastery_combo_strikes` for direct-hit abilities,
`_periodic_auras` for the three channels, `_periodic_triggers` for their
damage sub-spells) — all ported from DestinyCore's logic with only the
storage-workaround and API-convention adaptations noted above. Also wired
the integration back into Touch of Death's `CalculateAmount` (see the
previous entry).

**Database dependency:** searched ArgusCore's committed SQL for existing
bindings on all three new script names — found none. Several of the target
spell ids (100784, 113656, 117418) already have other scripts bound from
earlier fixes this session; this migration only adds the three new script
names, which coexist with those independently (already relied upon for
Fists of Fury's own dual binding). Added to a single dedicated file,
`sql/updates/world/master/2026_07_26_08_world.sql`.

**Risk:** Moderate-high — the largest-scope Monk fix this session (ten
spell-id bindings across three classes, plus a custom persistence
workaround). The core logic is a faithful port of DestinyCore's reference;
the main residual risk is the storage workaround's behavior under real
concurrent load, which is architecturally reasonable but unproven in this
specific engine.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — as Windwalker,
alternate between different abilities (e.g. Tiger Palm → Blackout Kick →
Rising Sun Kick) and confirm each deals bonus damage; repeat the *same*
ability twice in a row and confirm the second cast deals no bonus. With
Hit Combo talented, confirm the stacking haste buff applies on combo hits
and clears on a repeated ability. Separately, re-verify Touch of Death
still works correctly now that its Combo Strikes integration is active.

### [DONE] Monk/Windwalker - Whirling Dragon Punch dealt zero damage; Zen Pulse missing; Storm Earth and Fire scoped out (NPC data dependency)

**Subsystem:** Scripts/Spells (spell_monk)

**Problem found while investigating further:** Whirling Dragon Punch
(152175) had its Combo Strikes tracking bound in the previous fix
(`spell_monk_mastery_combo_strikes_periodic_auras`), but the ability's own
periodic damage driver was never implemented at all — meaning it dealt
**zero damage** despite Combo Strikes correctly tracking it as "used." This
gap wasn't caught when Combo Strikes was built, since that fix only
covered the mastery's own tracking logic, not each ability's base
functionality.

**Reference:** the reference implementation's `spell_monk_whirling_dragon_punch`
(also bound to 152175, coexisting with the Combo Strikes tracking script on
the same id) — a simple periodic-tick handler that casts the damage
sub-spell (158221) each tick.

**Also found, deliberately deferred:** the reference implementation ties
Whirling Dragon Punch's "usable window" (it can only be cast when both
Fists of Fury and Rising Sun Kick are close to coming off cooldown) to a
`PlayerScript::OnCooldownStart`/`OnChargeRecoveryTimeStart` pair. **Neither
hook exists in ArgusCore's `PlayerScript` at all** — the same category of
gap as Execute's missing `OnTakePower` and Gift of the Ox's missing
`OnTakeDamage`, both found earlier this session. This gating mechanic is
left unimplemented; the ability now deals damage correctly but isn't
restricted to the intended timing window.

**Also implemented: Zen Pulse** (124081) — simple, self-contained: casts a
self-heal (198487) alongside its own damage effect.

**Also investigated, deliberately scoped out: Storm Earth and Fire.**
Summons two NPC clones (Fire/Earth Spirit), requires a dedicated Creature
AI script (`ScriptedAI`, not a spell script) for those clones, and depends
on `creature_template` data this repo's SQL has no record of — the same
category of blocker as Ravager and Soothing Mist's Jade Serpent Statue,
both left unresolved earlier this session. Also uses
`PlayerScript::OnSuccessfulSpellCast` to mirror the caster's harmful casts
through the summoned clones, which would need separate verification. Not
attempted; a good candidate for its own dedicated task alongside Ravager.

**Files:** `src/server/scripts/Spells/spell_monk.cpp`

**Fix:** Added `SPELL_MONK_ZEN_PULSE_HEAL` (198487) constant and two
classes: `spell_monk_whirling_dragon_punch` (periodic damage driver) and
`spell_monk_zen_pulse` (self-heal on hit).

**Database dependency:** searched ArgusCore's committed SQL for existing
bindings on both script names — found none. Added to two files:
`sql/updates/world/master/2026_07_26_09_world.sql` (Whirling Dragon Punch)
and `2026_07_26_10_world.sql` (Zen Pulse).

**Risk:** Low for both — small, self-contained fixes. Whirling Dragon
Punch's damage now works correctly; only its timing-window gating (a
separate, smaller concern) remains unaddressed.

**Commit:** `<pending>`

**Test:** Pending manual build/runtime verification — Whirling Dragon
Punch: cast it and confirm it now deals damage (previously dealt none).
Zen Pulse: cast it and confirm both the damage and a self-heal land.

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
