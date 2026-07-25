-- Bind the newly-implemented Warrior Massacre C++ spell script to its spell id.
-- spell_warr_massacre is a brand-new class bound to spell 206315 - no existing
-- spell_script_names row for it anywhere in this repo's SQL, so without this
-- binding Execute crits would never grant a free next Rampage.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warr_massacre';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(206315, 'spell_warr_massacre'); -- Massacre
