-- Bind the newly-implemented Demon Hunter Immolation Aura C++ spell
-- script to its spell id. No existing spell_script_names row for this
-- ScriptName anywhere in this repo's SQL. Purely cosmetic (shortens the
-- glow visual to match the buff's real duration) - the periodic damage
-- itself is unaffected either way, since it's handled by the spell's own
-- DB2 data.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dh_immolation_aura';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(258920, 'spell_dh_immolation_aura'); -- Immolation Aura
