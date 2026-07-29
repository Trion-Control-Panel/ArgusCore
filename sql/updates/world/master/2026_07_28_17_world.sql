-- Warlock: first pass of a class-diff sweep (Warlock was previously entirely unaudited). This file
-- has the same higher false-positive rate as Paladin/Shaman's early sweeps - several candidates
-- from the initial diff turned out to already be covered under more modern designs, or to be
-- glyph-only dead code (the entire glyph system is gone since Legion 7.0).
--
-- Confirmed false positives, no change needed: Corruption's "Absolute Corruption" duration logic
-- (146739) - already covered by the existing spell_warl_absolute_corruption, bound directly to
-- Corruption's own cast and gated on IsPvP() instead of the reference's manual periodic
-- caster-distance/death check. Life Tap - the reference's only non-glyph logic is a "fizzle below
-- 15% health unless glyphed" safety exclusively there to support the (now-removed) Glyph of Life
-- Tap; without it there's nothing left to port.
--
-- Fix: four genuinely missing, self-contained Demonology/Destruction abilities, each confirmed via
-- logs/DestinyCore/sql/DB_world_735.02.sql's own spell_script_names dump:
--
-- - Call Dreadstalkers (104316) - summons 2 Dreadstalkers, plus Wild Imps via Improved
--   Dreadstalkers.
-- - Hand of Gul'dan (105174) - summons extra Wild Imps and scales its damage (86040) by Soul
--   Shards spent, with Hand of Doom additionally applying Doom. Deliberately merged what the
--   reference splits into two scripts (105174 and 86040) into one: the damage spell (86040) has
--   no Soul Shard cost of its own, so a script bound to it independently (as the reference does)
--   can't re-derive how many shards the original 105174 cast spent - ArgusCore's engine has no
--   shared-state channel between two separately-triggered casts for this. Computing the scaled
--   damage in 105174's own script (which still has that cost information via
--   GetPowerTypeCostAmount) and passing it to 86040 as a SPELLVALUE_BASE_POINT0 override avoids
--   the problem entirely, so 86040 needs no spell_script_names row of its own.
-- - Havoc (80240) - copies damage the caster deals elsewhere onto the Havoc'd target too.
-- - Channel Demonfire (196447) - periodically damages a random nearby enemy carrying the
--   caster's own Immolate; translated the reference's now-unresolvable
--   `SPELL_WARLOCK_IMMOLATE_DOT` constant to ArgusCore's own already-established
--   `SPELL_WARLOCK_IMMOLATE_PERIODIC` (same id, 157736, just named differently), and its
--   `GetAttackableUnitListInRange` (which doesn't exist in ArgusCore) to the standard
--   `Trinity::AnyUnfriendlyUnitInObjectRangeCheck` + `UnitListSearcher` + `Cell::VisitAllObjects`
--   idiom already used elsewhere in the codebase for the same "gather nearby enemies" need.
--
-- Deferred this pass: Implosion (196277) - the reference's own script body is entirely commented
-- out (dead code, nothing to translate); building the Wild-Imp-explosion mechanic from scratch
-- without a working reference to verify against carries more drift risk than this sweep's
-- established pattern of binding to existing, working reference logic. Demonwrath (193439) and the
-- Demonic Calling talent it depends on - a larger, more interdependent cluster deferred to a
-- follow-up pass rather than rushed. Incinerate (29722) - the reference's Soul Shard generation
-- values look like a pre-rework (MoP Burning Embers-era) balance leftover that doesn't match
-- current Destruction design; left uninvestigated rather than guessed at. Fear's "fear buff"
-- duration-cap script (5782/204730) - unclear whether this is still-relevant PvP DR handling or
-- dead weight; low priority.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN
(
    'spell_warl_call_dreadstalkers',
    'spell_warl_hand_of_guldan',
    'spell_warl_havoc',
    'spell_warl_channel_demonfire'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(104316, 'spell_warl_call_dreadstalkers'),
(105174, 'spell_warl_hand_of_guldan'),
(80240, 'spell_warl_havoc'),
(196447, 'spell_warl_channel_demonfire');
