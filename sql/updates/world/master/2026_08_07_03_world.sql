-- spell_warr_rumbling_earth implemented a Warrior talent added in Battle for Azeroth, after
-- Legion ended. The C++ class has been removed; unbind the stale row.
DELETE FROM `spell_script_names` WHERE `spell_id` = 46968 AND `ScriptName` = 'spell_warr_rumbling_earth';
