-- Bind the newly-implemented Warrior Executioner's Precision C++ spell script to its
-- spell id. spell_warr_executioners_precision is a brand-new class bound to spell
-- 238147 - no existing spell_script_names row for it anywhere in this repo's SQL,
-- so without this binding Execute would never grant the next-Mortal-Strike bonus.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warr_executioners_precision';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(238147, 'spell_warr_executioners_precision'); -- Executioner's Precision
