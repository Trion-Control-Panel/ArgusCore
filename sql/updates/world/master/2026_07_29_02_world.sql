-- Shaman: Molten Thunder (469344/469346) removed - confirmed The War Within patch 11.0.5
-- (2024-10-22) content, no Legion connection. Part of the ongoing Legion 7.3.5 forward-drift
-- removal pass (see ARGUSCORE_FIXES.md). Cleans up the two dangling spell_script_names rows this
-- removal leaves behind: 469344 pointed at the removed proc-count helper aura, and 197214 (the
-- real Legion Sundering spell id) pointed at the removed Molten-Thunder-specific overlay on
-- Sundering's own cast - Sundering itself needs no script now that this talent-gated interaction
-- is gone.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_sha_molten_thunder', 'spell_sha_molten_thunder_sundering');
