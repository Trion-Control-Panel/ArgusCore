-- Fix NPC 2993 (Baine Bloodhoof) display ID: 3792 is incorrect, correct display is 30272.
UPDATE `creature_template_model` SET `CreatureDisplayID` = 30272
WHERE `CreatureID` = 2993
  AND `CreatureDisplayID` = 3792;
