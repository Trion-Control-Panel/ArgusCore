-- Warlock: removed Bilescourge Bombers (267211/267212/282248, spell_warl_bilescourge_bombers +
-- at_warl_bilescourge_bombers areatrigger) and Shadow Invocation (422054, spell_warl_shadow_invocation)
-- - confirmed drift, no Legion connection. Part of the ongoing Legion 7.3.5 forward-drift removal
-- pass (see ARGUSCORE_FIXES.md).
--
-- Bilescourge Bombers was added in patch 8.0.1 (2018-07-17, Battle for Azeroth) and removed
-- entirely in patch 12.0.0 (2026-01-20); Shadow Invocation was added in patch 10.2.0 (2023-11-07,
-- Dragonflight), also removed in 12.0.0. Neither existed at any point during Legion. Both talents
-- shared the same BilescourgeBombersEvent helper class (delayed bomb-crash visual/missile), which
-- is why they're removed together in one pass rather than separately.
--
-- Cleanup covers both tables: the shared spell_script_names row for the SpellScript, and the
-- areatrigger_create_properties row (matched by ScriptName, Id 15141 confirming the code's own
-- header comment) for the AreaTriggerAI.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_warl_bilescourge_bombers', 'spell_warl_shadow_invocation');
UPDATE `areatrigger_create_properties` SET `ScriptName` = '' WHERE `Id` = 15141 AND `IsCustom` = 0 AND `ScriptName` = 'at_warl_bilescourge_bombers';
