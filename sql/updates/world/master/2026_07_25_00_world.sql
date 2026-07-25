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

-- Bind four Mage C++ spell scripts added earlier this session, before the
-- spell_script_names binding requirement above was discovered. All four are
-- brand-new class names (never existed under any name before this session),
-- so unlike spell_mage_pyroblast/spell_mage_flamestrike/spell_mage_ice_lance
-- (which were only extended, not renamed, and keep whatever binding already
-- made them fire), these four have had no binding at all since being written
-- and have been silently inert:
--   spell_mage_pyroblast_clearcasting_driver (44448)  - Hot Streak generation
--   spell_mage_combustion                    (190319) - Combustion crit-rating double
--   spell_mage_brain_freeze                  (190447) - Brain Freeze proc generation
--   spell_mage_chain_reaction                (195419) - Chain Reaction stacking

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_mage_pyroblast_clearcasting_driver', 'spell_mage_combustion', 'spell_mage_brain_freeze', 'spell_mage_chain_reaction');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(44448, 'spell_mage_pyroblast_clearcasting_driver'), -- Hot Streak driver
(190319, 'spell_mage_combustion'),                   -- Combustion
(190447, 'spell_mage_brain_freeze'),                 -- Brain Freeze
(195419, 'spell_mage_chain_reaction');                -- Chain Reaction

-- Bind the newly-implemented Warrior Revenge Trigger C++ spell script (Protection).
-- spell_warr_revenge_trigger is a brand-new class bound to spell 5301 (the innate
-- trigger passive, distinct from Revenge itself at 6572) - no existing
-- spell_script_names row for it anywhere in this repo's SQL, so without this
-- binding the passive would never reset Revenge's cooldown on proc.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warr_revenge_trigger';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(5301, 'spell_warr_revenge_trigger'); -- Revenge (trigger passive)
