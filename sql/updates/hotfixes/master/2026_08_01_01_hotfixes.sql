-- Hotfix-override tables for GroupFinderActivity.db2, GroupFinderActivityGrp.db2, and
-- GroupFinderCategory.db2 (Group Finder / "LFG List" premade group listings feature - see
-- ARGUSCORE_FIXES.md, LFG List opcode group). These tables are deliberately left EMPTY: the
-- primary data source is the real client .db2 files (already verified byte-for-byte against
-- this project's own GroupFinderActivity.db2 - see the DB2Structure.h comment on
-- GroupFinderActivityEntry), and these hotfix tables only ever matter if a row is later listed
-- in `hotfix_data` for override - which none are. Schema-only, matching the existing
-- achievement/achievement_locale convention for a DB2 type with LocalizedString fields.

CREATE TABLE `group_finder_activity` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `FullName` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `ShortName` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `MinGearLevelSuggestion` smallint unsigned NOT NULL DEFAULT '0',
  `MapID` smallint unsigned NOT NULL DEFAULT '0',
  `AreaID` smallint unsigned NOT NULL DEFAULT '0',
  `GroupFinderCategoryID` tinyint unsigned NOT NULL DEFAULT '0',
  `GroupFinderActivityGrpID` tinyint unsigned NOT NULL DEFAULT '0',
  `OrderIndex` tinyint NOT NULL DEFAULT '0',
  `MinLevel` tinyint unsigned NOT NULL DEFAULT '0',
  `MaxLevelSuggestion` tinyint unsigned NOT NULL DEFAULT '0',
  `DifficultyID` tinyint unsigned NOT NULL DEFAULT '0',
  `Flags` tinyint unsigned NOT NULL DEFAULT '0',
  `DisplayType` tinyint unsigned NOT NULL DEFAULT '0',
  `MaxPlayers` tinyint unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `group_finder_activity_locale` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `locale` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `FullName_lang` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `ShortName_lang` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`locale`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `group_finder_activity_grp` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `Name` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `OrderIndex` tinyint unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `group_finder_activity_grp_locale` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `locale` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `Name_lang` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`locale`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `group_finder_category` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `Name` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `Unknown1` tinyint unsigned NOT NULL DEFAULT '0',
  `Unknown2` tinyint unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `group_finder_category_locale` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `locale` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `Name_lang` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`locale`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
