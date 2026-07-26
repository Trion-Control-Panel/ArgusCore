-- Bind the newly-implemented Demon Hunter Vengeful Retreat C++ spell
-- script to its spell id. No existing spell_script_names row for this
-- ScriptName anywhere in this repo's SQL, so without this binding the
-- Momentum talent's speed buff would never trigger.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dh_vengeful_retreat';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(198813, 'spell_dh_vengeful_retreat'); -- Vengeful Retreat
