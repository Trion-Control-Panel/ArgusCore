-- Mage: full recheck against real Legion 7.3.5 client data.

-- Alter Time (spell_mage_alter_time_aura/spell_mage_alter_time_active) removed from
-- spell_mage.cpp: confirmed via web search that Alter Time was removed in patch 7.0.3 - the
-- Legion pre-patch itself - and only reintroduced in Shadowlands (9.0.1/9.0.2). Didn't exist
-- during Legion 7.3.5 at all (same pattern as DK's Death Siphon). All four referenced ids
-- (110909, 127140, 342246, 342247) confirmed completely absent from this build. Neither class
-- was ever bound in the DB, so nothing to unbind here.

-- Radiant Spark (spell_mage_radiant_spark, 376103/376105) removed: confirmed Dragonflight
-- (patch 10.0.0) content via web search, removed again in 11.0.0. Both ids absent from this
-- build, class never bound in the DB.

-- Tempest Barrier (spell_mage_tempest_barrier, 382289/382290) removed: confirmed Dragonflight
-- (patch 10.0.0) content via web search, removed again in 12.0.0. Both ids absent from this
-- build, class never bound in the DB.

-- spell_mage_prismatic_barrier removed: an exact functional duplicate of spell_mage_arcane_barrier
-- (both computed the same 7x-healing-bonus SCHOOL_ABSORB formula for the same real id, 235450).
-- spell_mage_arcane_barrier's header comment was corrected too - 235450 is actually named
-- "Prismatic Barrier" per its own tooltip; no separate player-facing "Arcane Barrier" spell
-- exists in this build under any id. spell_mage_arcane_barrier is already correctly bound
-- (spell_script_names, migration 2026_07_26_75) and needed no binding change, only the comment
-- fix and removal of the dead duplicate.

-- Conjure Refreshment (spell_mage_conjure_refreshment) was never bound anywhere in this
-- codebase's SQL history. Confirmed real id is 190336 (matches both the tooltip and the real
-- SPELL_EFFECT_DUMMY effect structure this script's OnEffectHitTarget hook expects) - not
-- SPELL_MAGE_CONJURE_REFRESHMENT (116136), which is a different, correctly-used *internal*
-- outcome spell (a plain CREATE_ITEM effect the script casts when solo, no script needed for
-- that half) alongside SPELL_MAGE_CONJURE_REFRESHMENT_TABLE (167145, the grouped variant).
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(190336, 'spell_mage_conjure_refreshment');

-- spell_proc coverage: Chrono Shift (235711), Blazing Barrier (235313), Burning Determination
-- (198063), Ice Barrier (11426), and Erosion (205039) all gate their DoCheckProc/OnEffectProc
-- hooks behind Aura::GetProcEffectMask, which returns 0 unconditionally without a spell_proc row.
-- All five have confirmed real native proc data in SpellAuraOptions.db2 for this build, so a bare
-- SpellId-only row is enough to restore function.
--
-- Chilled to the Core (195448) and Frenetic Speed (236058) were also checked and DO have
-- confirmed real native proc data, but are deliberately NOT included below - both already have a
-- spell_proc row from the earlier server-wide migration (2026_08_07_08_world.sql, its own
-- spell_mage section). Re-inserting them here would violate spell_proc's primary key.
--
-- Brain Freeze (190447), Fingers of Frost (112965), and Improved Mana Gems (37447/61062) were
-- checked too but have NO native SpellAuraOptions data at all for this build - bare stub rows
-- would do nothing, so they're deliberately NOT included here; need individual investigation.
INSERT IGNORE INTO `spell_proc` (`SpellId`) VALUES
(235711),
(235313),
(198063),
(11426),
(205039);
