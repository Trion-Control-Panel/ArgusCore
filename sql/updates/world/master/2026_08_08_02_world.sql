-- Hunter: full recheck against real Legion 7.3.5 client data.

-- Cobra Sting (378750) removed from spell_hunter.cpp: confirmed Dragonflight (patch 10.0.0)
-- content via web search, didn't exist yet in Legion 7.3.5. Id completely absent from this build
-- and never bound in the DB, so nothing to unbind here.

-- Scrappy (459533) removed from spell_hunter.cpp: confirmed The War Within (patch 11.0.0)
-- content via web search, didn't exist yet in Legion 7.3.5. Id completely absent from this build
-- and never bound in the DB, so nothing to unbind here.

-- spell_proc coverage: Wild Call (185789), True Aim (199527), and Way of the Mok'Nathal (201082)
-- are all already bound in spell_script_names but gate their DoCheckProc hooks behind
-- Aura::GetProcEffectMask, which returns 0 unconditionally without a spell_proc row. All three
-- have confirmed real native proc data in SpellAuraOptions.db2 for this build (ProcTypeMask/
-- ProcChance), so a bare SpellId-only row is enough to restore function.
--
-- Misdirection (34477) was checked too but has NO native SpellAuraOptions data at all for this
-- build - a bare stub row would do nothing (no data to fall back to), so it's deliberately NOT
-- included here; needs individual investigation instead of a guessed row.
INSERT INTO `spell_proc` (`SpellId`) VALUES
(185789),
(199527),
(201082);
