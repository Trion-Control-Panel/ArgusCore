-- Shaman: real Legion Maelstrom Weapon (187880) is a simple proc-energize passive (cast 187890
-- on weapon/Windfury hits), not a 5-stack consumable buff; that apparatus modeled Dragonflight
-- content and has been removed. Hailstorm (210853/210854) was wrongly wired into it - it is
-- actually Frostbrand's (196834) own proc and unrelated to Maelstrom Weapon; rebound accordingly.
-- Swirling Maelstrom (384359, Dragonflight 10.0.0) has no Legion presence and has been removed.
DELETE FROM `spell_script_names` WHERE `spell_id` = 334196 AND `ScriptName` = 'spell_sha_hailstorm';
DELETE FROM `spell_script_names` WHERE `spell_id` = 344179 AND `ScriptName` = 'spell_sha_maelstrom_weapon_proc';
DELETE FROM `spell_script_names` WHERE `spell_id` = 384359 AND `ScriptName` = 'spell_sha_swirling_maelstrom';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (196834, 'spell_sha_hailstorm');
