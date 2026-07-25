-- Correct the Warrior Execute spell script binding.
-- The old spell_warr_execute_damage class (bound to spell 260798, a verbatim copy of
-- TrinityCore-master's modern-retail Execute) has been replaced with a new
-- spell_warr_execute class implementing the actual Legion 7.3.5 Execute mechanic
-- (163201, plus its PvP-talent variant 217955): base damage plus scaling bonus
-- damage from up to 10 additional Rage spent, with a 30% rage refund if the
-- target survives.
--
-- Deletes any binding for the old, no-longer-existing class name (in case the
-- external TDB has one this repo's own SQL doesn't show), and adds fresh
-- bindings for the new class under both spell ids.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_warr_execute_damage', 'spell_warr_execute');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(163201, 'spell_warr_execute'), -- Execute (Arms)
(217955, 'spell_warr_execute'); -- Execute (PvP talent variant)
