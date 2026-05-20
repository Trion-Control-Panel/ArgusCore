-- Fix Val'kyr Shadowguard display ID: 25452 is incorrect, correct display is 67146.
UPDATE `creature_template_model` SET `CreatureDisplayID` = 67146
WHERE `CreatureID` IN (39120,39121,39122)
  AND `CreatureDisplayID` = 25452;
