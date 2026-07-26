-- Bind Chi Burst's new C++ scripts.
--
-- Unlike Gift of the Ox's orb (still unbound - that row genuinely doesn't exist in this
-- repo's SQL and would need guessed DB2 asset data), Chi Burst's two AreaTriggers already
-- have real rows shipped in the base world database with correct visual/shape/curve data
-- and an empty ScriptName column, so this only needs to fill that column in, not invent a
-- new row:
--   areatrigger_create_properties.Id 1315 -> AreaTriggerId 5302 (Chi Burst damage half)
--   areatrigger_create_properties.Id 1316 -> AreaTriggerId 5300 (Chi Burst heal half)
--
-- spell_monk_chi_burst_heal (130654) is a brand-new spell_script_names row - no existing
-- binding for it anywhere in this repo's SQL.

UPDATE `areatrigger_create_properties` SET `ScriptName` = 'at_monk_chi_burst_damage' WHERE `Id` = 1315 AND `AreaTriggerId` = 5302;
UPDATE `areatrigger_create_properties` SET `ScriptName` = 'at_monk_chi_burst_heal' WHERE `Id` = 1316 AND `AreaTriggerId` = 5300;

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_monk_chi_burst_heal';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(130654, 'spell_monk_chi_burst_heal'); -- Chi Burst (heal)
