-- Bind the newly-implemented Warrior Tactician C++ spell script to its spell id.
-- spell_warr_tactician is a brand-new class bound to spell 184783 - no existing
-- spell_script_names row for it anywhere in this repo's SQL, so without this
-- binding rage-spending abilities would never have a chance to reset Colossus
-- Smash/Mortal Strike's cooldowns.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warr_tactician';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(184783, 'spell_warr_tactician'); -- Tactician
