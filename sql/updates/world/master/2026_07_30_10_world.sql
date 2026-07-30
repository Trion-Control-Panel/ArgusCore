-- Fan of Knives (51723) - Assassination's AoE builder, entirely missing from ArgusCore
-- (not to be confused with Shuriken Storm, Subtlety's separate AoE builder, already implemented).
DELETE FROM `spell_script_names` WHERE `spell_id` = 51723;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(51723, 'spell_rog_fan_of_knives');
