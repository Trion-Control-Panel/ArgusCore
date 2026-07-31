-- Monk: Expel Harm (115072, baseline level 26) - heals the caster and discharges half the
-- amount healed as damage to nearby attackable enemies. Entirely missing from ArgusCore.
-- Id corrected from 322101 (a much later/modern-retail id) to 115072, confirmed via
-- DestinyCore and AshamaneCore (identical implementations) and matching Mists of
-- Pandaria-through-Legion history.
DELETE FROM `spell_script_names` WHERE `spell_id` = 115072 AND `ScriptName` = 'spell_monk_expel_harm';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(115072, 'spell_monk_expel_harm');
