-- Bind the newly-implemented Hunter Kill Command C++ spell scripts to their
-- spell ids. Neither ScriptName has an existing spell_script_names row
-- anywhere in this repo's SQL, so without this binding Kill Command would
-- do nothing at all when cast (no pet-redirect handler, no damage payload).

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_hun_kill_command', 'spell_hun_kill_command_proc');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(34026, 'spell_hun_kill_command'),      -- Kill Command
(83381, 'spell_hun_kill_command_proc'); -- Kill Command (damage)
