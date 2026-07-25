-- Bind the newly-implemented Fury Rampage C++ spell script to its spell id.
-- spell_warr_rampage was added in src/server/scripts/Spells/spell_warrior.cpp
-- but has no spell_script_names row right now, so the spell system has no way
-- to invoke it.
--
-- History (already-applied migrations, for context):
--   2025_06_07_02_world.sql added (184367, 'spell_warr_rampage_enrage'), then
--   2025_06_08_02_world.sql deleted that same binding again while re-pointing
--   Enrage generation cleanly at (184361, 'spell_warr_enrage_proc') - leaving
--   Rampage (184367) with no binding at all.
--
-- Uses the actual C++ class name (spell_warr_rampage) rather than reusing the
-- old dangling 'spell_warr_rampage_enrage' name, since that name described
-- Enrage application specifically - the current spell_warr_rampage class only
-- handles the Whirlwind-cleave-consumption side; Enrage generation from
-- Rampage remains handled separately by the already-bound spell_warr_enrage_proc.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warr_rampage';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(184367, 'spell_warr_rampage'); -- Rampage
