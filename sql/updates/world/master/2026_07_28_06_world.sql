-- Druid: bind newly-implemented Primal Fury (159286) - a passive proc-gate that only allows its
-- (DB2-driven) bonus combo point award through on a critical strike from Shred, Rake, Swipe (Cat
-- Form), or Moonfire (Cat Form). Corroborated byte-for-byte across both DestinyCore and
-- AshamaneCore. No existing spell_script_names row for this ScriptName anywhere in this repo's SQL
-- or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dru_primal_fury';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(159286, 'spell_dru_primal_fury');
