-- Bind four newly-implemented Monk C++ spell scripts to their spell ids. All four
-- are brand-new classes - no existing spell_script_names row for any of them
-- anywhere in this repo's SQL, so without these bindings Chi Wave would do
-- nothing at all when cast (no dummy-effect handler, no bounce chain, no heal
-- or damage payload).

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_monk_chi_wave', 'spell_monk_chi_wave_damage_missile', 'spell_monk_chi_wave_heal_missile', 'spell_monk_chi_wave_target_selector');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(115098, 'spell_monk_chi_wave'),                 -- Chi Wave (talent)
(132467, 'spell_monk_chi_wave_damage_missile'),  -- Chi Wave (damage missile)
(132464, 'spell_monk_chi_wave_heal_missile'),    -- Chi Wave (heal missile)
(132466, 'spell_monk_chi_wave_target_selector'); -- Chi Wave (target selector)
