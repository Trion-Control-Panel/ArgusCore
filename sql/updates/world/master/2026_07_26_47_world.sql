-- Bind the newly-implemented Death Knight/Unholy disease system C++ scripts to their spell
-- ids: Outbreak (instant Virulent Plague application), Virulent Plague itself (periodic
-- damage + chance to erupt), Virulent Eruption (the AoE burst on eruption/death), and Epidemic
-- (single-target + AoE spender that consumes Virulent Plague). None of these ScriptNames have
-- an existing spell_script_names row anywhere in this repo's SQL or the base TDB dump - without
-- this binding Unholy had no disease system at all.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_dk_outbreak', 'spell_dk_virulent_plague', 'spell_dk_virulent_eruption',
    'spell_dk_epidemic', 'spell_dk_epidemic_aoe'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(77575, 'spell_dk_outbreak'),             -- Outbreak
(191587, 'spell_dk_virulent_plague'),     -- Virulent Plague
(191685, 'spell_dk_virulent_eruption'),   -- Virulent Eruption
(207317, 'spell_dk_epidemic'),            -- Epidemic
(215969, 'spell_dk_epidemic_aoe');        -- Epidemic (AoE payload)
