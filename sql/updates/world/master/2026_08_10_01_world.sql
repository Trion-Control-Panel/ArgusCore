-- See ARGUSCORE_FIXES.md > DB Startup Error Log Triage > GameObject/NPCText referential-integrity errors (2026-08-10 startup log batch)

UPDATE `gameobject_template` SET `Data0`=1649 WHERE `entry`=185877 AND `type`=50 AND `Data0`=2864;
UPDATE `gameobject_template` SET `Data0`=1644 WHERE `entry`=185881 AND `type`=50 AND `Data0`=2856;
UPDATE `gameobject_template` SET `Data0`=0 WHERE `entry`=202357 AND `type`=2 AND `Data0`=17334;

UPDATE `gameobject_template` SET `Data1`=0 WHERE `type`=0 AND `Data1`=5708792 AND `entry` IN
    (210783,210784,210785,210786,210787,210788,210789,210800,210862,210863,210864,210866,210867);

UPDATE `gameobject_template` SET `Data1`=0 WHERE `type`=0 AND `Data1`=5701192 AND `entry` IN
    (211133,211134,211135,211136,211137,211138,211245,211246,211258,211260,211293);
