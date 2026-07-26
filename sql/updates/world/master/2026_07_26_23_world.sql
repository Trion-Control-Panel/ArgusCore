-- Bind the newly-implemented Hunter Marksmanship "Hunter's Mark / Vulnerable"
-- kit to its spell ids. None of these ScriptNames have an existing
-- spell_script_names row anywhere in this repo's SQL. Two of them
-- (spell_hun_multi_shot_marking_targets on Multi-Shot, spell_hun_arcane_shot
-- on Arcane Shot) are new, independent scripts bound alongside/instead of
-- existing content - see the C++ comments for details.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_hun_arcane_shot', 'spell_hun_hunters_mark', 'spell_hun_marked_shot',
    'spell_hun_marking_targets', 'spell_hun_multi_shot_marking_targets'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(185358, 'spell_hun_arcane_shot'),               -- Arcane Shot
(185987, 'spell_hun_hunters_mark'),              -- Hunter's Mark
(185901, 'spell_hun_marked_shot'),               -- Marked Shot
(223138, 'spell_hun_marking_targets'),           -- Marking Targets
(2643,   'spell_hun_multi_shot_marking_targets');-- Multi-Shot (Marking Targets tie-in)
