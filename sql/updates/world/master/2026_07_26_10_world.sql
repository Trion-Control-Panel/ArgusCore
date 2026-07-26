-- Bind the newly-implemented Monk Zen Pulse C++ spell script to its spell id.
-- spell_monk_zen_pulse is a brand-new class bound to spell 124081 - no existing
-- spell_script_names row for it anywhere in this repo's SQL, so without this
-- binding Zen Pulse would deal its damage but never heal the caster.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_monk_zen_pulse';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(124081, 'spell_monk_zen_pulse'); -- Zen Pulse
