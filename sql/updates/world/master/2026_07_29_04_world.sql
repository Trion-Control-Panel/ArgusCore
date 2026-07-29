-- Shaman: removed spell_sha_stormweaver - confirmed Dragonflight patch 10.1.0 (2023-05-02) PvP
-- talent content, no Legion connection. Part of the ongoing Legion 7.3.5 forward-drift removal
-- pass (see ARGUSCORE_FIXES.md). This standalone class was a Load()-gated overlay bound to
-- Maelstrom Weapon's own core spell id (187881) - a pure no-op for anyone without the Stormweaver
-- talent, same shape as Molten Thunder's Sundering overlay removed earlier this pass. The
-- Stormweaver PvP talent constants and their remaining references inside
-- spell_sha_maelstrom_weapon_base::GenerateMaelstromWeapon/ConsumeMaelstromWeapon are deliberately
-- left alone for now - that shared function needs its own dedicated rework pass (already flagged
-- separately) before any of its entangled drift can be safely removed piece by piece.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_sha_stormweaver';
