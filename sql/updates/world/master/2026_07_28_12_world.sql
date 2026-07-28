-- Paladin: bind newly-implemented Divine Hammer (198034) - grants 2 Holy Power on hit. Also
-- confirmed Light's Hammer already fully implemented under different class names
-- (spell_pal_light_hammer_init_summon/_periodic), using a more modern GetExecuteLogEffectTargets
-- approach instead of the reference's 200-yard creature grid scan - false positive from the
-- initial diff, no change needed. Corroborated across DestinyCore and AshamaneCore. No existing
-- spell_script_names row for this ScriptName anywhere in this repo's SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_pal_divine_hammer';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(198034, 'spell_pal_divine_hammer');
