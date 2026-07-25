-- Bind the newly-implemented Warrior Thunder Clap C++ spell script to its spell id.
-- spell_warr_thunder_clap is a brand-new class bound to spell 6343 (Thunder Clap
-- itself) - no existing spell_script_names row for it anywhere in this repo's
-- SQL, so without this binding Thunder Clap would never apply Weakened Blows
-- or the Thunderstruck stun.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warr_thunder_clap';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(6343, 'spell_warr_thunder_clap'); -- Thunder Clap
