-- Priest: partial recheck against real Legion 7.3.5 client data (empty-id sweep only - see
-- comment at the bottom for a much larger scope finding that still needs a follow-up pass).

-- Void Tendrils (spell_pri_void_tendrils, bound to 108920) unbound and removed from
-- spell_priest.cpp entirely: confirmed via web search that Void Tendrils was removed in patch
-- 7.0.3 - the Legion pre-patch itself - and only reintroduced in Dragonflight (10.0.0), under a
-- different id. Didn't exist during Legion 7.3.5 at all (same pattern as Mage's Alter Time and
-- DK's Death Siphon from earlier passes this session). This one was live: an earlier migration
-- (2026_07_26_56_world.sql) bound it to 108920 alongside four other, genuinely-real Voidform-
-- system classes, not realizing 108920 itself doesn't exist in this build - the ability has been
-- completely non-functional (silently inert, no error) since that migration. Unbinding now.
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_pri_void_tendrils';

-- Two more classes removed from spell_priest.cpp as dead code (both already unbound, so no DB
-- change needed for either): spell_pri_halo_shadow (bound to 120644) and
-- spell_pri_divine_star_shadow (bound to 122121). Both, plus a same-named ternary branch inside
-- the shared areatrigger_pri_halo / areatrigger_pri_divine_star structs, referenced a "Shadow
-- spec variant" of Halo/Divine Star (120644/390964/390971 and 122121/390845/390981 respectively)
-- that is confirmed completely absent from this build under any id - all six ids are absent, and
-- the 390xxx ones match the same later-expansion id cluster as most of this file's other empty
-- ids (see below). The real Holy-spec ids (120517/120696/120692 and 110744/122128/110745) are all
-- confirmed present and already correctly wired - since the "== SPELL_PRIEST_HALO_SHADOW"/
-- "== SPELL_PRIEST_DIVINE_STAR_SHADOW" comparisons could never actually match a real cast (no
-- spell exists at those ids to create the areatrigger in the first place), both areatriggers were
-- already unconditionally falling through to the Holy branch in practice - removing the dead
-- branches changes no behavior, just removes unreachable code and six now-unused constants.

-- Twist of Fate (spell_pri_twist_of_fate) was never bound to either of the two ids in its own
-- header comment. Of the two, 109142 ("Twist of Fate (Shadow)") is confirmed real and matches the
-- existing generic EFFECT_0/SPELL_AURA_PROC_TRIGGER_SPELL hook exactly (confirmed real native
-- SpellAuraOptions data, ProcTypeMask 2446676, flat ProcChance 100, no RPPM). 265259 ("Twist of
-- Fate (Discipline)") is confirmed completely absent - Discipline didn't have this talent yet in
-- Legion 7.3.5, so no binding is added for it (left as-is, not fixed - no confident replacement
-- id found). Binding + spell_proc added for the real Shadow variant only.
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(109142, 'spell_pri_twist_of_fate');
INSERT INTO `spell_proc` (`SpellId`) VALUES
(109142);

-- Two further items checked and left flagged (not fixed this pass - no confident low-risk fix
-- available):
--
-- Power Leech Passive (spell_pri_power_leech_passive, never bound): its own Validate() call
-- requires all four of SPELL_PRIEST_POWER_LEECH_{SHADOWFIEND,MINDBENDER}_{MANA,INSANITY} to
-- exist, but the two Mindbender ids (123051/200010, "Mana Leech"/"Power Leech") are confirmed
-- real while the two Shadowfiend ids (343727/262485) are confirmed absent - the whole class fails
-- Validate() and is inert as a result (matching its current already-unbound state, so no
-- regression either way). A same-named real "Mana Leech" (28305) exists for classic-era
-- Shadowfiend, but its effect layout (3 effects: DUMMY + two extra aura effects the class doesn't
-- expect) doesn't cleanly match what a Legion-era single-DUMMY-effect passive should look like, so
-- it wasn't used as a confident replacement.
--
-- MAJOR SCOPE FINDING, not addressed this pass: while checking spell_proc coverage, found that of
-- ~19 proc-hooked classes in this file, only 3 (spell_pri_atonement, spell_pri_focused_will,
-- spell_pri_shadowy_apparitions) are actually bound in spell_script_names at all - the rest
-- (spell_pri_aq_3p_bonus, spell_pri_atonement_passive, spell_pri_holy_words,
-- spell_pri_protective_light, spell_pri_shadow_mend_periodic_damage, spell_pri_surge_of_light,
-- spell_pri_t10_heal_2p_bonus, spell_pri_t3_4p_bonus, spell_pri_t5_heal_2p_bonus,
-- spell_pri_trail_of_light, spell_pri_vampiric_embrace, spell_pri_prayer_of_mending_aura) have no
-- spell_script_names row of any kind and are completely inert regardless of spell_proc. T3/T5 are
-- vanilla/AQ40-era tier bonuses (plausibly intentional, matching the existing
-- spell_pal_item_t6_trinket precedent), but Holy Words, Protective Light, Trail of Light,
-- Vampiric Embrace, and Prayer of Mending's own proc-half are all current, expected-functional
-- Legion-era Priest abilities that appear to be silently non-functional. This is a bigger check
-- than the empty-id sweep this migration covers and needs its own dedicated pass.
--
-- Also confirmed non-issues from the empty-id scan: this file has ~60 more SPELL_PRIEST_* enum
-- constants pointing at ids absent from this build (mostly Shadowlands/Dragonflight/War Within
-- talent-tree content, e.g. Dark Reprimand, Divine Image, Essence Devourer, Mind Devourer,
-- Ultimate Penitence, Whispering Shadows, the literal "HOLY_10_1_CLASS_SET" tier constants) that
-- are declared but never referenced by any class body - dead/unused constants, not live bugs,
-- same category as Paladin's Seal of Righteousness from the previous pass. Left alone given the
-- sheer volume; not worth a line-by-line cleanup pass on their own.
