-- See ARGUSCORE_FIXES.md for details.
UPDATE `gameobject_template` SET `data1` = 0 WHERE `entry` IN
(215130,215363,215364,215365,215366,215367,215368,215369,215370,215371,215372,215375,215376,215379,215380,215381,215382,
 215391,215459,215760,215787,216003,216004,216005,216006,216007,216008,216009,216060,
 216354,216355,216356,216357,216358,216359,216698,216699);

UPDATE `gameobject_template` SET `data0` = 0 WHERE `entry` IN
(215022,215029,215030,215177,215178,215179,215180,215383,215384,215385,215386);

UPDATE `gameobject_template` SET `type` = 10, `data0` = 0, `data1` = 2147483647, `data2` = 0, `data3` = 0, `data4` = 0 WHERE `entry` = 239423;
UPDATE `gameobject_template` SET `type` = 10, `data0` = 0, `data1` = 2147483647, `data2` = 0, `data3` = 0, `data4` = 1 WHERE `entry` = 239424;

UPDATE `gameobject_template` SET `data0` = 173162 WHERE `entry` = 231620;

DELETE FROM `destructible_hitpoint` WHERE `Id` IN (84, 85, 86, 87, 88, 89, 99);
INSERT INTO `destructible_hitpoint` (`Id`, `IntactNumHits`, `DamagedNumHits`) VALUES
(84, 0, 0), (85, 0, 0), (86, 0, 0), (87, 0, 0), (88, 0, 0), (89, 0, 0), (99, 0, 0);
