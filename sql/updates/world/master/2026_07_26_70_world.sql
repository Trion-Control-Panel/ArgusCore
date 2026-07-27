-- Bind the newly-implemented Death Knight Sudden Doom (Unholy passive) C++ script to its spell
-- id. No existing spell_script_names row for this ScriptName anywhere in this repo's SQL or the
-- base TDB dump - id confirmed via patch history (continuously live under 49530 from WotLK
-- through Legion and beyond) rather than a reference cross-check, since neither available
-- reference states its own outer spell id.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dk_sudden_doom';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(49530, 'spell_dk_sudden_doom');
