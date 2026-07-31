-- Shaman: Stormlash (195255/195222/213307) - Doomhammer artifact trait, entirely missing from
-- ArgusCore. Confirmed genuine Legion content (distinct from the removed WoD-era "Stormlash
-- Totem," the same reused-name-across-eras pattern already confirmed this session for Doom
-- Winds/Stormflurry/Death's Embrace). Confirmed via DestinyCore and AshamaneCore (identical
-- implementations).
DELETE FROM `spell_script_names` WHERE `spell_id` IN (195255, 195222) AND `ScriptName` IN ('spell_sha_stormlash', 'spell_sha_stormlash_buff');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(195255, 'spell_sha_stormlash'),
(195222, 'spell_sha_stormlash_buff');
