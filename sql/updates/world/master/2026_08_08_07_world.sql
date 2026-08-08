-- Priest: dedicated pass on the "classes with no spell_script_names row at all" finding from
-- the previous migration's empty-id sweep. For each, verified the class's own hooks (EFFECT
-- index + aura/effect type) against real fetched SpellEffect.db2 data for this build before
-- binding - these are genuine binding oversights, not wrong-id or wrong-mechanic bugs: the ids
-- in each class's own header comment/constants are correct and already match the coded hooks
-- exactly, they were simply never wired into spell_script_names.

-- Prayer of Mending - one of Holy Priest's most iconic spells, completely non-functional until
-- now. Three real ids involved, each independently verified against real effect data:
--   33076 (root cast, "Prayer of Mending (Dummy)"): real EFFECT_0 is SPELL_EFFECT_DUMMY (Effect
--     type 3), matches spell_pri_prayer_of_mending_dummy's hook exactly.
--   41635 (the granted HoT/jump buff): real EFFECT_0 is SPELL_EFFECT_APPLY_AURA (Effect type 6)
--     with SPELL_AURA_DUMMY (aura type 4) - matches BOTH halves of the
--     spell_pri_prayer_of_mending / spell_pri_prayer_of_mending_aura pair (the SpellScript half
--     hooks the APPLY_AURA cast effect to stash the Focused Mending flag onto the just-created
--     aura instance; the AuraScript half hooks the aura's own DUMMY proc later).
--   155793 ("Prayer of Mending (Jump)"): real EFFECT_0 is SPELL_EFFECT_DUMMY, matches
--     spell_pri_prayer_of_mending_jump's hook exactly.
-- Note: spell_pri_prayer_of_mending is registered via RegisterSpellAndAuraScriptPair(
-- spell_pri_prayer_of_mending, spell_pri_prayer_of_mending_aura), which expands to
-- RegisterSpellAndAuraScriptPairWithArgs(..., #script_1) - only script_1's stringified name is
-- ever registered as an invokable ScriptName; spell_pri_prayer_of_mending_aura is never bound
-- independently, GenericSpellAndAuraScriptLoader creates both instances internally. A single row
-- under 'spell_pri_prayer_of_mending' is correct and sufficient (confirmed against the macro
-- definition in ScriptMgr.h before writing this, not assumed).
DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_pri_prayer_of_mending_dummy', 'spell_pri_prayer_of_mending', 'spell_pri_prayer_of_mending_jump'
);
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(33076, 'spell_pri_prayer_of_mending_dummy'),
(41635, 'spell_pri_prayer_of_mending'),
(155793, 'spell_pri_prayer_of_mending_jump');

-- Seven more classes, each confirmed real and matching (EFFECT_0 verified DUMMY/PERIODIC_DUMMY
-- against real fetched data, matching the coded hook in every case):
--   195178 - Atonement (Passive)
--   63733  - Serendipity (Holy Words) - already carried an extensive prior-session comment
--     confirming its 3-effect layout against real 7.3.5.26972 data; only the binding was missing.
--   187464 - Shadow Mend (periodic damage half) - EFFECT_0 confirmed SPELL_AURA_PERIODIC_DUMMY
--     (aura 226, 1000ms period), EFFECT_1 confirmed SPELL_AURA_DUMMY, matching both hooks.
--   109186 - Surge of Light
--   200128 - Trail of Light
--   15286  - Vampiric Embrace - already carried a prior-session comment confirming EFFECT_0 is
--     SPELL_AURA_PERIODIC_DUMMY, not plain DUMMY; only the binding was missing.
--   193063 - Protective Light - EFFECT_0 confirmed SPELL_AURA_DUMMY, but has NO native
--     SpellAuraOptions proc data in this build (unlike the other six) - bound below, but no
--     spell_proc row added for it; needs individual investigation like this session's earlier
--     Misdirection/Brain Freeze findings.
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(195178, 'spell_pri_atonement_passive'),
(63733, 'spell_pri_holy_words'),
(187464, 'spell_pri_shadow_mend_periodic_damage'),
(109186, 'spell_pri_surge_of_light'),
(200128, 'spell_pri_trail_of_light'),
(15286, 'spell_pri_vampiric_embrace'),
(193063, 'spell_pri_protective_light');

-- spell_proc coverage for the newly-bound classes (all confirmed real native SpellAuraOptions
-- data, flat ProcChance, no RPPM). 109186 (Surge of Light), 187464 (Shadow Mend), and 195178
-- (Atonement Passive) already have spell_proc rows from the earlier server-wide migration
-- (2026_08_07_08_world.sql) - not re-inserted here to avoid a primary-key violation (caught by
-- re-checking every id against the full migration history, not just the initial pass - the first
-- check used a broken grep pipeline that silently missed this). 41635 (Prayer of Mending's own
-- proc-driven heal/jump) and 63733/15286 confirmed real native data with no existing row, and are
-- included below.
INSERT IGNORE INTO `spell_proc` (`SpellId`) VALUES
(63733),
(15286),
(41635);

-- Four classes intentionally left unbound (not a bug, matching the existing
-- spell_pal_item_t6_trinket precedent from the Paladin pass): all four are pre-Legion raid
-- tier-set bonuses whose granting gear isn't part of Legion's own itemization/loot tables, even
-- though the spell data itself still technically exists in this build's DB2 dump:
--   26169 - Oracle Healing Bonus (AQ40, vanilla T2.5)
--   28809 - Greater Heal (Naxxramas 40, vanilla T3)
--   37594 - Greater Heal Refund (Black Temple/Hyjal, TBC T5)
--   70770 - Item - Priest T10 Healer 2P Bonus (Icecrown Citadel, WotLK T10)
