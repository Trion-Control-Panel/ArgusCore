-- Warlock: full recheck against real Legion 7.3.5 client data.

-- Demonbolt (spell_warl_demonbolt, bound to 264178, casting SPELL_WARLOCK_DEMONBOLT_ENERGIZE/
-- 280127) removed from spell_warlock.cpp entirely: both ids confirmed absent from this build.
-- Real Legion Demonbolt (157695, confirmed via exact tooltip match: "Draws energy from your
-- demons... Generates 1 Soul Shard") already natively fires its own resource-generation spell
-- (196300, a plain SPELL_EFFECT_ENERGIZE) via its own EFFECT_1, a genuinely-implemented
-- SPELL_EFFECT_TRIGGER_SPELL (confirmed via Spell::EffectTriggerSpell, not a stub) - the class's
-- entire manual "cast an energize spell after cast" logic is redundant for the real spell (same
-- pattern as this session's earlier Rogue Premeditation finding). Never bound in the DB, so
-- nothing to unbind.

-- Soul Fire (spell_warl_soul_fire, bound to 6353) unbound: 6353 is confirmed absent from Spell.db2
-- under any id for this build, so the class has been silently non-functional since an earlier
-- migration (2025_05_30_04_world.sql) bound it. Four same-named "Soul Fire" candidates exist
-- (131381, 138554, 150289, 166864, all with identical single-effect SCHOOL_DAMAGE structure and
-- near-identical tooltip text) but none could be confidently disambiguated as the real Legion
-- 7.3.5 id - left unresolved rather than guessed at (see FIXME comment on
-- SPELL_WARLOCK_SOUL_FIRE_ENERGIZE in spell_warlock.cpp). Unbinding since a wrong-but-bound id is
-- no better than an unbound one.
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warl_soul_fire';

-- Death's Embrace's two DoT/heal-scaling pieces (spell_warl_deaths_embrace_dots,
-- spell_warl_deaths_embrace_drain_life) were never bound in the DB. An earlier migration
-- (2026_07_29_15_world.sql) had already documented that SPELL_WARLOCK_DEATHS_EMBRACE needed
-- rebinding from a modern retail id (453189) to the real Legion 7.1.5 id (234876), deferred
-- pending confirming 234876's effect layout - that id fix has since landed (the constant now
-- reads 234876 in the current file) and its real EFFECT_1 (SPELL_EFFECT_DUMMY) matches what both
-- classes' own Validate() calls expect, so both are ready to bind now:
--   spell_warl_deaths_embrace_dots -> Agony (980) and Corruption (146739). Both confirmed to
--     directly carry a native SPELL_AURA_PERIODIC_DAMAGE on EFFECT_0, matching the class's
--     generic EFFECT_ALL/SPELL_AURA_PERIODIC_DAMAGE hook. Unstable Affliction (the third DoT this
--     class's header comment lists it as also covering, under the wrong id 316099) is
--     deliberately NOT bound here - investigated the real id (30108, confirmed via exact tooltip
--     match: "you may afflict a target with up to N Unstable Afflictions at once", a
--     Legion-specific multi-DoT design) but its own effects are both SPELL_EFFECT_DUMMY, not a
--     direct PERIODIC_DAMAGE aura like Agony/Corruption - the real periodic-damage aura lives on
--     a separate hidden id (233490, referenced in 30108's own tooltip formula) that this generic
--     class may or may not correctly attach to depending on how 30108's DUMMY effects actually
--     apply it. Left unbound for Unstable Affliction specifically pending that follow-up.
--   spell_warl_deaths_embrace_drain_life -> Drain Life (234153). Confirmed real EFFECT_0 is
--     SPELL_AURA_PERIODIC_LEECH, matching the class's own hook exactly.
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(980, 'spell_warl_deaths_embrace_dots'),
(146739, 'spell_warl_deaths_embrace_dots'),
(234153, 'spell_warl_deaths_embrace_drain_life');

-- Left flagged, not fixed this pass (no confident low-risk fix, matching this session's "don't
-- guess" rule; class stays unbound, zero live impact either way):
--
-- Demonic Circle: Summon (spell_warl_demonic_circle_summon, real id 48018, already correctly
-- bound to its real cast spell) internally uses SPELL_WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST (62388,
-- confirmed absent) as a self-cast marker aura gating whether Demonic Circle: Teleport can be
-- cast (a "are you in range of your circle" check). Both DestinyCore and AshamaneCore solve this
-- with SendFakeAuraUpdate (a client-only fake aura display needing no real spell definition) -
-- but that API doesn't exist anywhere in ArgusCore's engine, so it isn't a straightforward
-- reference-to-idiom translation. Needs either a genuine real spell id or a considered
-- engine-level decision (e.g. a custom serverside marker id, matching the existing precedent for
-- Paladin's SPELL_PALADIN_IMMUNE_SHIELD_MARKER), not a guessed id swap.
