-- See ARGUSCORE_FIXES.md for details.

-- Missing graveyard link for Zone 6457 (New Tinkertown, the Gnome starting-experience revamp zone -
-- confirmed as playercreateinfo's own Race 7/Class * starting position, map 0). Every sibling starting
-- zone in this same content block (6450 Teldrassil/Aldrassil, 6451 Durotar/Valley of Trials, 6452
-- Mulgore/Red Cloud Mesa, 6453 Durotar/Echo Isles, 6454 Tirisfal/Deathknell, 6455 Eversong/Sunstrider
-- Isle, 6456 Azuremyst/Crash Site) already has exactly one graveyard_zone row plus a matching
-- CONDITION_SOURCE_TYPE_GRAVEYARD/CONDITION_TEAM row restricting it to that zone's own faction - 6457
-- was the only one missing both, which meant a Gnome dying there fell all the way back to the hardcoded
-- default Alliance graveyard (Westfall, ObjectMgr::GetDefaultGraveyard) instead of somewhere sensible
-- nearby. No reference-core data exists for this specific zone (not present in DestinyCore/AshamaneCore
-- SQL history - likely added to this build's client data after those forks), so the graveyard chosen
-- (1881, "Dun Morogh, Gnomeregan (Micro) GY") is not reference-verified the way most fixes in this file
-- are - it's the only Gnomeregan-specific graveyard already in this build's data, and New Tinkertown is
-- the Gnomeregan-adjacent starting-zone revamp, so it's a reasoned choice, not a guess at IDs that don't
-- exist. Reusing an existing graveyard for a second zone (it already serves zone 1, Dun Morogh) is normal,
-- not a conflict - AddGraveyardLink keys on (safeLocId, zoneId) pairs, not safeLocId alone.

DELETE FROM `graveyard_zone` WHERE `GhostZone` = 6457;
INSERT INTO `graveyard_zone` (`ID`, `GhostZone`, `Comment`) VALUES
(1881, 6457, 'Dun Morogh, Gnomeregan (Micro) GY - New Tinkertown');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 27 AND `SourceGroup` = 6457;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(27, 6457, 1881, 0, 0, 6, 0, 469, 0, 0, '', 0, 0, 0, '', 'Graveyard - 1881 - 6457 - Dun Morogh, Gnomeregan (Micro) GY - New Tinkertown - Team Alliance');
