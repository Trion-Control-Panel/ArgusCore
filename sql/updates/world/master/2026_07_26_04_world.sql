-- Bind five newly-implemented Monk Mistweaver spell scripts to their spell ids.
-- All five are brand-new classes - no existing spell_script_names row for any
-- of them anywhere in this repo's SQL, so without these bindings Mistweaver's
-- core healing kit (Enveloping Mist, Renewing Mist, Soothing Mist) would not
-- function at all.
--
-- Note: spell_monk_mists_of_life (already implemented, already bound) casts
-- both SPELL_MONK_RENEWING_MIST (119611) and SPELL_MONK_ENVELOPING_MIST (124682)
-- directly, and was previously inert on the Enveloping Mist half specifically
-- since 124682 had no script to apply the actual heal.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_monk_enveloping_mist', 'spell_monk_renewing_mist', 'spell_monk_renewing_mist_periodic', 'spell_monk_soothing_mist', 'spell_monk_soothing_mist_aura');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(124682, 'spell_monk_enveloping_mist'),        -- Enveloping Mist
(115151, 'spell_monk_renewing_mist'),          -- Renewing Mist (cast wrapper)
(119611, 'spell_monk_renewing_mist_periodic'), -- Renewing Mist (periodic HoT, jump mechanic)
(115175, 'spell_monk_soothing_mist'),          -- Soothing Mist (channel)
(193884, 'spell_monk_soothing_mist_aura');     -- Soothing Mist (redirect)
