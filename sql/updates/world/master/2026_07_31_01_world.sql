-- Priest: Focused Will (45243), Void Shift (108968), Spirit Shell (109964/114908) - all
-- entirely missing from ArgusCore. Confirmed genuine Legion 7.3.5 content this session.
--
-- Focused Will is bound to 45243 (the passive/ability id) - distinct from the granted buff
-- id (45242, SPELL_PRIEST_FOCUSED_WILL_BUFF in code), matching the standard WoW
-- ability-id-vs-proc-buff-id split.
--
-- Spirit Shell's heal-redirect is bound to the confirmed core heal spells already known to
-- this file (Heal 2060, Flash Heal 2061, Prayer of Healing 596, Renew 139) - the full
-- intended list of intercepted heals needs Legion 7.3.5 tooltip/DB2 verification before
-- broadening further (see the in-code comment on spell_pri_spirit_shell). 2061 and 139
-- already have an existing spell_pri_atonement_effect row each; this adds an additional,
-- independent ScriptName row for each, which is additive and does not conflict.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (45243, 108968) AND `ScriptName` IN ('spell_pri_focused_will', 'spell_pri_void_shift');
DELETE FROM `spell_script_names` WHERE `spell_id` IN (2060, 2061, 596, 139) AND `ScriptName` = 'spell_pri_spirit_shell';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(45243, 'spell_pri_focused_will'),
(108968, 'spell_pri_void_shift'),
(2060, 'spell_pri_spirit_shell'),
(2061, 'spell_pri_spirit_shell'),
(596, 'spell_pri_spirit_shell'),
(139, 'spell_pri_spirit_shell');
