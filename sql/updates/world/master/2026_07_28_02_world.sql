-- Druid: bind newly-implemented Shred (5221) - stealth/Incarnation crit and damage bonus, plus a
-- separate bonus against bleeding targets. Corroborated byte-for-byte across both DestinyCore and
-- AshamaneCore. No existing spell_script_names row for this ScriptName anywhere in this repo's
-- SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dru_shred';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(5221, 'spell_dru_shred');
