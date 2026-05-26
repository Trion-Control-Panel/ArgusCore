-- spell_script_names: bind missing DK and DH spell scripts

-- ============================================================
-- Death Knight
-- ============================================================

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_dk_anti_magic_shell_raid',
    'spell_dk_anti_magic_zone',
    'spell_dk_apocalypse',
    'spell_dk_bone_shield',
    'spell_dk_change_duration',
    'spell_dk_consumption',
    'spell_dk_corpse_explosion',
    'spell_dk_corpse_shield',
    'spell_dk_defile',
    'spell_dk_desecrated_ground',
    'spell_dk_festering_wound',
    'spell_dk_frost_shield',
    'spell_dk_frost_strike',
    'spell_dk_hook',
    'spell_dk_necrotic_strike',
    'spell_dk_pillar_of_frost',
    'spell_dk_purgatory',
    'spell_dk_purgatory_absorb',
    'spell_dk_tombstone',
    'spell_dk_will_of_the_necropolis'
);

-- death_gate_teleport covers both the standard gate (53822) and the Acherus Fortress variant (187753)
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dk_death_gate_teleport';

-- gorefiends_grasp sits alongside the existing spell_dk_subduing_grasp entry on 108199
DELETE FROM `spell_script_names` WHERE `spell_id` = 108199 AND `ScriptName` = 'spell_dk_gorefiends_grasp';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(50462,  'spell_dk_anti_magic_shell_raid'),
(50461,  'spell_dk_anti_magic_zone'),
(220143, 'spell_dk_apocalypse'),
(195181, 'spell_dk_bone_shield'),
(207290, 'spell_dk_change_duration'),
(51460,  'spell_dk_change_duration'),
(205223, 'spell_dk_consumption'),
(127344, 'spell_dk_corpse_explosion'),
(207319, 'spell_dk_corpse_shield'),
(152280, 'spell_dk_defile'),
(118009, 'spell_dk_desecrated_ground'),
(53822,  'spell_dk_death_gate_teleport'),
(187753, 'spell_dk_death_gate_teleport'),
(194311, 'spell_dk_festering_wound'),
(207203, 'spell_dk_frost_shield'),
(66196,  'spell_dk_frost_strike'),
(222026, 'spell_dk_frost_strike'),
(108199, 'spell_dk_gorefiends_grasp'),
(212472, 'spell_dk_hook'),
(212528, 'spell_dk_hook'),
(223929, 'spell_dk_necrotic_strike'),
(51271,  'spell_dk_pillar_of_frost'),
(116888, 'spell_dk_purgatory'),
(114556, 'spell_dk_purgatory_absorb'),
(219809, 'spell_dk_tombstone'),
(206967, 'spell_dk_will_of_the_necropolis');

-- ============================================================
-- Demon Hunter
-- ============================================================

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_dh_anguish',
    'spell_dh_anguish_damage',
    'spell_dh_blade_dance',
    'spell_dh_darkness',
    'spell_dh_desperate_instincts',
    'spell_dh_eye_of_leotheras',
    'spell_dh_fel_lance',
    'spell_dh_flaming_soul',
    'spell_dh_fueled_by_pain',
    'spell_dh_mana_break',
    'spell_dh_mana_rift',
    'spell_dh_nemesis',
    'spell_dh_reverse_magic',
    'spell_dh_shatter_soul',
    'spell_dh_shattered_souls',       -- new name (idempotent)
    'spell_dh_metamorphosis_impact',  -- new name (idempotent)
    'spell_dh_blade_dance_damage'     -- new name (idempotent)
);

-- spell_dh_ss was the old script name for Shattered Souls (178940, 204254)
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dh_ss';

-- spell_dh_metamorphosis was the old name bound to 200166; renamed to spell_dh_metamorphosis_impact
DELETE FROM `spell_script_names` WHERE `spell_id` = 200166 AND `ScriptName` = 'spell_dh_metamorphosis';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(202443, 'spell_dh_anguish'),
(202446, 'spell_dh_anguish_damage'),
-- blade_dance fires on the main cast (188499/210152); blade_dance_damage fires on each hit (199552/200685/210153/210155)
(188499, 'spell_dh_blade_dance'),
(210152, 'spell_dh_blade_dance'),
(199552, 'spell_dh_blade_dance_damage'),
(200685, 'spell_dh_blade_dance_damage'),
(210153, 'spell_dh_blade_dance_damage'),
(210155, 'spell_dh_blade_dance_damage'),
(209426, 'spell_dh_darkness'),
(205478, 'spell_dh_desperate_instincts'),
(206650, 'spell_dh_eye_of_leotheras'),
(206966, 'spell_dh_fel_lance'),
(238118, 'spell_dh_flaming_soul'),
(213017, 'spell_dh_fueled_by_pain'),
(203704, 'spell_dh_mana_break'),
(235904, 'spell_dh_mana_rift'),
(206491, 'spell_dh_nemesis'),
(205604, 'spell_dh_reverse_magic'),
(209651, 'spell_dh_shatter_soul'),
(209980, 'spell_dh_shatter_soul'),
(209981, 'spell_dh_shatter_soul'),
(210038, 'spell_dh_shatter_soul'),
(228533, 'spell_dh_shatter_soul'),
(237867, 'spell_dh_shatter_soul'),
(178940, 'spell_dh_shattered_souls'),
(204254, 'spell_dh_shattered_souls'),
(200166, 'spell_dh_metamorphosis_impact');

-- ============================================================
-- Demon Hunter: TC7-native scripts missing from DB
-- ============================================================

-- spell_dh_disable_absorb was the old name for 209258; replaced by spell_dh_last_resort
DELETE FROM `spell_script_names` WHERE `spell_id` = 209258 AND `ScriptName` = 'spell_dh_disable_absorb';

