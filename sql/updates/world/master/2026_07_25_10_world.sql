-- Bind the newly-implemented Warrior Dragon Roar C++ spell script to its spell id.
-- spell_warr_dragon_roar is a brand-new class bound to spell 118000 (confirmed via
-- a reference implementation's own committed spell_script_names data) - no existing
-- spell_script_names row for it anywhere in this repo's SQL, so without this
-- binding Dragon Roar would deal its direct damage but never knock the target back.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warr_dragon_roar';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(118000, 'spell_warr_dragon_roar'); -- Dragon Roar
