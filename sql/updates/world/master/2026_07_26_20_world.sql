-- Bind the newly-implemented Monk Teachings of the Monastery C++ spell
-- script to its spell id. No existing spell_script_names row for this
-- ScriptName anywhere in this repo's SQL, so without this binding Tiger
-- Palm would never grant the buff (202090) that Blackout Kick's own
-- existing consumption logic (spell_monk_blackout_kick) depends on -
-- the whole talent would silently do nothing.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_monk_teachings_of_the_monastery';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(116645, 'spell_monk_teachings_of_the_monastery'); -- Teachings of the Monastery
