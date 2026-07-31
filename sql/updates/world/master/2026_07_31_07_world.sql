-- Death Knight: Blighted Rune Weapon (195758) - Unholy talent, entirely missing from
-- ArgusCore. Confirmed added patch 7.0.3, removed BFA 8.0.1 (in scope for all of Legion
-- 7.3.5). Simple, low-risk port - identical implementation in both DestinyCore and
-- AshamaneCore, applies the already-existing SPELL_DK_FESTERING_WOUND (194310) on hit.
DELETE FROM `spell_script_names` WHERE `spell_id` = 195758 AND `ScriptName` = 'spell_dk_blighted_rune_weapon';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(195758, 'spell_dk_blighted_rune_weapon');
