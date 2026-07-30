-- Shaman: removed Fire Nova (333974/333977) and its Flame Shock enabler side-hook
-- (466620/466622) - confirmed drift, no Legion connection. Part of the ongoing Legion 7.3.5
-- forward-drift removal pass (see ARGUSCORE_FIXES.md).
--
-- Unlike Doom Winds (previous item this pass), Fire Nova has no Legion-era form to rebind to at
-- all: it was removed from the game entirely in patch 7.0.3 (2016-07-19, the Legion pre-patch)
-- and stayed gone for the whole of Legion AND Battle for Azeroth, only returning in Shadowlands
-- patch 9.0.1 (2020-10-13) as a new Enhancement talent. There is no point in the Legion 7.3.5
-- timeline where this ability existed in any form.
--
-- Both classes were bound to their own headline ids (matching their own header comments, no
-- "bound to a foreign base spell" confusion this time). The shared FireNovaTargetCheck struct they
-- both used is now fully unused (its only other consumer, Primordial Wave, was removed earlier
-- this pass) and was deleted along with them.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_sha_fire_nova', 'spell_sha_flame_shock_fire_nova_enabler');
