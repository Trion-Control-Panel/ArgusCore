-- Fix NPC 32315 (High Overlord Saurfang) display ID: 27905 is incorrect, correct display is 14732.
UPDATE `creature_template_model` SET `CreatureDisplayID` = 14732
WHERE `CreatureID` = 32315
  AND `CreatureDisplayID` = 27905;
