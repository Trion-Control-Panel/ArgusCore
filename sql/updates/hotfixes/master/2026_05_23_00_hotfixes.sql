-- Whirlwind (190411) - Fury Warrior: add 30 Rage cost missing from DB2 data.
-- ManaCost stores rage at 10x internal scale, so 30 rage = 300.
-- PowerType 1 = POWER_RAGE. ID matches SpellID for easy lookup.
DELETE FROM `spell_power` WHERE `ID` = 190411 AND `SpellID` = 190411;
INSERT INTO `spell_power` (`ManaCost`, `PowerCostPct`, `PowerPctPerSecond`, `RequiredAuraSpellID`, `PowerCostMaxPct`, `OrderIndex`, `PowerType`, `ID`, `ManaCostPerLevel`, `ManaPerSecond`, `OptionalCost`, `PowerDisplayID`, `AltPowerBarID`, `SpellID`, `VerifiedBuild`) VALUES
(300, 0, 0, 0, 0, 0, 1, 190411, 0, 0, 0, 0, 0, 190411, 1);
