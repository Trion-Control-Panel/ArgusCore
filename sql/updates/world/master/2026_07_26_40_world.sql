-- Bind Soul Cleave's missing damage-scaling script, Fel Devastation's
-- missing damage/heal-calculation scripts, and Fiery Brand's damage
-- reduction script. None of these ScriptNames have an existing
-- spell_script_names row anywhere in this repo's SQL.
--
-- IMPORTANT: this migration does NOT re-bind spell_dh_fel_devastation
-- itself (212084) - that ScriptName already has a row from an earlier
-- migration this session; only its C++ was corrected in place (it now
-- also casts the damage sub-spell each tick, not just the heal). No SQL
-- change is needed for that fix, only for the two new sub-spell scripts
-- below.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_dh_soul_cleave_damage', 'spell_dh_fel_devastation_damage',
    'spell_dh_fel_devastation_heal', 'spell_dh_fiery_brand_absorb'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(228478, 'spell_dh_soul_cleave_damage'),     -- Soul Cleave (damage)
(212105, 'spell_dh_fel_devastation_damage'), -- Fel Devastation (damage)
(212106, 'spell_dh_fel_devastation_heal'),   -- Fel Devastation (heal)
(204022, 'spell_dh_fiery_brand_absorb');     -- Fiery Brand (damage reduction)
