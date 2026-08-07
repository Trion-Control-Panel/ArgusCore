-- Fix 8 stale "tlpp_"-prefixed spell_script_names rows left over from a past content merge.
-- RegisterSpellScript/RegisterSpellAndAuraScriptPair stringify the bare class name, so these
-- prefixed names never matched any registered script. 6 of them had no correct row at all,
-- meaning those abilities ran with zero script logic (Heroic Leap's missing damage cast was one
-- symptom of this); the other 2 (187880, 201845) already had a correct row alongside the dead one.
DELETE FROM `spell_script_names` WHERE `ScriptName` IN
(
    'tlpp_spell_warr_heroic_leap',
    'tlpp_spell_warr_ignore_pain',
    'tlpp_spell_mage_ice_lance',
    'tlpp_spell_mage_ice_lance_damage',
    'tlpp_spell_sha_feral_spirit',
    'tlpp_spell_sha_healing_surge',
    'tlpp_spell_sha_maelstrom_weapon',
    'tlpp_spell_sha_stormbringer'
);

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(6544, 'spell_warr_heroic_leap'),
(190456, 'spell_warr_ignore_pain'),
(30455, 'spell_mage_ice_lance'),
(228598, 'spell_mage_ice_lance_damage'),
(51533, 'spell_sha_feral_spirit'),
(188070, 'spell_sha_healing_surge');

-- Warrior: Heroic Leap's jump/damage sub-spell id was also wrong (178368, which has no landing-
-- damage trigger at all) - the real id is 94954 ("[DND] Cosmetic Heroic Leap (Dest)"), whose own
-- native TRIGGER_MISSILE effect casts the damage automatically. spell_warr_heroic_leap_jump was
-- a manual-cast workaround for the wrong id and has been removed; unbind its stale row.
DELETE FROM `spell_script_names` WHERE `spell_id` = 178368 AND `ScriptName` = 'spell_warr_heroic_leap_jump';
