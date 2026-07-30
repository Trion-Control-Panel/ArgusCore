-- Warlock: removed spell_warl_siphon_life (452999/453000) - the C++ script implemented a modern
-- (Dragonflight/War Within-era) redesign of Siphon Life's self-heal as an explicit
-- SPELL_AURA_DUMMY proc casting a separate heal spell, bound to a modern id that isn't even the
-- real Siphon Life. Part of the ongoing Legion 7.3.5 forward-drift removal pass (see
-- ARGUSCORE_FIXES.md).
--
-- Siphon Life ITSELF is genuine, ancient Warlock content, moved to a level 60 Affliction talent in
-- patch 7.0.3 (2016-07-19) and present throughout Legion - its real, long-standing spell id is
-- **63106** (confirmed via wowhead and corroborated by AshamaneCore's own
-- SPELL_WARLOCK_SIPHON_LIFE_HEAL/SPELL_WARLOCK_GLYPH_OF_SIPHON_LIFE constants, both = 63106), not
-- 452999/453000 used by the removed class.
--
-- Confirmed via AshamaneCore's genuine Legion-era spell_warlock.cpp: it declares the 63106 constant
-- but never uses it in any scripted class - the self-heal is purely data-driven, baked directly
-- into the DoT's own periodic-leech-style effect in spell data, needing no C++ override at all.
-- Verified no spell_script_names row exists for 63106 in the base TDB dump either, consistent with
-- "no script needed". Removing the modern proc-based reimplementation restores this Legion-correct
-- "no script" state rather than requiring a rebind.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warl_siphon_life';
