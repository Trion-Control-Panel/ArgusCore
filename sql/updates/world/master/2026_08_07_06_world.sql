-- Warrior: full spell-script recheck against real Legion 7.3.5 client data.

-- Colossus Smash's script was bound to 262161, a later-expansion remake id (confirmed absent
-- from this build); the real Legion Warbreaker artifact-skin id is 209577.
DELETE FROM `spell_script_names` WHERE `spell_id` = 262161 AND `ScriptName` = 'spell_warr_colossus_smash';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (209577, 'spell_warr_colossus_smash');

-- Dragon Roar and Last Stand were previously found to be fully native (no script needed) and
-- their C++ classes removed, but their spell_script_names rows were never cleaned up.
DELETE FROM `spell_script_names` WHERE `spell_id` = 118000 AND `ScriptName` = 'spell_warr_dragon_roar';
DELETE FROM `spell_script_names` WHERE `spell_id` = 12975 AND `ScriptName` = 'spell_warr_last_stand';

-- 59665 does not exist in this build under any id (confirmed via wago.tools and the local Spell
-- dump) and has no corresponding C++ class anywhere in this repo's history; pure base-dump cruft.
DELETE FROM `spell_script_names` WHERE `spell_id` = 59665 AND `ScriptName` = 'spell_warr_vigilance_redirect_threat';

-- spell_improved_whirlwind modeled a rage-per-target-hit mechanic that doesn't exist for this id
-- in Legion; the real effect (casting the Whirlwind Cleave Aura) is natively data-driven. Class
-- removed.
DELETE FROM `spell_script_names` WHERE `spell_id` = 190411 AND `ScriptName` = 'spell_improved_whirlwind';

-- Strategist (384041) and Critical Thinking (389306) are Dragonflight/later content confirmed
-- absent from this build; classes removed.
DELETE FROM `spell_script_names` WHERE `spell_id` = 384041 AND `ScriptName` = 'spell_warr_strategist';
DELETE FROM `spell_script_names` WHERE `spell_id` = 389306 AND `ScriptName` = 'spell_warr_critical_thinking';

-- Storm Bolts (436162, plural multi-target buff) is confirmed absent from this build; Storm Bolt
-- itself has no AoE target-select effect to fall back from. Class removed.
DELETE FROM `spell_script_names` WHERE `spell_id` = 107570 AND `ScriptName` = 'spell_warr_storm_bolts';
