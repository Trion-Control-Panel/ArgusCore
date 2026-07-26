-- Bind three newly-implemented Death Knight C++ scripts to their spell ids: Obliterate
-- (Frost's core Killing Machine consumer), Frozen Pulse (Frost's low-rune AoE proc), and
-- Death Siphon (Unholy talent, heals for a portion of its damage). None of these ScriptNames
-- have an existing spell_script_names row anywhere in this repo's SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_dk_obliterate', 'spell_dk_frozen_pulse', 'spell_dk_death_siphon'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(49020, 'spell_dk_obliterate'),    -- Obliterate
(194909, 'spell_dk_frozen_pulse'), -- Frozen Pulse
(108196, 'spell_dk_death_siphon'); -- Death Siphon
