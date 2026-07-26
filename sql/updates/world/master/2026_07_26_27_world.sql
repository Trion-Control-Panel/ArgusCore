-- Bind the newly-implemented Hunter Barrage, Beast Cleave, Beast Cleave
-- (proc), and Intimidation C++ spell scripts to their spell ids. None of
-- these ScriptNames have an existing spell_script_names row anywhere in
-- this repo's SQL.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_hun_barrage', 'spell_hun_beast_cleave', 'spell_hun_beast_cleave_proc', 'spell_hun_intimidation'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(120360, 'spell_hun_barrage'),           -- Barrage
(115939, 'spell_hun_beast_cleave'),      -- Beast Cleave (Multi-Shot tie-in)
(118455, 'spell_hun_beast_cleave_proc'), -- Beast Cleave (proc)
(19577,  'spell_hun_intimidation');      -- Intimidation
