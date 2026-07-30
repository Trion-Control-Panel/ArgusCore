-- Shaman: removed Primordial Wave (375982/375984) - confirmed Shadowlands 9.0 Necrolord Covenant
-- class ability, later folded into a talent, removed entirely in patch 12.0.0. No Legion
-- connection. Part of the ongoing Legion 7.3.5 forward-drift removal pass (see ARGUSCORE_FIXES.md).
--
-- Fully self-contained: bound to its own headline id (375982, matching this row), so no foreign
-- base-spell rebind confusion this time. Its only shared dependencies (FireNovaTargetCheck,
-- spell_sha_maelstrom_weapon_base::GenerateMaelstromWeapon) are utility code also used by other
-- genuine Legion scripts and are untouched.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_sha_primordial_wave';
