-- Mage: bind four more newly-implemented spells found while continuing the class-diff sweep
-- (Erosion, Erosion's timer half, Frenetic Speed, Ice Floes). No existing spell_script_names
-- rows for any of these ScriptNames anywhere in this repo's SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_mage_erosion', 'spell_mage_erosion_timer', 'spell_mage_frenetic_speed', 'spell_mage_ice_floes'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(205039, 'spell_mage_erosion'),
(210154, 'spell_mage_erosion_timer'),
(236058, 'spell_mage_frenetic_speed'),
(108839, 'spell_mage_ice_floes');
