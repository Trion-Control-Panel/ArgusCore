-- Rogue: full recheck against real Legion 7.3.5 client data.

-- Shuriken Tornado (spell_rog_shuriken_tornado, bound to 277925) removed from spell_rogue.cpp:
-- confirmed via web search to be Battle for Azeroth content, added in patch 8.0.1 - didn't exist
-- yet in Legion 7.3.5. Id confirmed completely absent from this build. This one was live: an
-- earlier migration (2025_05_25_04_world.sql) bound it, so the (real, correctly-implemented)
-- Shuriken Storm periodic-retrigger mechanic has been silently non-functional since. Unbinding.
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_rog_shuriken_tornado';

-- Premeditation (proc) (spell_rog_premeditation_proc, never bound) removed from spell_rogue.cpp
-- entirely, along with its SPELL_ROGUE_PREMEDITATION_ENERGIZE constant (343170, confirmed absent
-- - same later-expansion-remake pattern as this file's two already-corrected sibling ids,
-- 343160->196979 and 343173->235777). Real Legion Premeditation Aura (235777) doesn't need a
-- separate "energize" spell manually cast via a DUMMY proc at all - its own EFFECT_0 is already a
-- native SPELL_EFFECT_ENERGIZE (2 combo points), confirmed genuinely implemented via
-- Spell::EffectEnergize (not a stub). spell_rog_premeditation (the class that actually casts
-- 235777 when Stealth is applied) was never bound either - binding it now to the real trigger id,
-- 1784 (Stealth), restores the whole mechanic with no scripting changes needed for the
-- combo-point grant itself.
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(1784, 'spell_rog_premeditation');

-- Roll the Bones (spell_rog_roll_the_bones, never bound) rebound from 315508 (confirmed a
-- later-expansion remake id, absent from this build) to the real Legion id 193316 (confirmed via
-- exact tooltip match: "Finishing move that rolls the dice of fate... 1 point: 12 seconds, 2
-- points: 18 seconds..." and matching real SPELL_EFFECT_APPLY_AURA/DUMMY structure at EFFECT_0).
-- The class's own logic doesn't reference the id internally at all (only the stale header
-- comment), so this was a clean rebind with zero code risk.
--
-- Kingsbane (spell_rog_kingsbane, never bound) rebound from 385627 (confirmed a later-expansion
-- remake id, absent - Kingsbane was reworked from a Legion Artifact ability into a standalone
-- talent in a later expansion) to the real Legion Artifact-era id 192759 (confirmed via exact
-- tooltip match, "Kingslayers" Artifact weapon flavor text, and matching real EFFECT_4
-- SPELL_AURA_PROC_TRIGGER_SPELL structure, exactly matching the class's own hook).
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(193316, 'spell_rog_roll_the_bones'),
(192759, 'spell_rog_kingsbane');

-- spell_proc coverage: Kingsbane (192759) gates its DoCheckEffectProc hook behind
-- Aura::GetProcEffectMask, which returns 0 unconditionally without a spell_proc row. Confirmed
-- real native SpellAuraOptions data for this build (flat ProcChance 101, SpellProcsPerMinuteID 0
-- - not RPPM, a plain "always eligible" style native chance), so a bare SpellId-only row is
-- enough to restore function. Roll the Bones doesn't need one - its hook is a plain
-- OnEffectHitTarget, not proc-based.
INSERT IGNORE INTO `spell_proc` (`SpellId`) VALUES
(192759);

-- Dedicated pass on classes with no spell_script_names row at all (same check as the Priest
-- pass): found seven more proc-hooked Rogue classes completely unbound, all core, currently
-- expected-functional Legion abilities - Blade Flurry, Honor Among Thieves, Mastery: Main
-- Gauche, Restless Blades, Ruthlessness, Turn the Tables, Venomous Wounds. Each independently
-- verified: real ids match the class's own header comment/constants exactly, and the coded
-- EFFECT-index + aura/effect type in each hook matches real fetched SpellEffect.db2 data for
-- this build (e.g. Turn the Tables' EFFECT_0/SPELL_AURA_PROC_TRIGGER_SPELL hook matches real
-- aura type 42 at EFFECT_0 exactly; Venomous Wounds' EFFECT_1/SPELL_AURA_DUMMY hook matches real
-- aura type 4 at EFFECT_1).
-- Turn the Tables specifically needed BOTH halves bound to work at all: spell_rog_turn_the_tables
-- (198020) only gates whether its native SPELL_AURA_PROC_TRIGGER_SPELL fires (via the engine's
-- own AuraEffect::HandleProcTriggerSpellAuraProc, same mechanism confirmed earlier this session
-- for Paladin's Blade of Wrath) - actually detecting "no longer stunned" and granting the buff
-- happens in the companion class spell_rog_turn_the_tables_periodic_check, bound to the real
-- EffectTriggerSpell it fires into (198023, confirmed real EFFECT_0 SPELL_AURA_PERIODIC_DUMMY,
-- 100ms period, matching the class's own hook exactly). Missed on the first pass through this
-- migration - caught on a recheck by re-reading the class in full instead of just its Register().
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(13877, 'spell_rog_blade_flurry'),
(198031, 'spell_rog_honor_among_thieves'),
(76806, 'spell_rog_mastery_main_gauche'),
(79096, 'spell_rog_restless_blades'),
(14161, 'spell_rog_ruthlessness'),
(198020, 'spell_rog_turn_the_tables'),
(198023, 'spell_rog_turn_the_tables_periodic_check'),
(79134, 'spell_rog_venomous_wounds');

-- spell_proc coverage for the three of those seven that didn't already have a row from the
-- earlier server-wide migration (13877, 76806, 79096, and 14161 already did - not re-inserted,
-- would violate the primary key). All three confirmed real native SpellAuraOptions data.
INSERT IGNORE INTO `spell_proc` (`SpellId`) VALUES
(198031),
(198020),
(79134);
