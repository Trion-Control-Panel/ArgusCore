-- Mage: Frozen Veins (195345) - Frost artifact trait, entirely missing from ArgusCore.
-- Frostbolt casts reduce Icy Veins' cooldown. Confirmed via DestinyCore and AshamaneCore
-- (identical implementations).
DELETE FROM `spell_script_names` WHERE `spell_id` = 195345 AND `ScriptName` = 'spell_mage_frozen_veins';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(195345, 'spell_mage_frozen_veins');
