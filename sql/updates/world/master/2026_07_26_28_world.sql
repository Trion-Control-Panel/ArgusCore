-- Bind the newly-implemented Hunter Survival melee-kit C++ spell scripts to
-- their spell ids: Harpoon, Mongoose Bite, Snake Hunter, Way of the
-- Mok'Nathal. None of these ScriptNames have an existing spell_script_names
-- row anywhere in this repo's SQL.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_hun_harpoon', 'spell_hun_mongoose_bite', 'spell_hun_snake_hunter', 'spell_hun_way_of_the_moknathal'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(190925, 'spell_hun_harpoon'),             -- Harpoon
(190928, 'spell_hun_mongoose_bite'),       -- Mongoose Bite
(201078, 'spell_hun_snake_hunter'),        -- Snake Hunter
(201082, 'spell_hun_way_of_the_moknathal');-- Way of the Mok'Nathal
