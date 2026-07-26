-- Bind three newly-implemented Monk C++ spell scripts to their spell ids. All
-- three are brand-new classes - no existing spell_script_names row for any of
-- them anywhere in this repo's SQL, so without these bindings Rising Thunder's
-- dedicated proc coverage would be missing (a duplicate-but-harmless overlap
-- with the existing Rising Sun Kick check), and both Surging Mist variants
-- would heal without any of their target-selection logic.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_monk_rising_thunder', 'spell_monk_surging_mist', 'spell_monk_surging_mist_glyphed');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(210804, 'spell_monk_rising_thunder'),        -- Rising Thunder
(116694, 'spell_monk_surging_mist'),          -- Surging Mist
(123273, 'spell_monk_surging_mist_glyphed');  -- Surging Mist (glyphed)
