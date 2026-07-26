-- Bind the newly-implemented Demon Hunter Metamorphosis C++ spell scripts
-- to their spell ids. Neither ScriptName has an existing spell_script_names
-- row anywhere in this repo's SQL, so without these bindings the class's
-- signature cooldown would do nothing but apply its transform buff (from
-- the spell's own DB2 data) - no leap, no Demon Reborn interaction.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_dh_metamorphosis', 'spell_dh_metamorphosis_immunity');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(191427, 'spell_dh_metamorphosis'),          -- Metamorphosis (leap)
(201453, 'spell_dh_metamorphosis_immunity'); -- Metamorphosis (temporary immunity while leaping)
