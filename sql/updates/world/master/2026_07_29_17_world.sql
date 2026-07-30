-- Warlock: removed Pyrogenics (387095/387096) - confirmed Dragonflight patch 10.0.0 (2022-10-25)
-- Destruction talent, removed again in patch 12.0.1 (Midnight), no Legion connection. Part of the
-- ongoing Legion 7.3.5 forward-drift removal pass (see ARGUSCORE_FIXES.md).
--
-- Fully self-contained, bound to its own headline id (387095, matching this row).

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warl_pyrogenics';
