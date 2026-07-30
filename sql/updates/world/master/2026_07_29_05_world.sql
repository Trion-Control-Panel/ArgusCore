-- Shaman: removed Stormblast (319930/390287/470466) - confirmed Dragonflight patch 10.0.0
-- (2022-10-25) content, no Legion connection. Part of the ongoing Legion 7.3.5 forward-drift
-- removal pass (see ARGUSCORE_FIXES.md). Before Dragonflight this was just "Rank 2 of Stormbringer"
-- (a passive bonus folded into Stormbringer itself, not its own spell/talent), so there is no
-- Legion-era id for it to be rebound to - it genuinely didn't exist as a separate scriptable
-- ability until this rework.
--
-- All three removed classes were Load()-gated on the Stormblast talent aura and were pure no-ops
-- without it; removing them doesn't touch Stormstrike/Windstrike's own base functionality.
--
-- Cleanup: matched by ScriptName rather than spell id, since a per-id search under-counted -
-- spell_sha_stormblast_damage and spell_sha_stormblast_proc were each bound to several of
-- Stormstrike/Windstrike's own sub-effect ids (32175/32176/115356/115357/115360 for the damage
-- class, 17364/115356 for the proc class) in addition to their own headline id.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_sha_stormblast', 'spell_sha_stormblast_damage', 'spell_sha_stormblast_proc');
