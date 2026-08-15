-- See ARGUSCORE_FIXES.md for details.

-- SkillRaceClassInfo is missing a Monk row for skill 118 (Dual Wield), even though SkillLineAbility
-- (the table mapping spells to skill-line permissions) ties Dual Wield to spell 674 with
-- ClassMask = 2573 = Warrior|Hunter|Rogue|Monk|Demon Hunter - Monk genuinely belongs in this group,
-- confirmed by direct user report (dual-wielding two weapons works in practice), but SkillRaceClassInfo
-- itself (the stricter table Player::_LoadSkills/LearnDefaultSkills actually enforce) only has 4 rows
-- for skill 118, covering Hunter+Rogue/Shaman/Death Knight/Demon Hunter - Monk is absent from that
-- specific table, causing every Monk with skill 118 in character_skills (however it got there - not
-- necessarily LearnDefaultSkills, see the aside in ARGUSCORE_FIXES.md) to be flagged
-- "forbidden skill ... for his race/class combination" and dropped on every login.
--
-- Unlike most fixes in this file, this row is NOT a recovered real Blizzard value - none of
-- DestinyCore/AshamaneCore/WowPacketParser carry SkillRaceClassInfo data, and this table isn't the
-- kind of thing exposed in packet captures. It's constructed to mirror the shape of the existing
-- Hunter+Rogue row (RaceMask -1/all races, Flags 146, SkillTierID 0, Availability 1, MinLevel 1),
-- swapping ClassMask to Monk only (512 = 1 << 9), on the strength of the SkillLineAbility corroboration
-- above plus the direct gameplay confirmation. ID deliberately far outside any real SkillRaceClassInfo
-- ID range seen in this build's data (existing rows: 131/883/913/1661) so it reads as clearly synthetic,
-- not a guessed-at real Blizzard record. VerifiedBuild matches this project's existing hotfix precedent
-- (2026_08_11_01_hotfixes.sql).

INSERT IGNORE INTO `skill_race_class_info` (`ID`, `RaceMask`, `SkillID`, `Flags`, `SkillTierID`, `Availability`, `MinLevel`, `ClassMask`, `VerifiedBuild`) VALUES
(900118, -1, 118, 146, 0, 1, 1, 512, 26635);
