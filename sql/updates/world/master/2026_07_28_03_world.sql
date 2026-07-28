-- Druid: bind three more newly-implemented Feral abilities found while continuing the class-diff
-- sweep - Swipe/Cat Form (106785), Brutal Slash (202028), Thrash/Cat Form (106830). All three
-- share the same "award exactly one combo point per cast, not per AoE target" pattern; Swipe also
-- has a separate bonus-damage-vs-bleeding-target effect. Corroborated byte-for-byte across both
-- DestinyCore and AshamaneCore. No existing spell_script_names rows for any of these ScriptNames
-- anywhere in this repo's SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_dru_swipe', 'spell_dru_brutal_slash', 'spell_dru_thrash_cat'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(106785, 'spell_dru_swipe'),
(202028, 'spell_dru_brutal_slash'),
(106830, 'spell_dru_thrash_cat');
