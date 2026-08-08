-- Cross-class: folds in and completes the work from an unfinished, uncommitted
-- 2026_08_07_09_world.sql (found sitting untracked in the working tree, predating this session -
-- superseded by this migration, not deleted since it wasn't ours to remove).
--
-- That file's premise was solid and independently re-verified here: RegisterSpellAndAuraScriptPair
-- (X, Y) binds only under X's stringified name (confirmed against the ScriptMgr.h macro
-- definition, same as the Priest Prayer of Mending finding earlier this pass) - so any earlier
-- codebase-wide spell_proc scan that matched proc-hook class names directly against
-- spell_script_names.ScriptName would silently miss cases where the proc hook lives in Y (the
-- second/aura half of the pair) rather than X. It found 4 such cases and queued spell_proc rows
-- for all 4, all independently re-confirmed here to have real native SpellAuraOptions proc data
-- for build 7.3.5.26972 (171975 is RPPM-based - SpellProcsPerMinuteID 107 - but per this session's
-- earlier Paladin finding, Aura::CalcProcChance's native ProcBasePPM path overrides a bare row's
-- flat Chance at proc-time regardless, so a bare row is still correct/safe here).
--
-- Two of the four (116095 - Monk Disable, 171975 - Warlock Grimoire of Synergy) were already
-- correctly bound in spell_script_names, so only needed the spell_proc row (which is all the old
-- file did for them). The other two (27243 - Warlock Seed of Corruption, 57934 - Rogue Tricks of
-- the Trade) turned out to ALSO be missing their spell_script_names binding entirely - the old
-- file's own comments ("bound as spell_warl_seed_of_corruption_dummy" / "bound as
-- spell_rog_tricks_of_the_trade") were incorrect assumptions, not verified fact: neither ScriptName
-- has ever appeared in spell_script_names anywhere in this repo's migration history. Both classes'
-- ids were independently confirmed correct against real Spell/effect data (27243 = "Seed of
-- Corruption" by name; 57934 already carries its own "// 57934 - Tricks of the Trade" header
-- comment in spell_rogue.cpp) - adding the missing bindings here too, so the spell_proc rows are
-- actually meaningful instead of silently inert.
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(27243, 'spell_warl_seed_of_corruption_dummy'),
(57934, 'spell_rog_tricks_of_the_trade');

INSERT INTO `spell_proc` (`SpellId`) VALUES
(27243),
(57934),
(116095),
(171975);
