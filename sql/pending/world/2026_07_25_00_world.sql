-- Bind the newly-implemented Fury Rampage and Arms Sudden Death C++ spell scripts
-- to their spell ids. Both scripts were added in
-- src/server/scripts/Spells/spell_warrior.cpp but have no spell_script_names row
-- right now, so the spell system has no way to invoke them.
--
-- History (already-applied migrations, for context):
--   2025_06_06_03_world.sql deleted the 'spell_warr_sudden_death' binding and
--   never replaced it (the old C++ implementation it pointed to no longer
--   existed).
--   2025_06_07_02_world.sql added (184367, 'spell_warr_rampage_enrage'), then
--   2025_06_08_02_world.sql deleted that same binding again while re-pointing
--   Enrage generation cleanly at (184361, 'spell_warr_enrage_proc') - leaving
--   Rampage (184367) with no binding at all.
--
-- This migration adds fresh bindings for the two scripts implemented this
-- session, using the actual C++ class names (spell_warr_rampage,
-- spell_warr_sudden_death) rather than reusing the old dangling
-- 'spell_warr_rampage_enrage' name, since that name described Enrage
-- application specifically - the current spell_warr_rampage class only
-- handles the Whirlwind-cleave-consumption side; Enrage generation from
-- Rampage remains handled separately by the already-bound spell_warr_enrage_proc.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_warr_rampage', 'spell_warr_sudden_death');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(184367, 'spell_warr_rampage'),   -- Rampage
(52437, 'spell_warr_sudden_death'); -- Sudden Death

-- Bind the newly-implemented Bladestorm C++ spell scripts (Arms/Fury shared ability).
-- Same gap as above: three new classes were added to spell_warrior.cpp
-- (spell_warr_bladestorm, spell_warr_bladestorm_new, spell_warr_bladestorm_offhand)
-- but none of the three spell ids they cover had an existing spell_script_names row
-- anywhere in this repo's SQL, so none of them would fire without this binding.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_warr_bladestorm', 'spell_warr_bladestorm_new', 'spell_warr_bladestorm_offhand');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(227847, 'spell_warr_bladestorm'),        -- Bladestorm
(222634, 'spell_warr_bladestorm_new'),    -- New Bladestorm (periodic driver)
(95738, 'spell_warr_bladestorm_offhand'); -- Bladestorm Offhand
