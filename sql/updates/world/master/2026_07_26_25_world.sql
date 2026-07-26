-- Bind the newly-implemented Hunter Raptor Strike and Carve C++ spell
-- scripts to their spell ids. Neither ScriptName has an existing
-- spell_script_names row anywhere in this repo's SQL, so without these
-- bindings Survival's Serpent Sting-on-melee-hit interaction would never
-- fire from either ability.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_hun_raptor_strike', 'spell_hun_carve');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(186270, 'spell_hun_raptor_strike'), -- Raptor Strike
(187708, 'spell_hun_carve');         -- Carve
