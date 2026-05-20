-- Fix Baine Bloodhoof display ID: 3792 is incorrect, correct display is 66086.
UPDATE `creature_template_model` SET `CreatureDisplayID` = 66086
WHERE `CreatureID` IN (90710,117121)
  AND `CreatureDisplayID` = 3792;
