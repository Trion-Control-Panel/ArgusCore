-- Mage: follow-up fixes after discovering this session's empty-id scan had false negatives
-- (wago.tools' filter[SpellID] can substring-match neighboring ids, e.g. querying 89832 also
-- returns unrelated 189832 data, which fooled the original bulk "did any row come back" check).
-- Re-scanned every class's cached fetch data with a precise exact-column-match filter; for Mage
-- this surfaced two more genuinely-empty ids beyond what the original Mage pass caught.

-- Arcane Barrage's "Rank 3" mana-refund-per-Arcane-Charge mechanic (SPELL_MAGE_ARCANE_BARRAGE_R3/
-- 321526, SPELL_MAGE_ARCANE_BARRAGE_ENERGIZE/321529) removed from spell_mage.cpp entirely:
-- confirmed via web search to be Shadowlands 9.0.2 content ("Rank 3 added at level 52, granting 2%
-- of maximum mana per Arcane Charge spent") - didn't exist in Legion 7.3.5. Both ids confirmed
-- completely absent from this build. This is also exactly what the pasted validation-error log
-- showed ("SpellScriptBase::ValidateSpellInfo: Spell 321526/321529 does not exist"), which is what
-- prompted re-checking this class. The real Arcane Charge consumption itself (a genuine, tooltip-
-- documented cost, not tied to either broken id) is kept - only the mana-refund branch is gone.
-- spell_mage_arcane_barrage was never bound in the DB, so nothing to unbind.

-- spell_mage_firestarter_dots rebound from 321712 (a stale header comment referencing an id
-- confirmed completely absent from this build) to the real id 12654 (Ignite, already used
-- elsewhere in this same file as SPELL_MAGE_IGNITE). Confirmed via effect structure: this class's
-- hook expects EFFECT_ALL/SPELL_AURA_PERIODIC_DAMAGE (a DoT tick), which Pyroblast (11366, the
-- name the stale comment referenced) doesn't have at all - it's a single-effect direct-damage
-- spell - while Ignite's real EFFECT_0 is exactly SPELL_AURA_PERIODIC_DAMAGE (1000ms period),
-- matching the hook precisely. Never bound in the DB before now.
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(12654, 'spell_mage_firestarter_dots');
