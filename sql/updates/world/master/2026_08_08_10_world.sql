-- Shaman: full recheck against real Legion 7.3.5 client data. This file had by far the largest
-- drift of any class this session - 21 empty ids found, spanning multiple full mechanics that
-- were never actually in Legion. None of the affected classes were bound in the DB, so all of
-- this is zero-live-impact cleanup rather than fixing currently-broken behavior, with one
-- exception (Aftershock, rebound and newly bound below).

-- Nature's Guardian (spell_sha_natures_guardian, bound to 30884) removed entirely: confirmed via
-- web search that 30884 itself was removed in patch 7.0.3 - the Legion pre-patch itself - and
-- only reintroduced in Dragonflight (10.0.0), under a new design. Didn't exist during Legion
-- 7.3.5 at all (same pattern as Priest's Void Tendrils, Mage's Alter Time, DK's Death Siphon).
-- 30884 is confirmed completely absent from this build (no Spell record, local dump or live
-- wago.tools query) - its own SPELL_SHAMAN_NATURES_GUARDIAN_COOLDOWN dependency (445698) was also
-- absent, which is what surfaced this while investigating the empty-id list; but 30884 being
-- absent too means the whole class needed removing, not just that one constant.

-- Elemental Weapons (spell_sha_elemental_weapons, bound to 384355, plus its
-- SPELL_SHAMAN_ELEMENTAL_WEAPONS_BUFF constant 408390) removed entirely: confirmed via web search
-- that "Elemental Weapons" as a passive was removed in Mists of Pandaria patch 5.0.4 and only
-- reintroduced in Dragonflight (10.0.0) with a different design (checking "do you have an active
-- weapon imbue" and scaling a shared buff) - didn't exist at any point during Legion.

-- Unlimited Power (spell_sha_unlimited_power, bound to 260895, plus its
-- SPELL_SHAMAN_UNLIMITED_POWER_BUFF constant 272737) removed entirely: both ids confirmed
-- completely absent from this build under any name - no Legion-era Elemental Shaman mechanic
-- matching "stacking buff from consecutive Lava Bursts" was found under any other id either.

-- Ashen Catalyst (spell_sha_ashen_catalyst, bound to 390370) removed entirely: confirmed via web
-- search to be Dragonflight (patch 10.0.0) content.

-- Deeply Rooted Elements (spell_sha_deeply_rooted_elements, bound to 378270) removed entirely:
-- confirmed absent from this build under any id; web search confirms it's current-retail talent
-- tree content (most recently a row-6 class talent), no trace of a Legion-era equivalent.

-- Forceful Winds and Unruly Winds (SPELL_SHAMAN_FORCEFUL_WINDS_TALENT/PROC, 262647/262652, and
-- SPELL_SHAMAN_UNRULY_WINDS, 390288) removed from WindfuryProcEvent::Trigger entirely: both
-- confirmed Dragonflight-era Enhancement Shaman talents (262647 confirmed via an exact id match
-- on a current Dragonflight/current-retail talent database, not just a name coincidence) with no
-- Legion equivalent. Unruly Winds' guard also had a genuine null-pointer dereference bug
-- (`shaman->GetAuraEffect(SPELL_SHAMAN_UNRULY_WINDS, EFFECT_0)` was dereferenced via
-- `unrulyWinds->GetAmount()` with no null check) - dormant only because the id doesn't exist so
-- GetAuraEffect always returned null and the surrounding script was never actually invoked
-- (unbound); removing the whole block resolves this rather than just patching the null check,
-- since the mechanic itself doesn't belong in Legion at all.
DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_sha_natures_guardian', 'spell_sha_elemental_weapons', 'spell_sha_unlimited_power',
    'spell_sha_ashen_catalyst', 'spell_sha_deeply_rooted_elements'
);

-- Aftershock (spell_sha_aftershock) rebound from 273221 (a later-expansion remake id, absent from
-- this build) to the real Legion id 210707 (confirmed via exact tooltip match: "Your spells
-- refund X% of all Maelstrom spent on them", and matching real EFFECT_0 SPELL_AURA_DUMMY
-- structure, BasePoints 30 corroborating the coded roll_chance_i(aurEff->GetAmount()) design -
-- statistically equivalent to a flat 30% refund). SPELL_SHAMAN_AFTERSHOCK_ENERGIZE (210712) was
-- already correctly set to the real companion id, only the class's own binding was wrong. Never
-- bound in the DB before now.
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(210707, 'spell_sha_aftershock');
INSERT IGNORE INTO `spell_proc` (`SpellId`) VALUES
(210707);

-- Left flagged, not fixed this pass (no confident low-risk id found, matching this session's
-- "don't guess" rule; all three classes stay unbound, zero live impact either way):
--
-- Molten Assault (spell_sha_molten_assault, gated on SPELL_SHAMAN_MOLTEN_ASSAULT/334033): the
-- class itself is correctly bound to real Lava Lash (60103) with a verified hook, but its gating
-- talent id doesn't exist under any id, and real Legion's actual Lava-Lash-adjacent talent
-- (Lashing Flames, 238142) is a different mechanic (a damage-amp debuff, not a Flame-Shock
-- spread) - not a confident substitute.
--
-- Windfury Weapon (spell_sha_windfury_weapon, gated on SPELL_SHAMAN_WINDFURY_ENCHANTMENT/334302 -
-- already flagged by an earlier session) and its proc half (spell_sha_windfury_weapon_proc, bound
-- to SPELL_SHAMAN_WINDFURY_AURA/319773, also confirmed absent this pass) both stay unbound. The
-- underlying cast spell (33757, Windfury Weapon) is confirmed real and its own EFFECT_0 hook
-- already correct; only the enchant-application and self-tracking-aura ids are unresolved.
--
-- Flametongue Weapon (spell_sha_flametongue_weapon, gated on
-- SPELL_SHAMAN_FLAMETONGUE_WEAPON_ENCHANT/334294): the class's own cast spell was already
-- rebound in an earlier session (318038 -> confirmed via SPELL_SHAMAN_FLAMETONGUE_WEAPON_AURA's
-- own comment, 160098 is the real aura), but the item-enchant-application id it casts is a
-- separate unresolved id.
--
-- Restorative Mists Initial (SPELL_SHAMAN_RESTORATIVE_MISTS_INITIAL/294020, used only as a
-- same-spell exclusion check inside the already-real spell_sha_restorative_mists/114083): since
-- the id never matches anything, the exclusion silently never applies (not a crash, just an
-- always-false comparison) - no confident replacement id found.
