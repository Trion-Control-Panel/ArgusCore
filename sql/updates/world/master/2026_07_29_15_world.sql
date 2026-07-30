-- Warlock: removed spell_warl_deaths_embrace, the sub-piece of Death's Embrace that modified
-- Malefic Rapture's damage - confirmed drift, no Legion connection. Part of the ongoing Legion
-- 7.3.5 forward-drift removal pass (see ARGUSCORE_FIXES.md).
--
-- Death's Embrace itself is genuine Legion 7.1.5 (2017-01-10) Affliction content, confirmed via
-- wowhead/wowdb - but its real Legion form (id 234876) increased damage on Agony, Corruption,
-- Unstable Affliction, and Drain Soul, not Malefic Rapture. Malefic Rapture (324536/324540) is a
-- Shadowlands 9.0.1 (2020-10-13) ability that replaced Drain Soul as Affliction's spender/finisher
-- and was removed again in patch 12.0.0 - it has no place in Legion 7.3.5's spell kit at all, so
-- this piece is pure removal with nothing to rebind (Drain Soul, the ability Malefic Rapture
-- replaced, already exists in this codebase as its own spell and isn't affected by this change).
--
-- spell_warl_deaths_embrace_dots (covers Agony/Corruption/Unstable Affliction, the genuinely
-- Legion-relevant piece) and spell_warl_deaths_embrace_drain_life are NOT touched by this migration
-- - they're still bound to the wrong constant (SPELL_WARLOCK_DEATHS_EMBRACE = 453189, the modern
-- retail id) and need a real rebind to 234876, but that requires confirming 234876's actual
-- SpellEffect index layout first (see ARGUSCORE_FIXES.md follow-up) - left unchanged rather than
-- guessed at.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warl_deaths_embrace';
