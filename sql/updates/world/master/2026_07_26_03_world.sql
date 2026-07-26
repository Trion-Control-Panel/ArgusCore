-- Bind the newly-implemented Monk Purifying Brew and Guard C++ spell scripts to
-- their spell ids. Both are brand-new classes - no existing spell_script_names
-- row for either anywhere in this repo's SQL, so without these bindings
-- Purifying Brew would never reduce active Stagger, and Guard would provide no
-- effect for a protected ally at all.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_monk_purifying_brew', 'spell_monk_guard');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(119582, 'spell_monk_purifying_brew'), -- Purifying Brew
(202162, 'spell_monk_guard');          -- Guard
