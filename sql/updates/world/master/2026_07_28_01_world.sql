-- Druid: start of a new full class-diff sweep (Druid/Paladin/Shaman/Warlock had not yet been
-- audited this pass). Two genuinely-missing Feral abilities implemented, corroborated by matching
-- implementations in both DestinyCore and AshamaneCore:
--   - Rake (1822) - stealth/Incarnation: King of the Jungle bonus damage, and a stun that only
--     triggers when used from stealth
--   - Maim (22570) - combo-point-scaled stun duration (same pattern as Rogue's Kidney Shot fixed
--     earlier this session)
-- No existing spell_script_names rows for either ScriptName anywhere in this repo's SQL or the
-- base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_dru_maim', 'spell_dru_rake');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(22570, 'spell_dru_maim'),
(1822, 'spell_dru_rake');
