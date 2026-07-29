-- Warlock: sixth pass, revisiting one item from the "unclear relevance" bucket after finding
-- stronger evidence. Soul Harvest (196098) turned out to be corroborated across all four
-- reference cores (not just the two primaries, as first assessed) - LegionCore-7.3.5/V2's
-- independently-written version confirmed the mechanic is real: extends the buff's own duration
-- based on how many nearby enemies carry the caster's spec DoT (Agony/Doom/Immolate for
-- Affliction/Demonology/Destruction), capped at 15 targets.
--
-- Translated two old-style APIs while porting: the reference's `PLAYER_FIELD_CURRENT_SPEC_ID`
-- switch (already established as a pre-refactor idiom earlier in this sweep, e.g. Paladin's old
-- Judgment) became `Player::GetPrimarySpecialization()` compared against `ChrSpecialization`
-- enumerators, matching the idiom already used elsewhere in this codebase (e.g. the Paladin
-- Consecration areatrigger's `ChrSpecialization::PaladinProtection` check). The reference's
-- `GetAttackableUnitListInRange()`/LegionCore's `GetCountMyAura()` (neither exists in ArgusCore)
-- became the same `Trinity::AnyUnfriendlyUnitInObjectRangeCheck` + `UnitListSearcher` +
-- `Cell::VisitAllObjects` idiom already used for Channel Demonfire earlier in this sweep. Also
-- preferred LegionCore-7.3.5's DB2-driven per-target/max-duration values (its own EFFECT_1/EFFECT_2)
-- over the primary references' hardcoded "2 seconds per target" magic number.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warl_soul_harvest';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(196098, 'spell_warl_soul_harvest');
