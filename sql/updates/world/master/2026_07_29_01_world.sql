-- Cleanup for the ongoing Legion 7.3.5 forward-drift removal pass (see ARGUSCORE_FIXES.md): several
-- of the removed post-Legion abilities had existing spell_script_names/areatrigger_create_properties
-- rows in the base TDB dump pointing at C++ classes that no longer exist now that the drift has been
-- removed from spell_paladin.cpp and spell_shaman.cpp. These rows are harmless at runtime (the engine
-- just logs a "script not found" warning and the spell/areatrigger runs with no script), but are stale
-- references that should be cleaned up alongside the code removal rather than left dangling.
--
-- Paladin - Blade of Vengeance (removed, Dragonflight 10.0.7): both of its ScriptName rows pointed at
-- classes bound to Blade of Justice's own effects (184575, 404358) - Blade of Justice itself needs no
-- script now that the Blade of Vengeance interaction is gone.
-- Paladin - Ashen Hallow (removed, Shadowlands covenant ability): its AreaTrigger (Id 19042,
-- AreaTriggerId 23072) had ScriptName 'areatrigger_pal_ashen_hallow' set directly in the base dump;
-- cleared rather than deleting the row itself (the row describes real DB2-referenced AreaTrigger
-- shape data unrelated to the drift, only the ScriptName column needs clearing).
--
-- Shaman - Arctic Snowstorm (removed, The War Within 11.0): its cast-proc ScriptName row (462764) and
-- its AreaTrigger's ScriptName (Id 34472, AreaTriggerId 36797) both need clearing.
-- Shaman - Voltaic Blaze (removed, The War Within 11.0.5): all three of its ScriptName rows (470053,
-- 470057, 470058).

DELETE FROM `spell_script_names` WHERE `ScriptName` IN
(
    'spell_pal_blade_of_vengeance',
    'spell_pal_blade_of_vengeance_aoe_target_selector',
    'spell_sha_arctic_snowstorm',
    'spell_sha_voltaic_blaze',
    'spell_sha_voltaic_blaze_aura',
    'spell_sha_voltaic_blaze_talent'
);

UPDATE `areatrigger_create_properties` SET `ScriptName` = '' WHERE `Id` = 19042 AND `IsCustom` = 0 AND `ScriptName` = 'areatrigger_pal_ashen_hallow';
UPDATE `areatrigger_create_properties` SET `ScriptName` = '' WHERE `Id` = 34472 AND `IsCustom` = 0 AND `ScriptName` = 'areatrigger_sha_arctic_snowstorm';
