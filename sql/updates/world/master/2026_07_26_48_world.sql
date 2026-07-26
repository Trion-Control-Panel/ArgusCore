-- Bind the newly-implemented Death Knight Empower Rune Weapon C++ script to its spell id.
-- No existing spell_script_names row for this ScriptName anywhere in this repo's SQL or the
-- base TDB dump, so without this binding the cooldown reset all runes without the client-side
-- rune display resyncing (or resetting cooldowns at all, in ArgusCore's case).

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dk_empower_rune_weapon';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(47568, 'spell_dk_empower_rune_weapon');
