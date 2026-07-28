-- Paladin: bind two more newly-implemented spells found while continuing the class-diff sweep -
-- Greater Blessing of Kings (203538, a periodically-refilling absorb shield capped at 2.7x spell
-- power - a Legion ability that reuses an old pre-Cataclysm ability name) and Light of the Martyr
-- (183998, heals the target for self-damage equal to 50% of the healing done). Corroborated across
-- DestinyCore and AshamaneCore. Word of Glory was investigated and deliberately skipped - the
-- reference manually adjusts Holy Power via SetPower() after cast, which would very likely
-- double-subtract against ArgusCore's native DB2-driven power cost system; needs the mechanic
-- reworked around the confirmed-working GetPowerTypeCostAmount()-style idiom (as used for Rogue's
-- Kidney Shot and Druid's Maim earlier this session) before it can be safely ported, and several of
-- its talent interactions (Divine Purpose, Fires of Justice) may already be covered by ArgusCore's
-- existing generic proc-based spell_pal_divine_purpose. No existing spell_script_names rows for
-- either bound ScriptName anywhere in this repo's SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_pal_greater_blessing_of_kings', 'spell_pal_light_of_the_martyr');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(203538, 'spell_pal_greater_blessing_of_kings'),
(183998, 'spell_pal_light_of_the_martyr');
