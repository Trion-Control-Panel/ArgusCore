-- Bind the newly-implemented Monk Keg Smash and Legacy of the Emperor C++ spell
-- scripts to their spell ids. Both are brand-new classes - no existing
-- spell_script_names row for either anywhere in this repo's SQL, so without
-- these bindings Keg Smash would deal its direct damage but apply none of its
-- debuffs/Chi generation, and Legacy of the Emperor would apply to no one.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_monk_keg_smash', 'spell_monk_legacy_of_the_emperor');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(121253, 'spell_monk_keg_smash'),             -- Keg Smash
(115921, 'spell_monk_legacy_of_the_emperor'); -- Legacy of the Emperor
