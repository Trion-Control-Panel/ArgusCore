-- Bind the newly-implemented Monk Zen Pilgrimage C++ spell script to both of
-- its spell ids. No existing spell_script_names row for this ScriptName
-- anywhere in this repo's SQL, so without this binding neither the outbound
-- teleport to the class order hall nor the return trip would do anything.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_monk_zen_pilgrimage';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(126892, 'spell_monk_zen_pilgrimage'), -- Zen Pilgrimage
(126895, 'spell_monk_zen_pilgrimage'); -- Zen Pilgrimage: Return
