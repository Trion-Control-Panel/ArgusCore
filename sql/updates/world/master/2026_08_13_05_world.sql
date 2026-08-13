-- See ARGUSCORE_FIXES.md for details.
UPDATE `creature_template` SET `unit_flags` = `unit_flags` & 0x2002340 WHERE (`unit_flags` & ~0x2002340) != 0;
UPDATE `creature_template` SET `unit_flags2` = `unit_flags2` & 0x4034823 WHERE (`unit_flags2` & ~0x4034823) != 0;
UPDATE `creature_template` SET `unit_flags3` = `unit_flags3` & 0x14de0b6 WHERE (`unit_flags3` & ~0x14de0b6) != 0;

UPDATE `creature` SET `unit_flags` = `unit_flags` & 0x2002340 WHERE `unit_flags` IS NOT NULL AND (`unit_flags` & ~0x2002340) != 0;
UPDATE `creature` SET `unit_flags2` = `unit_flags2` & 0x4034823 WHERE `unit_flags2` IS NOT NULL AND (`unit_flags2` & ~0x4034823) != 0;
UPDATE `creature` SET `unit_flags3` = `unit_flags3` & 0x14de0b6 WHERE `unit_flags3` IS NOT NULL AND (`unit_flags3` & ~0x14de0b6) != 0;

DELETE FROM `creature_template_model` WHERE `CreatureID` = 29152;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `Probability`, `VerifiedBuild`) VALUES
(29152, 0, 5008, 1, 0),
(29152, 1, 5006, 1, 0),
(29152, 2, 5005, 1, 0),
(29152, 3, 5003, 1, 0);
