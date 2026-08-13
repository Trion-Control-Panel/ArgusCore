-- See ARGUSCORE_FIXES.md for details.
UPDATE `creature_template_model` SET `CreatureDisplayID` = 18342 WHERE `CreatureID` = 18604 AND `Idx` = 0;
UPDATE `creature_template_model` SET `CreatureDisplayID` = 71757 WHERE `CreatureID` = 20267 AND `Idx` = 0;
UPDATE `creature_template_model` SET `CreatureDisplayID` = 16217 WHERE `CreatureID` = 24692 AND `Idx` = 0;
UPDATE `creature_template_model` SET `CreatureDisplayID` = 16217 WHERE `CreatureID` = 25544 AND `Idx` = 0;
UPDATE `creature_template_model` SET `CreatureDisplayID` = 42775 WHERE `CreatureID` = 30426 AND `Idx` = 0;
UPDATE `creature_template_model` SET `CreatureDisplayID` = 42775 WHERE `CreatureID` = 30427 AND `Idx` = 0;
UPDATE `creature_template_model` SET `CreatureDisplayID` = 42775 WHERE `CreatureID` = 30428 AND `Idx` = 0;
UPDATE `creature_template_model` SET `CreatureDisplayID` = 67156 WHERE `CreatureID` = 38258 AND `Idx` = 0;
UPDATE `creature_template_model` SET `CreatureDisplayID` = 60753 WHERE `CreatureID` = 88133 AND `Idx` = 0;
UPDATE `creature_template_model` SET `CreatureDisplayID` = 66086 WHERE `CreatureID` = 90710 AND `Idx` = 0;
UPDATE `creature_template_model` SET `CreatureDisplayID` = 14732 WHERE `CreatureID` = 103663 AND `Idx` = 0;
UPDATE `creature_template_model` SET `CreatureDisplayID` = 14732 WHERE `CreatureID` = 111115 AND `Idx` = 0;
UPDATE `creature_template_model` SET `CreatureDisplayID` = 66086 WHERE `CreatureID` = 117121 AND `Idx` = 0;

DELETE FROM `creature_template_model` WHERE `CreatureID` IN
(109071,110519,111105,111106,112590,89753,91195,91498,93773,93791,93795,100636,73716,29152);
