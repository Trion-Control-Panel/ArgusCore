-- See ARGUSCORE_FIXES.md > DB Startup Error Log Triage > GameObject/NPCText referential-integrity errors (2026-08-10 startup log batch)

UPDATE `npc_text` SET `Probability0` = 0
WHERE `ID` IN (2953, 2955, 2956, 2957, 7114, 8321, 8332, 10128, 11128, 12181, 13157, 14330)
  AND `Probability0` = 1 AND `BroadcastTextID0` = 0;

UPDATE `npc_text` SET `Probability1` = 0, `Probability2` = 0, `Probability3` = 0, `Probability4` = 0,
    `Probability5` = 0, `Probability6` = 0, `Probability7` = 0
WHERE `ID` = 11428 AND `Probability0` = 1 AND `BroadcastTextID0` = 22480
  AND (`Probability1` = 1 OR `Probability2` = 1 OR `Probability3` = 1 OR `Probability4` = 1
       OR `Probability5` = 1 OR `Probability6` = 1 OR `Probability7` = 1);
