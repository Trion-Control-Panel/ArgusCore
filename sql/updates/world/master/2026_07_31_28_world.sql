-- Warlock/Affliction: Summon Darkglare (103673 pet) / Eye Laser (205231) - Affliction cooldown
-- pet whose signature ability refreshes Doom on all afflicted enemies within range. Entirely
-- missing from ArgusCore. Confirmed via DestinyCore and AshamaneCore (identical
-- implementations). New pet_warlock.cpp added (matching the existing per-class Pet/ directory
-- convention already used for DK/Hunter/Mage/Priest/Shaman) binds the Darkglare pet AI via
-- creature_template.ScriptName, the same pattern already used for Shaman's Feral Spirit.
UPDATE `creature_template` SET `ScriptName` = 'npc_pet_warlock_darkglare' WHERE `entry` = 103673;

DELETE FROM `spell_script_names` WHERE `spell_id` = 205231 AND `ScriptName` = 'spell_warl_eye_laser';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(205231, 'spell_warl_eye_laser');
