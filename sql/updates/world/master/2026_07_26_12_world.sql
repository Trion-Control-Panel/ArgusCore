-- Bind four newly-implemented Monk C++ spell scripts to their spell ids. All
-- four are brand-new classes - no existing spell_script_names row for any of
-- them anywhere in this repo's SQL, so without these bindings Black Ox Brew
-- wouldn't reset Purifying Brew charges, Breath of Fire wouldn't apply its
-- DoT, Flying Serpent Kick wouldn't deal its AoE damage, and Spear Hand
-- Strike wouldn't silence its target.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_monk_black_ox_brew', 'spell_monk_breath_of_fire', 'spell_monk_flying_serpent_kick', 'spell_monk_spear_hand_strike');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(115399, 'spell_monk_black_ox_brew'),        -- Black Ox Brew
(115181, 'spell_monk_breath_of_fire'),       -- Breath of Fire
(115057, 'spell_monk_flying_serpent_kick'),  -- Flying Serpent Kick
(116705, 'spell_monk_spear_hand_strike');    -- Spear Hand Strike
