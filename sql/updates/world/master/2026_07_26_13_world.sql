-- Bind three newly-implemented Monk C++ spell scripts to their spell ids. All
-- three are brand-new classes - no existing spell_script_names row for any of
-- them anywhere in this repo's SQL, so without these bindings Essence Font
-- would heal without redistributing to the neediest target, Healing Elixirs
-- would never proc its self-heal, and Ring of Peace would never silence or
-- disarm anyone.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_monk_essence_font_heal', 'spell_monk_healing_elixirs_aura', 'spell_monk_ring_of_peace_aura');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(191840, 'spell_monk_essence_font_heal'),    -- Essence Font (heal)
(122280, 'spell_monk_healing_elixirs_aura'), -- Healing Elixirs
(140023, 'spell_monk_ring_of_peace_aura');   -- Ring of Peace
