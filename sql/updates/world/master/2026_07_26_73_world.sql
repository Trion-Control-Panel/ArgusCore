-- Rogue: remove three confirmed post-Legion (Dragonflight, patch 10.0) forward-drift features,
-- discovered while auditing Rogue's poison system:
--   - Improved Garrote (381632 + two associated buffs, 392401/392403) - a Dragonflight
--     Assassination talent
--   - Improved Shiv (319032, gating a Shiv damage-bonus interaction bolted onto the real,
--     ancient Shiv spell 5938) - also Dragonflight
--   - Amplifying Poison (381664) and Atrophic Poison (381637) - two of the seven "poisons" this
--     repo tracked, both Dragonflight additions to Rogue's poison system (Legion only had
--     Wound/Deadly/Crippling/Numbing); Instant Poison (315584) was also removed from the poison
--     list in the same pass - it existed before Legion, was explicitly removed in the Legion
--     7.0.3 pre-patch, and wasn't reintroduced until Shadowlands, so it was never live during
--     Legion at all despite ArgusCore's copy being a modern (Shadowlands+) id.
-- Instant Poison itself never had a spell_script_names row (it wasn't a separate script, just an
-- array entry), so only the three classes below needed unbinding.
-- See ARGUSCORE_FIXES.md for the full writeup.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_rog_improved_garrote', 'spell_rog_improved_garrote_damage', 'spell_rog_improved_shiv'
);
