-- Bind the newly-implemented Bladestorm C++ spell scripts (Arms/Fury shared ability).
-- Three new classes were added to spell_warrior.cpp (spell_warr_bladestorm,
-- spell_warr_bladestorm_new, spell_warr_bladestorm_offhand) but none of the
-- three spell ids they cover had an existing spell_script_names row anywhere
-- in this repo's SQL, so none of them would fire without this binding.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_warr_bladestorm', 'spell_warr_bladestorm_new', 'spell_warr_bladestorm_offhand');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(227847, 'spell_warr_bladestorm'),        -- Bladestorm
(222634, 'spell_warr_bladestorm_new'),    -- New Bladestorm (periodic driver)
(95738, 'spell_warr_bladestorm_offhand'); -- Bladestorm Offhand
