-- Bind twelve newly-implemented Demon Hunter C++ spell scripts to their
-- spell ids: Annihilation (+ its damage payloads), Blade Turning, Demon
-- Spikes' damage-reduction calc, Demon Blades, Demonic Infusion, Eye
-- Beam's initial-hit damage guard, Fel Eruption, Fel Mastery, Consume
-- Soul (Vengeance interactions), and Metamorphosis's Leech buffs (both
-- specs). None of these ScriptNames have an existing spell_script_names
-- row anywhere in this repo's SQL.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_dh_annihilation', 'spell_dh_annihilation_damage', 'spell_dh_blade_turning',
    'spell_dh_demon_spikes_buff', 'spell_dh_demon_blades', 'spell_dh_demonic_infusion',
    'spell_dh_eye_beam_trigger', 'spell_dh_fel_eruption', 'spell_dh_fel_mastery',
    'spell_dh_consume_soul_vengeance', 'spell_dh_metamorphosis_buffs', 'spell_dh_metamorphosis_buffs_veng'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(201427, 'spell_dh_annihilation'),                 -- Annihilation
(201428, 'spell_dh_annihilation_damage'),          -- Annihilation (mainhand/offhand damage)
(227518, 'spell_dh_annihilation_damage'),          -- Annihilation (mainhand/offhand damage)
(203753, 'spell_dh_blade_turning'),                -- Blade Turning
(203819, 'spell_dh_demon_spikes_buff'),             -- Demon Spikes (damage reduction)
(203555, 'spell_dh_demon_blades'),                 -- Demon Blades
(236189, 'spell_dh_demonic_infusion'),             -- Demonic Infusion
(198030, 'spell_dh_eye_beam_trigger'),             -- Eye Beam (initial cast damage guard)
(211881, 'spell_dh_fel_eruption'),                 -- Fel Eruption
(192939, 'spell_dh_fel_mastery'),                  -- Fel Mastery
(210042, 'spell_dh_consume_soul_vengeance'),       -- Consume Soul (Vengeance)
(203794, 'spell_dh_consume_soul_vengeance'),       -- Consume Soul (Vengeance)
(208015, 'spell_dh_consume_soul_vengeance'),       -- Consume Soul (Vengeance)
(162264, 'spell_dh_metamorphosis_buffs'),          -- Metamorphosis (Havoc buffs)
(187827, 'spell_dh_metamorphosis_buffs_veng');     -- Metamorphosis (Vengeance buffs)
