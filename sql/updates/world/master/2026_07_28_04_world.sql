-- Druid: bind newly-implemented Predatory Swiftness (16974 talent + 69369 proc buff) and
-- Bloodtalons (155672). Predatory Swiftness is bound under one shared script name each side:
-- the talent-check half to the four finishers that can trigger it (Maim 22570, Ferocious Bite
-- 22568, Rip 1079, Savage Roar 62071), and the buff-consuming half to the three spells it grants
-- a free cast to (Entangling Roots 339, Rebirth 20484, Regrowth 8936). Bloodtalons binds to
-- Regrowth (8936) alongside Predatory Swiftness's consuming half - spell_script_names supports
-- multiple ScriptNames per spell_id, confirmed via its composite (spell_id, ScriptName) key.
-- A reference bug was NOT ported: both DestinyCore and AshamaneCore's Bloodtalons has an inverted
-- null check (`if (caster) return;` instead of `if (!caster) return;`) that makes the ability's
-- actual logic permanently dead code - implemented with the corrected check instead. No existing
-- spell_script_names rows for any of these ScriptNames anywhere in this repo's SQL or the base
-- TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_dru_predatory_swiftness_maim', 'spell_dru_predatory_swiftness_ferocious_bite',
    'spell_dru_predatory_swiftness_rip', 'spell_dru_predatory_swiftness_savage_roar',
    'spell_dru_predatory_swiftness_aura_entangling_roots', 'spell_dru_predatory_swiftness_aura_rebirth',
    'spell_dru_predatory_swiftness_aura_regrowth', 'spell_dru_bloodtalons'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(22570, 'spell_dru_predatory_swiftness_maim'),
(22568, 'spell_dru_predatory_swiftness_ferocious_bite'),
(1079, 'spell_dru_predatory_swiftness_rip'),
(62071, 'spell_dru_predatory_swiftness_savage_roar'),
(339, 'spell_dru_predatory_swiftness_aura_entangling_roots'),
(20484, 'spell_dru_predatory_swiftness_aura_rebirth'),
(8936, 'spell_dru_predatory_swiftness_aura_regrowth'),
(8936, 'spell_dru_bloodtalons');
