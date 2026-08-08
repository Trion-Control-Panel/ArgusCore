-- Shaman: dedicated pass on the "classes with no spell_script_names row at all" finding from the
-- previous migration's empty-id sweep. Each of the 13 candidates was checked against real
-- SpellEffect.db2 data for this build before binding - not assumed correct just because the id
-- itself is real.

-- Ten classes confirmed correct (EFFECT index + aura/effect type verified to match real fetched
-- data exactly) and bound:
--   108281 - Ancestral Guidance (EFFECT_0 SPELL_AURA_PERIODIC_DUMMY, 500ms period)
--   204288 - Earth Shield (EFFECT_1 SPELL_AURA_DUMMY, matching the class's own health-threshold
--     read off the same effect)
--   170374 - Earthen Rage (Passive) (EFFECT_0 SPELL_AURA_DUMMY)
--   201900 - Hot Hand (EFFECT_0 SPELL_AURA_PROC_TRIGGER_SPELL - already had a prior-session
--     comment confirming this; only the binding was missing)
--   77756  - Lava Surge (EFFECT_0 SPELL_AURA_DUMMY) - needed its companion class bound too, see
--     below
--   187880 - Maelstrom Weapon (EFFECT_0 SPELL_AURA_DUMMY)
--   168534 - Mastery: Elemental Overload (EFFECT_0 SPELL_AURA_DUMMY)
--   201845 - Stormsurge (EFFECT_0 SPELL_AURA_DUMMY)
--   51564  - Tidal Waves (EFFECT_0 SPELL_AURA_DUMMY)
--   200071 - Undulation (EFFECT_0 SPELL_AURA_DUMMY)
--
-- Lava Surge specifically needed a second id: spell_sha_lava_surge (77756, the proc-chance
-- talent) casts SPELL_SHAMAN_LAVA_SURGE (77762) as its buff, but the companion class that
-- actually consumes that buff and resets Lava Burst's cooldown - spell_sha_lava_surge_proc - was
-- ALSO unbound. Bound both together (same lesson as Rogue's Turn the Tables from an earlier pass
-- this session: verify the full chain, not just the first id in a class's own header comment).
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(108281, 'spell_sha_ancestral_guidance'),
(204288, 'spell_sha_earth_shield'),
(170374, 'spell_sha_earthen_rage_passive'),
(201900, 'spell_sha_hot_hand'),
(77756, 'spell_sha_lava_surge'),
(77762, 'spell_sha_lava_surge_proc'),
(187880, 'spell_sha_maelstrom_weapon'),
(168534, 'spell_sha_mastery_elemental_overload'),
(201845, 'spell_sha_stormsurge'),
(51564, 'spell_sha_tidal_waves'),
(200071, 'spell_sha_undulation_passive');

-- spell_proc coverage for three of those ten that didn't already have a row from the earlier
-- server-wide migration (108281, 204288, 170374, 187880, and 51564 already did - not re-inserted,
-- would violate the primary key). Hot Hand (201900) is RPPM-based (SpellProcsPerMinuteID 99) but
-- per this session's earlier Paladin finding, the engine's native ProcBasePPM path in
-- Aura::CalcProcChance overrides a bare row's flat Chance at proc-time regardless, so a bare row
-- is still correct here. Mastery: Elemental Overload (168534) and Undulation (200071) were
-- checked too but have NO native SpellAuraOptions data at all for this build - bare stub rows
-- would do nothing, so they're deliberately NOT included; need individual investigation.
INSERT INTO `spell_proc` (`SpellId`) VALUES
(201900),
(77756),
(201845);

-- Three of the thirteen NOT bound this pass - each investigated in full and found to be
-- structurally broken in ways a binding alone can't fix (don't guess at a code restructure):
--
-- Ascendance (Restoration) (spell_sha_ascendance_restoration, real id 114052): its OnProcHeal
-- hook is registered on EFFECT_8, which doesn't exist on this spell at all (real 114052 only has
-- 2 effects). The real EFFECT_0 is SPELL_AURA_TRANSFORM (the cosmetic Water Ascendant model
-- change) - unrelated to the heal-accumulation mechanic the hook needs. An earlier session had
-- already partially investigated this (comment says "was wrongly bound to EFFECT_6", but the live
-- code shows EFFECT_8, and neither matches real data) and left it explicitly unresolved. Binding
-- now would let the periodic-redistribution half run but with nothing to ever redistribute
-- (silently inert, not a crash) - not worth binding in this half-working state.
--
-- Flametongue Weapon (proc) (spell_sha_flametongue_weapon_aura, real id 160098): hooked to
-- EFFECT_0/SPELL_AURA_DUMMY and manually casts SPELL_SHAMAN_FLAMETONGUE_ATTACK (10444) on proc,
-- but real 160098 has EFFECT_0 as a native SPELL_AURA_MOD_DAMAGE_DONE (a passive %-damage stat
-- buff needing no script at all) and EFFECT_1 as SPELL_AURA_PROC_TRIGGER_SPELL, natively firing
-- 160099 (not 10444 - which traces to a completely different parent spell, 193796, an
-- older-era/unrelated "Flametongue Attack"). Needs a code fix (retarget the hook to EFFECT_1 and
-- the cast to 160099, or remove the class entirely since PROC_TRIGGER_SPELL may already be fully
-- native), not a binding.
--
-- Icefury (spell_sha_icefury, real id 210714): the hook itself is already correct (EFFECT_2
-- SPELL_AURA_ADD_PCT_MODIFIER, fixed and commented by an earlier session), but Validate() also
-- requires SPELL_SHAMAN_FROST_SHOCK_ENERGIZE (289439), which is confirmed absent from this build
-- under any id (see the existing FIXME on that constant) - Validate() will always fail regardless
-- of binding, so binding this now would have zero effect until that separate dependency is
-- resolved.
