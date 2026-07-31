-- Warlock/Demonology: Implosion (196277) - commands all active Wild Imp pets to fly to the
-- target and explode. Entirely missing from ArgusCore (both DestinyCore and AshamaneCore have
-- this ability's real logic commented out as dead code referencing an undefined
-- PET_ENTRY_WILD_IMP that never compiled). Implemented fresh from the reference's clear intent
-- plus an independently-confirmed Wild Imp creature entry (55659).
DELETE FROM `spell_script_names` WHERE `spell_id` = 196277 AND `ScriptName` = 'spell_warl_implosion';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(196277, 'spell_warl_implosion');
