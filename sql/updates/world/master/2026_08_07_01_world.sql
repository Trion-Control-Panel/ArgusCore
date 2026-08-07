-- spell_dru_celestial_alignment cast pre-Legion Eclipse spells that don't exist in this build.
-- The C++ class has been removed; unbind the stale rows.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (102560, 194223) AND `ScriptName` = 'spell_dru_celestial_alignment';
