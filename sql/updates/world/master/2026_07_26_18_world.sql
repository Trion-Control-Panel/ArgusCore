-- Bind the newly-implemented Monk Lifecycles C++ spell script to its spell id.
-- No existing spell_script_names row for this ScriptName anywhere in this
-- repo's SQL, so without this binding alternating Vivify/Enveloping Mist
-- would never grant the mana-cost-reduction buff on the other spell.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_monk_lifecycles';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(197915, 'spell_monk_lifecycles'); -- Lifecycles
