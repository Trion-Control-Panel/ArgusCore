-- See ARGUSCORE_FIXES.md > DB Startup Error Log Triage > GameObject/NPCText referential-integrity errors (2026-08-10 startup log batch)

DELETE FROM `gameobject_template_addon` WHERE `entry` IN (280939,280940,280941);

DELETE FROM `transports` WHERE `guid`=35 AND `entry`=278407;
