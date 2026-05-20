-- Fix NPC 2052 display ID: 908 is incorrect, correct display is 229.
UPDATE `creature_template_model` SET `CreatureDisplayID` = 229
WHERE `CreatureID` = 2052
  AND `CreatureDisplayID` = 908;
