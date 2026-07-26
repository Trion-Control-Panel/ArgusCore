-- Bind the newly-implemented Warrior Unrivaled Strength C++ spell script to its
-- spell id. spell_warr_unrivaled_strength is a brand-new class bound to spell
-- 200860 - no existing spell_script_names row for it anywhere in this repo's
-- SQL, so without this binding Battle Cry's crit-damage bonus would never apply.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warr_unrivaled_strength';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(200860, 'spell_warr_unrivaled_strength'); -- Unrivaled Strength
