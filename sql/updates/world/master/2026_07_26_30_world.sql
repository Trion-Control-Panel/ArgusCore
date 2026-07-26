-- Bind five newly-implemented Hunter Beast Mastery/Survival C++ spell
-- scripts to their spell ids: Flanking Strike (+ its two pet-damage
-- payloads), Killer Cobra, True Aim. None of these ScriptNames have an
-- existing spell_script_names row anywhere in this repo's SQL.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_hun_flanking_strike', 'spell_hun_flanking_strike_proc', 'spell_hun_flanking_strike_proc_up',
    'spell_hun_killer_cobra', 'spell_hun_true_aim'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(202800, 'spell_hun_flanking_strike'),        -- Flanking Strike
(204740, 'spell_hun_flanking_strike_proc'),   -- Flanking Strike (pet damage)
(206933, 'spell_hun_flanking_strike_proc_up'),-- Flanking Strike (pet damage, +50%)
(199532, 'spell_hun_killer_cobra'),           -- Killer Cobra
(199527, 'spell_hun_true_aim');               -- True Aim
