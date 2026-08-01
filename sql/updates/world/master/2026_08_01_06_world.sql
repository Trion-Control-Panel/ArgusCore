-- Rebind two Druid spell_script_names rows from modern-remake spell ids to the real Legion ids.
-- Both scripts' C++ logic is genuine, correct Legion-era content; only the DB binding was wrong
-- (bound to a later-expansion rework id instead of the Legion-era one). Confirmed via DestinyCore's
-- live 735.02 world dump (spell_dru_berserk -> 106951) and via this file's own code comments,
-- which already documented the real id (194223) alongside the wrong one it was actually bound to.
-- See ARGUSCORE_FIXES.md for the full investigation (also found and removed 8 other Druid
-- spell_script_names rows that turned out to be genuine post-Legion forward drift with no
-- Legion equivalent at all, not a rebind case - see the C++ diff in the same commit).

-- spell_dru_berserk: 50334 (pre-Cata id) -> 106951 (the real, stable Legion-era Guardian Berserk id)
DELETE FROM `spell_script_names` WHERE `spell_id` = 50334 AND `ScriptName` = 'spell_dru_berserk';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (106951, 'spell_dru_berserk');

-- spell_dru_celestial_alignment: 383410/390414 (Dragonflight remake ids) -> 194223/102560
-- (Celestial Alignment / Incarnation: Chosen of Elune, both trigger the same instant-dual-eclipse
-- effect this script implements, and were already documented in the file's own comments).
DELETE FROM `spell_script_names` WHERE `spell_id` IN (383410, 390414) AND `ScriptName` = 'spell_dru_celestial_alignment';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
    (194223, 'spell_dru_celestial_alignment'),
    (102560, 'spell_dru_celestial_alignment');

-- Cleanup: remove the spell_script_names rows for the 8 classes confirmed as post-Legion forward
-- drift and removed from spell_druid.cpp in the same pass (astral_communion_celestial_alignment,
-- astral_smolder, embrace_of_the_dream, embrace_of_the_dream_effect, luxuriant_soil,
-- natures_grace, natures_grace_eclipse, power_of_the_archdruid) - the C++ classes no longer
-- exist, so these rows would otherwise just be silent dead weight going forward.
DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_dru_astral_communion_celestial_alignment',
    'spell_dru_astral_smolder',
    'spell_dru_embrace_of_the_dream',
    'spell_dru_embrace_of_the_dream_effect',
    'spell_dru_luxuriant_soil',
    'spell_dru_natures_grace',
    'spell_dru_natures_grace_eclipse',
    'spell_dru_power_of_the_archdruid'
);
