-- Warlock: removed Perpetual Unstability (459376/459461) - confirmed The War Within patch 11.0.0
-- (2024-07-23) talent, removed again in patch 12.0.0, no Legion connection. Part of the ongoing
-- Legion 7.3.5 forward-drift removal pass (see ARGUSCORE_FIXES.md).
--
-- Effectively a no-op without the talent aura (guarded by a runtime GetAuraEffect check rather than
-- a Load() override, but the same practical effect) - safe to remove outright. Bound to Unstable
-- Affliction's own cast id (316099, matching its own header comment) rather than its own talent id;
-- Unstable Affliction's other existing scripts in this file are untouched.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warl_perpetual_unstability';
