-- spell_monk_zen_pilgrimage was split into two C++ classes (spell_monk_zen_pilgrimage for
-- 126892 - Zen Pilgrimage, and spell_monk_zen_pilgrimage_return for 126895 - Zen Pilgrimage:
-- Return) to stop a benign-but-noisy startup hook-mismatch warning caused by one shared class
-- registering hooks shaped for two different spells' effect data. Rebind 126895's row to the new
-- scriptname; 126892's row already correctly points at 'spell_monk_zen_pilgrimage' and needs no
-- change.
DELETE FROM `spell_script_names` WHERE `spell_id` = 126895 AND `ScriptName` = 'spell_monk_zen_pilgrimage';
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (126895, 'spell_monk_zen_pilgrimage_return');

-- spell_hun_barrage: bound to 120360 (Barrage, the periodic-trigger container - only 1 real
-- effect, a PERIODIC_TRIGGER_SPELL with no cone-area target at all), but the class's LOS-filtering
-- logic needs a TARGET_UNIT_CONE_ENEMY_24-shaped spell - that's 120361 (the damage tick 120360
-- casts repeatedly, confirmed via SpellEffect data: both of its effects have ImplicitTarget 24).
DELETE FROM `spell_script_names` WHERE `spell_id` = 120360 AND `ScriptName` = 'spell_hun_barrage';
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (120361, 'spell_hun_barrage');
