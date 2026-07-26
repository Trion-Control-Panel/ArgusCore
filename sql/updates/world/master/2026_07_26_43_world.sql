-- Bind the newly-implemented Demon Hunter Fel Rush (air/water dash) C++
-- spell+aura pair to its spell id. No existing spell_script_names row
-- for this ScriptName anywhere in this repo's SQL, so without this
-- binding Fel Rush used while airborne/underwater wouldn't set the
-- correct dash speed or clean up gravity/fall/hover state afterward.
--
-- NOTE: RegisterSpellAndAuraScriptPair(spell_1, spell_2) registers both
-- halves of the pair under a single ScriptName - the stringified name of
-- the first (SpellScript) argument only, not two separate names.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dh_fel_rush_dash';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(197923, 'spell_dh_fel_rush_dash'); -- Fel Rush (air/water dash)
