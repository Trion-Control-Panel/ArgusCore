-- Fix High Overlord Saurfang display ID: 23033 is incorrect for Legion, correct display is 14732.
UPDATE `creature_template_model` SET `CreatureDisplayID` = 14732
WHERE `CreatureID` IN (25256,25749,73716,89753,91195,91498,93773,93791,93795,100636,103663,109071,110519,111105,111106,111115,112590)
  AND `CreatureDisplayID` = 23033;