-- spell_dh_fel_rush on 192611 was stale; damage fires via spell_dh_fel_rush_charge on 197922/197923
DELETE FROM `spell_script_names` WHERE `spell_id` = 192611 AND `ScriptName` = 'spell_dh_fel_rush';

-- Idempotent cleanup for all scripts being inserted below
DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_dh_last_resort',
    'spell_dh_first_blood',
    'spell_dh_chaos_strike',
    'spell_dh_essence_break',
    'spell_dh_felblade',
    'spell_dh_army_unto_oneself',
    'spell_dh_felblade_charge',
    'spell_dh_felblade_cooldown_reset_proc',
    'spell_dh_fiery_brand',
    'spell_dh_deflecting_spikes',
    'spell_dh_demon_spikes',
    'spell_dh_vengeful_retreat_damage',
    'spell_dh_glide_timer',
    'spell_dh_collective_anguish',
    'spell_dh_cycle_of_hatred',
    'spell_dh_furious_gaze',
    'spell_dh_fel_devastation',
    'spell_dh_darkglare_boon',
    'spell_dh_collective_anguish_eye_beam',
    'spell_dh_cycle_of_hatred_remove_stacks',
    'spell_dh_cycle_of_hatred_talent',
    'spell_dh_chaotic_transformation',
    'spell_dh_charred_warblades',
    'spell_dh_calcified_spikes_periodic',
    'spell_dh_chaos_theory',
    'spell_dh_chaos_theory_drop_charge',
    'spell_dh_feast_of_souls',
    'spell_dh_fel_flame_fortification',
    'spell_dh_inner_demon',
    'spell_dh_restless_hunter',
    'spell_dh_know_your_enemy',
    'spell_dh_monster_rising',
    'spell_dh_shattered_destiny',
    'spell_dh_sigil_of_chains',
    'spell_dh_student_of_suffering',
    'spell_dh_tactical_retreat',
    'spell_dh_unhindered_assault',
    'spell_dh_soul_furnace',
    'spell_dh_soul_furnace_conduit'
);

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
-- Last Resort (replaces old disable_absorb)
(209258, 'spell_dh_last_resort'),
-- First Blood aura (mandatory for blade_dance* GetScript<spell_dh_first_blood> calls)
(206416, 'spell_dh_first_blood'),
-- Chaos Strike proc aura
(197125, 'spell_dh_chaos_strike'),
-- Essence Break
(258860, 'spell_dh_essence_break'),
-- Felblade chain (felblade → felblade_charge → damage)
(232893, 'spell_dh_felblade'),
(232893, 'spell_dh_army_unto_oneself'),
(213241, 'spell_dh_felblade_charge'),
(203557, 'spell_dh_felblade_cooldown_reset_proc'),
(236167, 'spell_dh_felblade_cooldown_reset_proc'),
-- Fiery Brand
(204021, 'spell_dh_fiery_brand'),
-- Demon Spikes
(203819, 'spell_dh_deflecting_spikes'),
(203720, 'spell_dh_demon_spikes'),
-- Vengeful Retreat damage filter
(198813, 'spell_dh_vengeful_retreat_damage'),
-- Glide duration aura
(197154, 'spell_dh_glide_timer'),
-- Eye Beam multi-script bindings
(198013, 'spell_dh_collective_anguish'),
(198013, 'spell_dh_cycle_of_hatred'),
(198013, 'spell_dh_furious_gaze'),
-- Fel Devastation multi-script bindings
(212084, 'spell_dh_fel_devastation'),
(212084, 'spell_dh_darkglare_boon'),
(212084, 'spell_dh_collective_anguish'),
-- Collective Anguish Eye Beam aura
(391057, 'spell_dh_collective_anguish_eye_beam'),
-- Cycle of Hatred auras
(1214890, 'spell_dh_cycle_of_hatred_remove_stacks'),
(258887,  'spell_dh_cycle_of_hatred_talent'),
-- Chaotic Transformation fires when Metamorphosis (191427) is cast
(191427, 'spell_dh_chaotic_transformation'),
-- Charred Warblades
(213010, 'spell_dh_charred_warblades'),
-- Calcified Spikes (Vengeance PvP talent)
(391171, 'spell_dh_calcified_spikes_periodic'),
-- Chaos Theory (fires on same cast as blade_dance)
(188499, 'spell_dh_chaos_theory'),
(210152, 'spell_dh_chaos_theory'),
(390195, 'spell_dh_chaos_theory_drop_charge'),
-- Feast of Souls fires on Soul Cleave cast
(228477, 'spell_dh_feast_of_souls'),
-- Fel Flame Fortification fires when Immolation Aura (258920) is applied
(258920, 'spell_dh_fel_flame_fortification'),
-- Inner Demon and Restless Hunter both attach to Metamorphosis transform
(162264, 'spell_dh_inner_demon'),
(162264, 'spell_dh_restless_hunter'),
-- Know Your Enemy passive
(388118, 'spell_dh_know_your_enemy'),
-- Monster Rising passive
(452414, 'spell_dh_monster_rising'),
-- Shattered Destiny passive
(388116, 'spell_dh_shattered_destiny'),
-- Sigil of Chains trigger spell
(208673, 'spell_dh_sigil_of_chains'),
-- Student of Suffering fires on Sigil of Flame AoE
(204598, 'spell_dh_student_of_suffering'),
-- Tactical Retreat fires on Vengeful Retreat trigger
(198793, 'spell_dh_tactical_retreat'),
-- Unhindered Assault passive
(444931, 'spell_dh_unhindered_assault'),
-- Soul Furnace stack aura
(391166, 'spell_dh_soul_furnace'),
-- Soul Furnace conduit aura
(339424, 'spell_dh_soul_furnace_conduit');
