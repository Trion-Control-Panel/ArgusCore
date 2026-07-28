-- Mage: Ray of Frost (205021) had a Fingers of Frost interaction bolted onto it that was only
-- ever added in Battle for Azeroth's 8.0.1 pre-patch (2018), well after Legion 7.3.5 - confirmed
-- via Wowhead/Warcraft Wiki patch history. Removed that logic and the now-unused SpellScript half
-- (renaming the surviving AuraScript from 'spell_mage_ray_of_frost' to 'spell_mage_ray_of_frost_aura'
-- to match its actual class name), and implemented the real Legion mechanic that was missing
-- instead: Ray of Frost Buff (208166), which suppresses Ray of Frost's own cooldown for the
-- duration of the channel and restores it (force-ending the channel too, if still active) once
-- the buff falls off. See ARGUSCORE_FIXES.md for the full writeup.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_mage_ray_of_frost', 'spell_mage_ray_of_frost_aura', 'spell_mage_ray_of_frost_buff'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(205021, 'spell_mage_ray_of_frost_aura'),
(208166, 'spell_mage_ray_of_frost_buff');
