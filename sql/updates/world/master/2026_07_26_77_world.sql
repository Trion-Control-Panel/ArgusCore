-- Mage: bind three more newly-implemented, previously entirely-missing spells found while
-- continuing the class-diff sweep (Mirror Image, Frozen Orb, and Meteor). No existing
-- spell_script_names rows for any of these ScriptNames anywhere in this repo's SQL or the base
-- TDB dump. Meteor's two AreaTriggerCreatePropertiesId rows (3467, 1712) already existed in the
-- base TDB dump with empty ScriptNames - same "row exists, never bound" shape as the Priest/
-- Warrior AreaTriggers fixed earlier this pass.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_mage_mirror_image_summon', 'spell_mage_frozen_orb',
    'spell_mage_meteor', 'spell_mage_meteor_damage'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(55342, 'spell_mage_mirror_image_summon'),
(84721, 'spell_mage_frozen_orb'),
(153561, 'spell_mage_meteor'),
(153564, 'spell_mage_meteor_damage');

UPDATE `areatrigger_create_properties` SET `ScriptName` = 'at_mage_meteor_timer' WHERE `Id` = 3467;
UPDATE `areatrigger_create_properties` SET `ScriptName` = 'at_mage_meteor_burn' WHERE `Id` = 1712;
