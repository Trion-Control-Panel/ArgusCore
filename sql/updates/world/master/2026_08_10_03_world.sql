-- See ARGUSCORE_FIXES.md > DB Startup Error Log Triage > GameObject/NPCText referential-integrity errors (2026-08-10 startup log batch)

UPDATE `gameobject_template` SET
    `type`=6, `displayId`=5333, `name`='Snow Pile', `IconName`='', `castBarCaption`='', `unk1`='',
    `size`=4, `Data0`=0, `Data1`=0, `Data2`=0, `Data3`=0, `Data4`=0, `Data5`=0, `Data6`=0, `Data7`=0,
    `Data8`=0, `Data9`=0, `Data10`=0, `Data11`=0, `Data12`=0, `Data13`=0, `Data14`=0, `Data15`=0,
    `Data16`=0, `Data17`=0, `Data18`=0, `Data19`=0, `Data20`=0, `Data21`=0, `Data22`=0, `Data23`=0,
    `Data24`=0, `Data25`=0, `Data26`=0, `Data27`=0, `Data28`=0, `Data29`=0, `Data30`=0, `Data31`=0,
    `Data32`=0, `VerifiedBuild`=15595
    WHERE `entry`=188187 AND `type`<>6;
