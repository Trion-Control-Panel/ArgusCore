-- Bind the newly-implemented Arms Sudden Death C++ spell script to its spell id.
-- spell_warr_sudden_death was added in src/server/scripts/Spells/spell_warrior.cpp
-- but has no spell_script_names row right now, so the spell system has no way
-- to invoke it.
--
-- History (already-applied migrations, for context):
--   2025_06_06_03_world.sql deleted the 'spell_warr_sudden_death' binding and
--   never replaced it (the old C++ implementation it pointed to no longer
--   existed).

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warr_sudden_death';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(52437, 'spell_warr_sudden_death'); -- Sudden Death
