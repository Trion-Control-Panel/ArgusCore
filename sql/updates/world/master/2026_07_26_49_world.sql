-- Unbind six Death Knight script names whose C++ classes were removed from spell_dk.cpp for
-- being confirmed post-Legion forward drift, discovered while auditing DK against the reference
-- (DK predates Legion, unlike Demon Hunter, so this file mixes real backward-drift leftovers
-- AND forward-drift content - each of these six was independently cross-checked against the
-- base TDB dump's own spell_script_names table, confirmed as live-bound, and confirmed to have
-- no equivalent under any name in either Legion-era reference codebase available in this repo):
--   - spell_dk_brittle (374504 - Shadowlands-era Frost mechanic)
--   - spell_dk_icy_talons_buff (only gated an unrelated modern talent check on the real,
--     legitimate Icy Talons buff - the buff itself needs no script and is unaffected)
--   - spell_dk_reaper_of_souls (440002 - a modern Hero Talent tree ability)
--   - spell_dk_subduing_grasp (454822, bound to three spell ids: Death Grip, Gorefiend's
--     Grasp, and Abomination Limb - the latter is itself a modern class ability)
--   - spell_dk_voracious (273953 - not present in Legion, confirmed via two independent
--     Legion-era references)
--   - at_dk_death_and_decay (its entire body only handled the Cleaving Strikes/Sanguine Ground
--     forward-drift interactions - Death and Decay's own core damage is unaffected, handled
--     elsewhere)
-- See ARGUSCORE_FIXES.md for the full writeup.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_dk_brittle', 'spell_dk_icy_talons_buff', 'spell_dk_reaper_of_souls',
    'spell_dk_subduing_grasp', 'spell_dk_voracious'
);

UPDATE `areatrigger_create_properties` SET `ScriptName` = '' WHERE `ScriptName` = 'at_dk_death_and_decay';
