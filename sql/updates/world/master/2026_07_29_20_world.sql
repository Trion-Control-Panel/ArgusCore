-- Warlock: removed Wither (445465/445474) - confirmed The War Within 11.0.0 (2024-07-23) Hellcaller
-- Hero Talent content ("Your Corruption is turned into Wither"), no Legion connection (Hero Talents
-- as a system did not exist until The War Within). Part of the ongoing Legion 7.3.5 forward-drift
-- removal pass (see ARGUSCORE_FIXES.md).
--
-- Two entanglement points, both resolved without touching the genuine Legion content they share
-- code with:
--  1. spell_warl_absolute_corruption (extends a DoT's duration while Absolute Corruption is active)
--     is a generic, id-agnostic class bound to BOTH Corruption (146739, genuine Legion) and Wither
--     (445474, drift) via two separate spell_script_names rows under the same ScriptName. Only the
--     445474 row is removed here - the 146739 row, and the class itself, are untouched.
--  2. spell_warl_soul_fire::HandleTriggers used to branch between casting Wither's periodic or
--     Immolate's periodic based on whether the caster had the Wither talent aura. Restored to
--     always cast Immolate's periodic (SPELL_WARLOCK_IMMOLATE_PERIODIC) unconditionally, matching
--     Soul Fire's genuine Legion-only behavior (Demonology's Soul Fire refreshes/reapplies
--     Immolate) with the Hero-Talent branch removed. No DB change needed for this piece since it
--     was inline logic, not a separate registration.

DELETE FROM `spell_script_names` WHERE `spell_id` = 445474 AND `ScriptName` = 'spell_warl_absolute_corruption';
