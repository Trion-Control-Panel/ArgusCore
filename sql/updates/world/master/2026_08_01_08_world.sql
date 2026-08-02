-- Monk: spell_monk_crackling_jade_lightning removed (see ARGUSCORE_FIXES.md). Its only job was
-- granting a chi proc while "Stance of the Spirited Crane" was active - that stance (and
-- Mistweaver's whole dual-stance system) doesn't exist in 7.3.5.26972's client data, having
-- been removed from the game before Legion. Crackling Jade Lightning's real periodic damage is
-- handled by the base engine, not this script, so removing the binding is safe.
DELETE FROM `spell_script_names` WHERE `spell_id` = 117952 AND `ScriptName` = 'spell_monk_crackling_jade_lightning';
