-- Bind the newly-implemented Warrior Revenge Trigger C++ spell script (Protection).
-- spell_warr_revenge_trigger is a brand-new class bound to spell 5301 (the innate
-- trigger passive, distinct from Revenge itself at 6572) - no existing
-- spell_script_names row for it anywhere in this repo's SQL, so without this
-- binding the passive would never reset Revenge's cooldown on proc.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warr_revenge_trigger';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(5301, 'spell_warr_revenge_trigger'); -- Revenge (trigger passive)
