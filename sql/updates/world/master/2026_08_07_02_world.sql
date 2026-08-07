-- spell_dru_inner_peace implemented damage-reduction/knockback-immunity behavior for Tranquility
-- that doesn't exist in this build (Inner Peace here only reduces Tranquility's cooldown).
-- The C++ class has been removed; unbind the stale row.
DELETE FROM `spell_script_names` WHERE `spell_id` = 740 AND `ScriptName` = 'spell_dru_inner_peace';
