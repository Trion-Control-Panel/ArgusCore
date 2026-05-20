-- Fix NPC 11878 (Nathanos Blightcaller) display ID: 11814 is incorrect, correct display is 66644.
UPDATE `creature_template_model` SET `CreatureDisplayID` = 66644
WHERE `CreatureID` = 11878
  AND `CreatureDisplayID` = 11814;
