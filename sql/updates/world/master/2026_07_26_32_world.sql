-- Unbind 29 Demon Hunter script names whose C++ classes were removed from
-- spell_dh.cpp for being confirmed post-Legion (BfA/Shadowlands/Dragonflight/
-- current-retail) forward-drift content that never existed in Legion 7.3.5.
-- Demon Hunter was introduced IN Legion, so unlike other classes there is no
-- backward-drift risk here - anything not from Legion 7.0-7.3.5 is forward
-- drift by definition. See ARGUSCORE_FIXES.md for the full audit writeup.
--
-- Most of these already had spell_script_names rows from this repo's own
-- prior SQL (2026_05_26_00_world.sql, 2025_05_25_05_world.sql); this removes
-- those bindings so the database no longer references ScriptNames that no
-- longer exist in the compiled binary. Two (the AreaTriggerAI classes) were
-- never bound in this repo's SQL at all - included here defensively in case
-- they're bound elsewhere (e.g. a base TDB dump); harmless no-op otherwise.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_dh_army_unto_oneself', 'spell_dh_calcified_spikes_periodic', 'spell_dh_chaos_theory',
    'spell_dh_chaos_theory_drop_charge', 'spell_dh_chaotic_transformation', 'spell_dh_collective_anguish',
    'spell_dh_collective_anguish_eye_beam', 'spell_dh_cycle_of_hatred', 'spell_dh_cycle_of_hatred_remove_stacks',
    'spell_dh_cycle_of_hatred_talent', 'spell_dh_darkglare_boon', 'spell_dh_deflecting_spikes',
    'spell_dh_essence_break', 'spell_dh_fel_flame_fortification', 'spell_dh_furious_gaze',
    'spell_dh_inner_demon', 'spell_dh_know_your_enemy', 'spell_dh_monster_rising',
    'spell_dh_restless_hunter', 'spell_dh_shattered_destiny', 'spell_dh_soul_furnace',
    'spell_dh_soul_furnace_conduit', 'spell_dh_student_of_suffering', 'spell_dh_tactical_retreat',
    'spell_dh_unhindered_assault', 'spell_dh_vengeful_retreat_damage', 'spell_dh_violent_transformation'
);

UPDATE `areatrigger_create_properties` SET `ScriptName` = '' WHERE `ScriptName` IN ('at_dh_glaive_tempest', 'at_dh_inner_demon');
