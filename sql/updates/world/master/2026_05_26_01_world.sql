-- spell_script_names: bind missing Monk spell scripts

-- Fix: rising_sun_kick was bound to spell 185099 (wrong ID).
-- spell_monk_rising_sun_kick fires on 107428 (main RSK cast, Combat Conditioning).
DELETE FROM `spell_script_names` WHERE `spell_id` = 185099 AND `ScriptName` = 'spell_monk_rising_sun_kick';

-- Idempotent cleanup for all scripts being inserted below
DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_monk_crackling_jade_lightning',
    'spell_monk_crackling_jade_lightning_knockback_proc_aura',
    'spell_monk_jade_walk',
    'spell_monk_open_palm_strikes',
    'spell_monk_power_strike_periodic',
    'spell_monk_power_strike_proc',
    'spell_monk_pressure_points',
    'spell_monk_rising_sun_kick',
    'spell_monk_roll_aura',
    'spell_monk_save_them_all',
    'spell_monk_stagger',
    'spell_monk_stagger_damage_aura',
    'spell_monk_stagger_debuff_aura',
    'spell_monk_tigers_lust'
);

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
-- Crackling Jade Lightning channel; procs Chi in Crane Stance
(117952, 'spell_monk_crackling_jade_lightning'),
-- Proc aura on the target: triggers knockback when hit while channeling
(117959, 'spell_monk_crackling_jade_lightning_knockback_proc_aura'),
-- Jade Walk periodic; grants speed buff when out of combat
(450553, 'spell_monk_jade_walk'),
-- Open Palm Strikes: chance-on-hit proc aura (filters by talent EFFECT_1 amount)
(392972, 'spell_monk_open_palm_strikes'),
-- Power Strike periodic aura (121817): grants the proc aura on each tick
(121817, 'spell_monk_power_strike_periodic'),
-- Power Strike proc aura (129914): energizes on next melee hit
(129914, 'spell_monk_power_strike_proc'),
-- Paralysis (115078): Pressure Points talent suppresses the dispel effect
(115078, 'spell_monk_pressure_points'),
-- Rising Sun Kick (107428): applies Mortal Wounds when caster has Combat Conditioning
(107428, 'spell_monk_rising_sun_kick'),
-- Roll forward (107427) and backward (109131) movement auras
(107427, 'spell_monk_roll_aura'),
(109131, 'spell_monk_roll_aura'),
-- Save Them All (389579): heals bonus when target is below threshold
(389579, 'spell_monk_save_them_all'),
-- Stagger absorb aura (115069): splits damage into stagger debuffs
(115069, 'spell_monk_stagger'),
-- Stagger damage ticker (124255): deals stagger pool damage each tick
(124255, 'spell_monk_stagger_damage_aura'),
-- Light/Moderate/Heavy Stagger debuff auras; set tick damage on apply/reapply
(124273, 'spell_monk_stagger_debuff_aura'),
(124274, 'spell_monk_stagger_debuff_aura'),
(124275, 'spell_monk_stagger_debuff_aura'),
-- Tiger's Lust (116841): removes movement-impairing effects on cast
(116841, 'spell_monk_tigers_lust');
