-- Monk: follow-up fixes from the same methodology-gap re-scan that produced the Mage follow-up
-- (2026_08_08_13). Monk's "full depth" batch turned out to be deeper than simple id swaps - most
-- of these are cases where the coded mechanic itself was wrong for real Legion 7.3.5, not just
-- pointing at a bad id. Full reasoning for each is in the C++ comments; summary here.

-- Legacy of the Emperor: SPELL_MONK_LEGACY_OF_THE_EMPEROR's value had drifted to 117667 (also
-- confirmed absent) while its own FIXME comment and DB binding both still referenced 115921 -
-- reverted the constant back to 115921 to remove the inconsistency. Still unresolved/flagged
-- either way (both ids confirmed absent, no confident replacement found); no functional change,
-- this was purely a documentation/consistency fix.

-- Keg Smash (spell_monk_keg_smash, bound to 121253, live): real Keg Smash's own tooltip
-- ("...Reduces the remaining cooldown on your Brews...") confirms it neither generates Chi nor
-- applies a separate "Weakened Blows"/"Dizzying Haze" debuff - all three ids the class relied on
-- (115798, 127796, 116330) are confirmed absent. The movement slow is already native (real
-- EFFECT_2). Removed the three wrong CastSpell calls; what real Keg Smash actually needs (Brew
-- cooldown reduction) isn't implemented at all - left as a documented gap (exact reduction amount
-- not guessed at). Also removed the same dead SPELL_MONK_DIZZYING_HAZE check from
-- spell_monk_breath_of_fire's condition (redundant with the already-correct
-- SPELL_MONK_KEG_SMASH_AURA check right next to it, per that class's own existing comment).

-- Spear Hand Strike (spell_monk_spear_hand_strike, bound to 116705, live): real Spear Hand
-- Strike's EFFECT_0 is a native SPELL_EFFECT_INTERRUPT_CAST, whose engine handler already calls
-- SpellHistory::LockSpellSchool using the spell's own duration - exactly the silence this class
-- was manually re-applying via SPELL_MONK_SPEAR_HAND_STRIKE_SILENCE (116709, confirmed absent).
-- Removed as redundant.

-- Soothing Mist (spell_monk_soothing_mist, bound to 115175, live): real Soothing Mist's tooltip
-- makes no mention of chance-based resource generation - the 25%-chance-to-energize logic
-- (SPELL_MONK_SOOTHING_MIST_ENERGIZE/116335, confirmed absent) matches an older MoP-era design
-- (a same-named id, 125759, explicitly tooltips "chance to generate 1 Chi") that Legion's
-- redesign dropped - same category as this session's earlier Mana Tea finding. Removed.
-- SPELL_MONK_SOOTHING_MIST_VISUAL (125955, cosmetic-only, confirmed absent) left as-is/unresolved.

-- Chi Torpedo (spell_monk_chi_torpedo, bound to 115008, live): real Chi Torpedo's SpellEffect
-- data is 6 effects, all movement-related (matching the same forced-movement aura pattern already
-- fixed for Monk's Roll and DH's Fel Rush earlier this session) - no damage or heal effect exists
-- anywhere in the real spell. Removed the "cone damage enemies / heal allies while rolling" logic
-- entirely (both ids it relied on, 117993/124040, confirmed absent) - the movement itself needs
-- no script, already fully native. Kept the unrelated PvP-gloves decrease-speed cleanup.

-- Enveloping Mist (spell_monk_enveloping_mist, bound to 124682, live) removed entirely: real
-- Enveloping Mist's own EFFECT_0 is a native SPELL_AURA_PERIODIC_HEAL, matching its own tooltip
-- exactly - it already heals on its own the moment it's applied. The class's premise ("casts the
-- actual heal, 132120, after cast completes") doesn't hold up against real SpellEffect data;
-- 132120 is confirmed absent. spell_monk_mists_of_life (which casts 124682 directly) already
-- gets the real heal for free without this script. Also fixed spell_monk_lifecycles, which was
-- checking proc events against the same dead 132120 id instead of the real 124682 - it would
-- never have detected a real Enveloping Mist heal proc.
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_monk_enveloping_mist';

-- Innervate Rank 2 (spell_dru_innervate, Druid - found during the same re-scan): no confident
-- real "Innervate Rank 2" id found under any candidate name; left flagged, unbound anyway.
-- Idol of Feral Shadows / Idol of Worship (also Druid): confirmed WotLK-era relic-slot items
-- (relic slot removed since Cataclysm) - harmless no-op code, same category as this session's
-- earlier T6 trinket precedent; no fix needed.
-- Hunter's SPELL_HUNTER_STEADY_SHOT_FOCUS (77443): same already-documented "Steady Shot" id
-- cluster as the existing FIXME on SPELL_HUNTER_STEADY_SHOT (56641) - no new replacement found,
-- companion FIXME comment added; spell_hun_steady_shot stays unbound (zero live impact) either way.
