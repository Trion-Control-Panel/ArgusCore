-- Fix NPC 51467 (Moonkin Hatchling) display ID: 37413 is incorrect, correct display is 37526.
UPDATE `creature_template_model` SET `CreatureDisplayID` = 37526
WHERE `CreatureID` = 51467
  AND `CreatureDisplayID` = 37413;
