-- See ARGUSCORE_FIXES.md > DB Startup Error Log Triage > LoadCreatures() startup errors: map 720 spawnDifficulties + creature unit_flags corruption (2026-08-11 startup log batch)

UPDATE `creature` SET `spawnDifficulties` = '3,4,5,6' WHERE `map` = 720 AND `spawnDifficulties` = '14,15,33';

UPDATE `creature` SET `unit_flags` = NULL WHERE `guid` IN (314945,314948,314952,314962,314965,315003,315004,315005,315289,315292,315293,315305);
