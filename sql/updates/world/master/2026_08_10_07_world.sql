-- See ARGUSCORE_FIXES.md > DB Startup Error Log Triage > GameObject/NPCText referential-integrity errors (2026-08-10 startup log batch)

INSERT IGNORE INTO `destructible_hitpoint` (`Id`, `IntactNumHits`, `DamagedNumHits`) VALUES
(2, 0, 0),
(7, 0, 0),
(17, 0, 0),
(19, 0, 0),
(20, 0, 0);
