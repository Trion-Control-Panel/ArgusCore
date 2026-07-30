-- Warlock: removed Vile Taint (278350/386931, spell_warl_vile_taint) and the Curse of Exhaustion
-- cast bundled inside it (334275) - confirmed drift, no Legion connection for either. Part of the
-- ongoing Legion 7.3.5 forward-drift removal pass (see ARGUSCORE_FIXES.md).
--
-- Vile Taint was added in patch 8.0.1 (2018-07-17, Battle for Azeroth pre-patch) as an Affliction
-- talent - it did not exist during Legion.
--
-- Curse of Exhaustion was NOT simply a case of "wrong id, needs rebinding to the classic id
-- (18223)" as initially suspected before verifying: web research (warcraft.wiki.gg) confirms it was
-- removed from the game entirely in patch 6.0.2 (2014-10-14, Warlords of Draenor) and stayed gone
-- for the whole of WoD AND Legion, only being re-added in Shadowlands patch 9.0.1 (2020-10-13) as a
-- baseline level-12 ability. There is no point in the Legion 7.3.5 timeline where Curse of
-- Exhaustion existed in any form, so - like Fire Nova earlier this pass - this is a straight
-- removal with nothing to rebind to, not a Doom Winds/Stormflurry-style partial fix.
--
-- SPELL_WARLOCK_AGONY (980), also cast by this class, is genuine ancient Warlock content used
-- elsewhere in the file and is untouched.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warl_vile_taint';
