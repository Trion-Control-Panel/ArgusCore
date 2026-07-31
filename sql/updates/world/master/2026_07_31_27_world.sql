-- Monk: Storm, Earth, and Fire (137639) - Windwalker signature cooldown, entirely missing
-- from ArgusCore. Confirmed via DestinyCore and AshamaneCore (identical implementations).
-- Binds the SEF spirit clone AI to the Fire (69791) and Earth (69792) spirit creature
-- entries, matching the same creature_template.ScriptName pattern already used for Shaman's
-- Feral Spirit (npc_sha_feral_spirit, entry 29264).
UPDATE `creature_template` SET `ScriptName` = 'npc_monk_sef_spirit' WHERE `entry` IN (69791, 69792);

DELETE FROM `spell_script_names` WHERE `spell_id` = 137639 AND `ScriptName` = 'spell_monk_storm_earth_and_fire';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(137639, 'spell_monk_storm_earth_and_fire');
