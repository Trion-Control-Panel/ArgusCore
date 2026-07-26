-- Bind five newly-implemented Death Knight C++ scripts to their spell ids: Cadaverous Pallor,
-- Chill Streak (+ its damage sub-effect), and Zombie Explosion (three PvP Honor Talents), plus
-- Cold Heart (a Frostmourne artifact trait). None of these ScriptNames have an existing
-- spell_script_names row anywhere in this repo's SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_dk_cadaverous_pallor', 'spell_dk_chill_streak', 'spell_dk_chill_streak_damage',
    'spell_dk_zombie_explosion', 'spell_dk_cold_heart'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(201995, 'spell_dk_cadaverous_pallor'),   -- Cadaverous Pallor (Honor Talent)
(204199, 'spell_dk_chill_streak'),        -- Chill Streak (Honor Talent)
(204167, 'spell_dk_chill_streak_damage'), -- Chill Streak (damage)
(210141, 'spell_dk_zombie_explosion'),    -- Zombie Explosion (Honor Talent)
(248397, 'spell_dk_cold_heart');          -- Cold Heart (artifact trait)
