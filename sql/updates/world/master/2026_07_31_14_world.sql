-- Monk: Mana Tea (115294 channel / 123766 "Brewing: Mana Tea" stack-granting passive) -
-- Mistweaver mana-management mechanic, entirely missing from ArgusCore. Confirmed via
-- DestinyCore and AshamaneCore. Reimplemented without the references' raw SpellModifier/
-- flag128 hack and legacy AuraEffect::SetData callback - see in-code comments on
-- spell_monk_mana_tea and spell_monk_mana_tea_stacks for the ArgusCore-native equivalents
-- used instead (Aura::SetDuration, and the generic Chi-spend proc idiom already established
-- for DK's Runic Empowerment/Blood Charge).
DELETE FROM `spell_script_names` WHERE `spell_id` = 115294 AND `ScriptName` = 'spell_monk_mana_tea';
DELETE FROM `spell_script_names` WHERE `spell_id` = 123766 AND `ScriptName` = 'spell_monk_mana_tea_stacks';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(115294, 'spell_monk_mana_tea'),
(123766, 'spell_monk_mana_tea_stacks');
