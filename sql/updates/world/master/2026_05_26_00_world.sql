-- spell_script_names: bind ported Legion-era DK and DH spell scripts
-- Source: LegionCore-7.3.5-merged spell_script_names reference

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
    'spell_dh_shatter_soul'
);

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(202443, 'spell_dh_anguish'),
(202446, 'spell_dh_anguish_damage'),
(199552, 'spell_dh_blade_dance'),
(200685, 'spell_dh_blade_dance'),
(210153, 'spell_dh_blade_dance'),
(210155, 'spell_dh_blade_dance'),
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
(237867, 'spell_dh_shatter_soul');
