-- Bind the newly-implemented Warrior Overpower Proc Enabler C++ spell script to its
-- spell id. spell_warr_overpower_proc is a brand-new class bound to spell 60503 -
-- no existing spell_script_names row for it anywhere in this repo's SQL (the only
-- existing reference to 60503 is an unrelated bulk spell_proc PPM-rate row), so
-- without this binding the passive would never filter which abilities are allowed
-- to enable Overpower.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warr_overpower_proc';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(60503, 'spell_warr_overpower_proc'); -- Overpower Proc Enabler
