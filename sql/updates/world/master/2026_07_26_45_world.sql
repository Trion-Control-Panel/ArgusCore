-- Bind the newly-implemented Demon Hunter Infernal Strike (leap + impact damage) C++
-- scripts to their spell ids. Neither ScriptName has an existing spell_script_names
-- row anywhere in this repo's SQL or the base TDB dump, so without this binding
-- Infernal Strike would have no path validation and no impact damage on landing.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_dh_infernal_strike', 'spell_dh_infernal_strike_jump');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(189110, 'spell_dh_infernal_strike'),       -- Infernal Strike (cast/leap)
(189111, 'spell_dh_infernal_strike_jump');  -- Infernal Strike (jump/impact damage)
