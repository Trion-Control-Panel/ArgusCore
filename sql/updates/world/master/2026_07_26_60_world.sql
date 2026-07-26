-- Bind the newly-implemented Demon Hunter Illidan's Grasp C++ script to its spell id. No
-- existing spell_script_names row for this ScriptName anywhere in this repo's SQL or the base
-- TDB dump, so without this binding the ability would only ever mark targets and never pull
-- them (the ability's whole point).

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dh_illidans_grasp';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(205630, 'spell_dh_illidans_grasp');
