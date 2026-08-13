-- See ARGUSCORE_FIXES.md for details.
DELETE FROM `graveyard_zone` WHERE `ID` IN (5082,5083,5119,5140,5188,5284) AND `GhostZone` = 7705;
INSERT INTO `graveyard_zone` (`ID`, `GhostZone`, `Comment`) VALUES
(5082, 7705, '7.0 DH-Mardum - (01) Start'),
(5083, 7705, '7.0 DH-Mardum - (03) Seat of Command'),
(5119, 7705, '7.0 DH-Mardum - (04) Illidari Foothold'),
(5140, 7705, '7.0 DH-Mardum - (05) Volcano'),
(5188, 7705, '7.0 DH-Mardum - (06) The Fel Hammer'),
(5284, 7705, '7.0 DH-Mardum - (02) Molten Shore');
