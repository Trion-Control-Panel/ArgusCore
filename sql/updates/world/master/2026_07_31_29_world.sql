-- Shaman: Earthen Shield Totem (198838 summon / 201633 absorb / 201657 self-damage) -
-- Restoration talent, entirely missing from ArgusCore. Confirmed via DestinyCore and
-- AshamaneCore (identical implementations) - see in-code comments on
-- spell_sha_earthen_shield_absorb and npc_pet_shaman_earthen_shield_totem for the corrected
-- mechanic understanding and the ArgusCore-native redesign (periodic scan instead of an
-- AreaTrigger, and Unit::m_Controlled lookups instead of relying on the absorb buff's own
-- caster being the totem).
--
-- Creature entry 100943 confirmed via ArgusCore's own base world DB dump - a VerifiedBuild
-- row explicitly labels entry 100943 "Earthen Shield Totem" under a genuine Legion build
-- number, not the later "Earthen Wall Totem" (patch 8.0.1+) identity a plain web lookup alone
-- would have suggested.
UPDATE `creature_template` SET `ScriptName` = 'npc_pet_shaman_earthen_shield_totem' WHERE `entry` = 100943;

DELETE FROM `spell_script_names` WHERE `spell_id` = 201633 AND `ScriptName` = 'spell_sha_earthen_shield_absorb';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(201633, 'spell_sha_earthen_shield_absorb');
