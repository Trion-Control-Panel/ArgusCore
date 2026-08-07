-- Follow-up to 2026_08_07_08: the earlier full-codebase spell_proc scan matched proc-hook class
-- names directly against spell_script_names.ScriptName. That misses any class registered via
-- RegisterSpellAndAuraScriptPair(X, Y) where the proc hook lives in Y (the second class) - the
-- DB row is always bound under X's name, not Y's, so those spells were silently skipped entirely
-- rather than counted as missing. Found 5 such cases codebase-wide; 4 need this fix (the 5th,
-- Prayer of Mending 41635, already had a row). All 4 confirmed to have real native proc data in
-- SpellAuraOptions for build 7.3.5.26972, same as 2026_08_07_08 - see that migration for the
-- fallback mechanism this relies on.
INSERT INTO `spell_proc` (`SpellId`) VALUES
(27243), -- spell_warl_seed_of_corruption_dummy_aura (bound as spell_warl_seed_of_corruption_dummy)
(57934), -- spell_rog_tricks_of_the_trade_aura (bound as spell_rog_tricks_of_the_trade)
(116095), -- aura_monk_disable (bound as spell_monk_disable)
(171975); -- spell_warl_grimoire_of_synergy_aura (bound as spell_warl_grimoire_of_synergy)
