-- Rogue: remove seven more confirmed post-Legion forward-drift features, found while doing a
-- comprehensive re-scan of every remaining high (and not-so-high) spell id in this file after
-- the earlier poison/Improved Garrote/Improved Shiv cleanup:
--   - Acrobatic Strikes (455143/455144) - reworked in The War Within (11.0.5), originally a
--     later addition; not Legion content
--   - Airborne Irritant + its target-selection helper (200733/427773) - Dragonflight patch 10.2.0
--     (2023-11-07); note this one had a "safe-looking" id under 250,000, confirming id magnitude
--     alone isn't sufficient - it still needed a real patch-history check
--   - Blackjack (379005/394119) - clustered with other confirmed Dragonflight 10.0 ids
--     (Improved Garrote, Atrophic/Amplifying Poison) found earlier this pass
--   - Cloaked in Shadows (382515/386165) - Subtlety talent tuned as late as The War Within
--     11.0.2; never had a live spell_script_names row in the base TDB dump at all
--   - Soothing Darkness (393970/393971) - genuinely Legion-original (7.0.3) but ArgusCore's
--     implementation matches the Dragonflight 10.0 REDESIGN (heal-after-Vanish) rather than the
--     original Legion mechanic (periodic heal during Stealth/Shadow Dance) - removed rather than
--     guess at the correct historical id/mechanic
--   - Symbols of Death's Rank 2 bonus branch (328077) - a Shadowlands 9.0.1 addition bolted onto
--     the otherwise-correct, real-Legion Symbols of Death cast script; only the dead branch was
--     removed, the base ability (212283) is untouched and still functions
--   - Shot in the Dark + its buff half (257505/257506) - Battle for Azeroth (8.0.1); never had a
--     live spell_script_names row either
-- See ARGUSCORE_FIXES.md for the full writeup.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_rog_acrobatic_strikes', 'spell_rog_airborne_irritant',
    'spell_rog_airborne_irritant_target_selection', 'spell_rog_blackjack',
    'spell_rog_soothing_darkness', 'spell_rog_symbols_of_death'
);
