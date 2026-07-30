-- Warlock: removed Volatile Agony (453034/453035) - confirmed The War Within patch 11.0.0
-- (2024-07-23) talent, no Legion connection. Part of the ongoing Legion 7.3.5 forward-drift
-- removal pass (see ARGUSCORE_FIXES.md).
--
-- Same shape as Perpetual Unstability removed earlier this pass: effectively a no-op without the
-- talent aura (guarded by a runtime GetAuraEffect check rather than a Load() override), bound to
-- Agony's own cast id (980, matching its own header comment) rather than its own talent id. Agony's
-- other existing scripts in this file are untouched.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warl_volatile_agony';
