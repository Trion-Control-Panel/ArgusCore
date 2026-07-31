-- Shaman: Fury of Air (197211) - Enhancement talent, periodic AOE wind damage draining 5
-- Maelstrom per tick, entirely missing from ArgusCore. Confirmed via DestinyCore and
-- AshamaneCore (identical implementations).
DELETE FROM `spell_script_names` WHERE `spell_id` = 197211 AND `ScriptName` = 'spell_sha_fury_of_air';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(197211, 'spell_sha_fury_of_air');
