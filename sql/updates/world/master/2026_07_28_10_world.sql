-- Paladin: bind four more newly-implemented spells found while continuing the class-diff sweep -
-- Bastion of Light (204035, resets Shield of the Righteous charges), Divine Intervention (213313,
-- auto-triggers Divine Shield plus a heal on fatal damage, gated by Forbearance/Divine Shield
-- cooldown), Blade of Wrath's proc half (231832, resets Blade of Justice's cooldown), and Seraphim
-- (152262, consumes Shield of the Righteous charges to extend its own buff duration). All
-- corroborated across DestinyCore and AshamaneCore. Aura of Sacrifice (183416) was investigated
-- and deliberately deferred - even the references had to fabricate a custom, non-Blizzard
-- AreaTrigger id (100102) for it, and no equivalent real data exists in this repo's TDB dump
-- either; left as a documented gap rather than inventing new AreaTrigger infrastructure from
-- scratch. No existing spell_script_names rows for any of the four bound ScriptNames anywhere in
-- this repo's SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_pal_bastion_of_light', 'spell_pal_divine_intervention', 'spell_pal_blade_of_wrath_proc',
    'spell_pal_seraphim'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(204035, 'spell_pal_bastion_of_light'),
(213313, 'spell_pal_divine_intervention'),
(231832, 'spell_pal_blade_of_wrath_proc'),
(152262, 'spell_pal_seraphim');
