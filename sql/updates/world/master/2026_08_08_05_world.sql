-- Paladin: full recheck against real Legion 7.3.5 client data.

-- Righteous Verdict (spell_pal_righteous_verdict) was never bound in the DB under any id, and
-- both ids it referenced (267610 talent / 267611 buff aura) are confirmed BfA+ redesign ids,
-- completely absent from this build. Real Legion 7.3.5 ids are 238062 ("Righteous Verdict" talent,
-- confirmed matching EFFECT_0 SPELL_AURA_DUMMY structure the existing hook already expects) and
-- 238996 (the buff it casts, a native SPELL_AURA_ADD_PCT_MODIFIER keyed to Blade of Justice via
-- SpellClassMask - matches the "next Blade of Justice deals increased damage" tooltip exactly).
-- Rebound both constants in spell_paladin.cpp; adding the binding here since it never existed.
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(238062, 'spell_pal_righteous_verdict');

-- Three classes removed from spell_paladin.cpp entirely (none were ever bound in the DB, so no
-- DELETE needed - this is documentation only):
--
-- Final Verdict (spell_pal_final_verdict, bound to 383328/383329): both ids confirmed absent from
-- this build. Real Legion Final Verdict is 198038, a completely different mechanic than the coded
-- "roll a chance on hit, then cast a buff" design - it's a passive talent with two native
-- SPELL_AURA_ADD_PCT_MODIFIER effects (directly buffing Templar's Verdict and Divine Storm damage
-- via SpellClassMask), needing zero scripting at all (same category as this session's earlier
-- Mana Tea fix). No spell_script_names row is needed for 198038.
--
-- Moment of Glory (spell_pal_moment_of_glory, bound to 327193): confirmed completely absent from
-- this build under any id - no name match anywhere in the client dump either. Numeric id range and
-- lack of any Legion-era record both point to later-expansion (Dragonflight-era Protection PvP
-- talent) content that didn't exist in 7.3.5.
--
-- Steed of Liberty (spell_pal_steed_of_liberty, bound to 469304): confirmed via web search to be
-- The War Within content added in patch 11.0.5 (October 2024), over 8 years after Legion 7.3.5.
-- Id confirmed completely absent from this build.

-- spell_pal_art_of_war (267344, absent from this build - a current-retail id) removed entirely
-- from spell_paladin.cpp: an earlier pass (2026_07_28_10_world.sql) already added an independent,
-- already-correct implementation of this exact mechanic under a different class name,
-- spell_pal_blade_of_wrath_proc, already bound to the real Legion id (231832 - "Blade of Wrath").
-- Initially this pass tried to rebind spell_pal_art_of_war to 231832 too (with the same
-- DestinyCore/AshamaneCore-corroborated SPELL_AURA_PROC_TRIGGER_SPELL structure) before noticing
-- the file already had spell_pal_blade_of_wrath_proc doing the identical thing - kept the
-- already-bound original instead of creating a functional duplicate.
--
-- While cross-checking that duplicate, initially suspected a bug in 231832's existing spell_proc
-- coverage: 2026_08_07_08_world.sql inserted a bare `(231832)` row, and since 231832 is RPPM-based
-- (native SpellAuraOptions SpellProcsPerMinuteID 179), SpellMgr::LoadSpellProcs' load-time fallback
-- (spellInfo->ProcChance, a 101 sentinel meaning "RPPM governs", not a real 101% chance) looked like
-- it would bake a bogus ~always-proc Chance into the cached entry. Traced the actual proc-chance
-- code path (Aura::CalcProcChance in SpellAuras.cpp) before "fixing" this: it has a second,
-- unconditional check - `if (GetSpellInfo()->ProcBasePPM > 0.0f) chance = CalcPPMProcChance(...)`
-- - and ProcBasePPM is populated straight from the spell's own native SpellProcsPerMinuteID lookup
-- during SpellInfo construction (SpellInfo.cpp), entirely independent of the spell_proc table. So
-- the correct ~4-procs-per-minute rate was already in effect regardless of the row's Chance value -
-- not a real bug. Left as a no-op discovery; no UPDATE needed (none applied).

-- spell_proc coverage: Awakening (248033), Crusader Might (196926), Divine Purpose (223817),
-- Selfless Healer (85804), and the newly-rebound Righteous Verdict (238062) all gate their
-- DoCheckEffectProc/OnEffectProc hooks behind Aura::GetProcEffectMask, which returns 0
-- unconditionally without a spell_proc row. All five have confirmed real native proc data in
-- SpellAuraOptions.db2 for this build (flat ProcChance, no RPPM), so a bare SpellId-only row is
-- enough to restore function.
--
-- Righteous Protector (204074) was checked too but has NO native SpellAuraOptions data at all for
-- this build - a bare stub row would do nothing, so it's deliberately NOT included here; needs
-- individual investigation instead of a guessed row.
INSERT IGNORE INTO `spell_proc` (`SpellId`) VALUES
(248033),
(196926),
(223817),
(85804),
(238062);

-- One further item checked and left flagged (not fixed this pass - no confident low-risk fix
-- available, matching this session's "don't guess" rule):
--
-- Zeal (spell_pal_zeal, bound to 269569/269571): both ids are current-retail, absent from this
-- build. A real Legion "Zeal" exists (217020) but it's a fundamentally different, far more complex
-- mechanic (a 6-effect active Holy Power builder with its own charge/chain system) than the coded
-- simple "talent proc grants a stacking chain-damage buff" design - same category of gap as this
-- session's earlier Ring of Peace finding. Never bound in the DB currently, so zero live impact.

-- Also checked and confirmed non-issues (false positives from the empty-id scan, no code changes):
-- 9228 is an AreaTrigger.db2 template id (areatrigger_pal_consecration), not a Spell.csv id, so it
-- correctly doesn't appear in Spell data. 19746 (SPELL_PALADIN_CONCENTRACTION_AURA) and 25742
-- (SPELL_PALADIN_SEAL_OF_RIGHTEOUSNESS) are both declared but never referenced anywhere else in
-- the file - dead constants with wrong ids, but inert (minor cleanup candidates, not live bugs).
-- 61988 (SPELL_PALADIN_IMMUNE_SHIELD_MARKER) is an explicitly-commented serverside custom marker,
-- correctly used in spell_pal_lay_on_hands. 471195 only appears in a documentation comment on
-- spell_pal_lay_on_hands noting a future redesign lineage - the class itself correctly uses real
-- id 633, so this was never an active reference. 276111 (Divine Steed Dwarf) and 356717 (Infusion
-- of Light Energize) were already investigated and flagged in earlier passes.
