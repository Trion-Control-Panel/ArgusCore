-- Bind two newly-implemented Death Knight C++ scripts to their spell ids: Bonestorm (Blood
-- talent, spends Runic Power to extend duration and heal every tick) and Dark Succor (Unholy
-- passive, gates its free-cast proc to Death Strike). Neither ScriptName has an existing
-- spell_script_names row anywhere in this repo's SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_dk_bonestorm', 'spell_dk_dark_succor');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(194844, 'spell_dk_bonestorm'),   -- Bonestorm
(101568, 'spell_dk_dark_succor'); -- Dark Succor
