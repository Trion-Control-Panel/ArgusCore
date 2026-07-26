-- Bind three newly-implemented Monk defensive/utility C++ spell scripts to their
-- spell ids. All three are brand-new classes - no existing spell_script_names
-- row for any of them anywhere in this repo's SQL, so without these bindings
-- Dampen Harm would never absorb damage, Energizing Brew would be usable
-- outside combat, and Fortifying Brew would apply no buff at all.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_monk_dampen_harm', 'spell_monk_energizing_brew', 'spell_monk_fortifying_brew');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(122278, 'spell_monk_dampen_harm'),     -- Dampen Harm
(115288, 'spell_monk_energizing_brew'), -- Energizing Brew
(115203, 'spell_monk_fortifying_brew'); -- Fortifying Brew
