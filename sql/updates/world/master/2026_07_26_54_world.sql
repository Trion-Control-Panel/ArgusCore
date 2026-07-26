-- Death Knight Defile (152280) was only half-implemented: spell_dk_defile (already bound)
-- only suppresses an unused absorb effect - the actual ground-effect damage/Defile Mastery
-- interaction and the ground-effect debuff application never existed. Binds the new periodic
-- driver script (spell_dk_defile_periodic, a second independent script on the same spell id -
-- see the "multiple scripts per spell_id" pattern already used throughout this repo) and the
-- new AreaTriggerAI (at_dk_defile) that applies/removes the debuff as units cross the ground
-- effect's boundary. The AreaTrigger row itself (id 6212) has an empty ScriptName natively in
-- the base TDB dump, so this also fills that in.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dk_defile_periodic';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(152280, 'spell_dk_defile_periodic');

UPDATE `areatrigger_create_properties` SET `ScriptName` = 'at_dk_defile' WHERE `Id` = 1713 AND `AreaTriggerId` = 6212 AND `IsCustom` = 0;
