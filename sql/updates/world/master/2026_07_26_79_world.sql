-- Mage: bind five more newly-implemented spells found while continuing the class-diff sweep
-- (Chilled to the Core, Chrono Shift, Cinderstorm, Conflagration, Enhanced Pyrotechnics). No
-- existing spell_script_names rows for any of these ScriptNames anywhere in this repo's SQL or
-- the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_mage_chilled_to_the_core', 'spell_mage_chrono_shift', 'spell_mage_cinderstorm',
    'spell_mage_conflagration', 'spell_mage_enhanced_pyrotechnics'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(195448, 'spell_mage_chilled_to_the_core'),
(235711, 'spell_mage_chrono_shift'),
(198928, 'spell_mage_cinderstorm'),
(205023, 'spell_mage_conflagration'),
(157642, 'spell_mage_enhanced_pyrotechnics');
