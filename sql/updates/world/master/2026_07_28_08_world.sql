-- Paladin: start of a new full class-diff sweep (Paladin/Shaman/Warlock had not yet been audited
-- this pass). Investigated many candidates from the initial diff; most turned out to already be
-- implemented under a differently-structured design (Crusader Strike's Crusader's Might
-- interaction is already handled by the existing proc-gated spell_pal_crusader_might; Flash of
-- Light's Infusion of Light interaction is already handled by the existing charge-based
-- spell_pal_infusion_of_light). One genuine gap found: Beacon of Faith and Beacon of Virtue (Holy
-- Paladin talent variants of Beacon of Light) were not wired into the existing heal-echo system at
-- all - only base Beacon of Light worked. Implemented the missing half (Beacon of Faith's own
-- heal-echo proc at half Beacon of Light's healing, matching a real trade-off for being usable
-- alongside Beacon of Light on a second target) plus the apply/remove dispatcher and cast-time
-- mutual-exclusivity check for all three variants. Corroborated byte-for-byte across both
-- DestinyCore and AshamaneCore. No existing spell_script_names rows for any of these ScriptNames
-- anywhere in this repo's SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_pal_beacon_of_faith_proc', 'spell_pal_beacon_of_light',
    'spell_pal_beacon_of_faith_check_cast', 'spell_pal_beacon_of_light_aura',
    'spell_pal_beacon_of_faith_aura', 'spell_pal_beacon_of_virtue_aura'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(177173, 'spell_pal_beacon_of_faith_proc'),
(53563, 'spell_pal_beacon_of_light'),
(156910, 'spell_pal_beacon_of_faith_check_cast'),
(53563, 'spell_pal_beacon_of_light_aura'),
(156910, 'spell_pal_beacon_of_faith_aura'),
(200025, 'spell_pal_beacon_of_virtue_aura');
