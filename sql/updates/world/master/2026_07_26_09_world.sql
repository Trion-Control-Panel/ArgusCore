-- Bind the newly-implemented Monk Whirling Dragon Punch periodic driver to its
-- spell id. spell_monk_whirling_dragon_punch is a brand-new class bound to spell
-- 152175 - no existing spell_script_names row for this script name anywhere in
-- this repo's SQL, so without this binding Whirling Dragon Punch dealt zero
-- damage despite spell_monk_mastery_combo_strikes_periodic_auras already being
-- bound to the same spell id for combo tracking.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_monk_whirling_dragon_punch';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(152175, 'spell_monk_whirling_dragon_punch'); -- Whirling Dragon Punch
