# ArgusCore Fix Backlog

Tracks the incremental security/stability/correctness/blizzlike maintenance pass.
One fix per entry. See repo root workflow conventions for process rules.

Legend: `[ ]` open · `[WIP]` in progress · `[DONE]` shipped

---

## P0 — Security / Corruption

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

(Not populated by this pass — see `ROADMAP.md` / `plans/00-arguscore-analysis-and-roadmap.md`
for the active class-spell-porting and content workstreams, which are tracked
separately from this security/stability backlog.)

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
