-- Monk: Disable (116095, baseline snare) - upgrades to a root effect if the target is
-- already snared, and refreshes its own duration on the caster's subsequent melee hits.
-- Entirely missing from ArgusCore. Confirmed via DestinyCore and AshamaneCore (identical
-- implementations). spell_monk_disable (SpellScript) and aura_monk_disable (AuraScript) load
-- together under one ScriptName via RegisterSpellAndAuraScriptPair, so only one row is needed.
DELETE FROM `spell_script_names` WHERE `spell_id` = 116095 AND `ScriptName` = 'spell_monk_disable';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(116095, 'spell_monk_disable');
