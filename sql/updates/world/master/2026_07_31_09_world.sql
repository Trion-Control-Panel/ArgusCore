-- Mage: Ice Block (45438) - the Hypothermia anti-spam debuff (41425) and Glacial Insulation
-- talent interaction (235297, converts Ice Block into Ice Barrier on removal) were both
-- entirely missing. Confirmed via DestinyCore and AshamaneCore (identical implementations).
DELETE FROM `spell_script_names` WHERE `spell_id` = 45438 AND `ScriptName` = 'spell_mage_ice_block';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(45438, 'spell_mage_ice_block');
