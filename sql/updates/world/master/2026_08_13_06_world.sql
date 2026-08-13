-- See ARGUSCORE_FIXES.md for details.
UPDATE `gameobject_template` SET `data1` = 0 WHERE `type` = 0 AND `data1` IN (5705968,5743776,5749660,5755148,5756548);
UPDATE `gameobject_template` SET `data0` = 0 WHERE `type` = 3 AND `data0` = 7;
UPDATE `gameobject_template` SET `data0` = 0 WHERE `type` = 50 AND `data0` = 2858;
UPDATE `gameobject_template` SET `data0` = 0 WHERE `type` = 8 AND `data0` IN (5700984,5705760,5743620,5749504,5756392);
