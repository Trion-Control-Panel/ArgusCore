-- Monk: Elusive Brawler (117906 mastery proc / 195630 stacking dodge buff) - Brewmaster's
-- core mastery mechanic, entirely missing from ArgusCore. Confirmed via DestinyCore and
-- AshamaneCore (identical implementations).
DELETE FROM `spell_script_names` WHERE `spell_id` IN (117906, 195630) AND `ScriptName` IN ('spell_monk_elusive_brawler_mastery', 'spell_monk_elusive_brawler_stacks');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(117906, 'spell_monk_elusive_brawler_mastery'),
(195630, 'spell_monk_elusive_brawler_stacks');
