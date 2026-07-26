-- Bind two newly-implemented Priest C++ scripts to their spell ids: Purify (Discipline's
-- dispel, gated behind having something to dispel, with Purified Resolve/Spiritual Cleansing
-- Honor Talent interactions) and Psychic Scream (breaks the fear early once cumulative damage
-- exceeds 10% of the target's max health). Neither ScriptName has an existing spell_script_names
-- row anywhere in this repo's SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_pri_purify', 'spell_pri_psychic_scream');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(527, 'spell_pri_purify'),          -- Purify
(8122, 'spell_pri_psychic_scream'); -- Psychic Scream
