-- Bind the newly-implemented Hunter pet special-attack C++ spell scripts
-- to their spell ids: Cobra Spit (pet), Thunderstomp (pet). Neither
-- ScriptName has an existing spell_script_names row anywhere in this
-- repo's SQL.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_hun_pet_cobra_spit', 'spell_hun_pet_thunderstomp');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(206685, 'spell_hun_pet_cobra_spit'),  -- Cobra Spit (pet)
(63900,  'spell_hun_pet_thunderstomp');-- Thunderstomp (pet)
