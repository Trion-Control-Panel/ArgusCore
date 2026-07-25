-- Bind the newly-implemented Warrior Second Wind C++ spell scripts to their spell ids.
-- spell_warr_second_wind (29838, the proc passive) and spell_warr_second_wind_heal
-- (202147, the heal-over-time it grants) are brand-new classes - no existing
-- spell_script_names row for either anywhere in this repo's SQL, so without this
-- binding neither would ever fire.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_warr_second_wind', 'spell_warr_second_wind_heal');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(29838, 'spell_warr_second_wind'),      -- Second Wind (proc passive)
(202147, 'spell_warr_second_wind_heal'); -- Second Wind (heal)
