-- Bind two newly-implemented Death Knight C++ scripts to their spell ids: Outbreak's AoE
-- spread (196780, a separate effect-carrying spell that correctly spreads Virulent Plague to
-- nearby enemies - the clean version of the mechanic left unported on spell_dk_outbreak/77575
-- due to a targeting bug in the only reference available at the time) and Asphyxiate's stun
-- (108194, a separate follow-up spell rather than part of Asphyxiate's own effect list).
-- Neither ScriptName has an existing spell_script_names row anywhere in this repo's SQL or the
-- base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_dk_outbreak_aoe_dummy', 'spell_dk_asphyxiate');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(196780, 'spell_dk_outbreak_aoe_dummy'), -- Outbreak (AoE spread)
(108194, 'spell_dk_asphyxiate');         -- Asphyxiate
