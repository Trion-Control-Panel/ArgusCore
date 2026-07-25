-- Bind the newly-implemented Warrior Frenzy C++ spell script to its spell id.
-- spell_warr_frenzy is a brand-new class bound to spell 206313 - no existing
-- spell_script_names row for it anywhere in this repo's SQL, so without this
-- binding the Frenzy haste buff would never be restricted to Furious Slash only.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warr_frenzy';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(206313, 'spell_warr_frenzy'); -- Frenzy
