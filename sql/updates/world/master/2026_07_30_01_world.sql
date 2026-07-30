-- Light of Dawn (85222) / Light of Dawn AOE-heal trigger (185984)
-- Was entirely missing from ArgusCore (a declared-but-never-used constant and nothing else).
-- Confirmed genuine Legion 7.3.5 content via DestinyCore/AshamaneCore corroboration.
-- New implementation uses ArgusCore's own health-pct-sort target-selection idiom (matching
-- spell_pal_holy_prism_selector) instead of the reference cores' legacy RandomResize pick,
-- and reads the heal amount from the spell's own DB2 effect data instead of a hardcoded
-- attack-power multiplier.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (85222, 185984);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(85222, 'spell_pal_light_of_dawn'),
(185984, 'spell_pal_light_of_dawn_trigger');
