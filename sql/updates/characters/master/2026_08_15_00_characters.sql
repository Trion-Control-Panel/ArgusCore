-- See ARGUSCORE_FIXES.md for details.

-- Legion personal-rating arena system: replaces the old persistent Arena Team system (arena_team /
-- arena_team_member, left in place below, unreferenced from now on - Blizzard removed persistent
-- arena teams in the Warlords of Draenor pre-patch; ArgusCore is Legion 7.3.5). One row per
-- MAX_PVP_SLOT (6) per character, rewritten in full (DELETE + re-INSERT) on every character save.
CREATE TABLE IF NOT EXISTS `character_arena_data` (
  `guid` BIGINT UNSIGNED NOT NULL,
  `slot` TINYINT UNSIGNED NOT NULL,
  `rating` INT UNSIGNED NOT NULL DEFAULT '0',
  `bestRatingOfWeek` INT UNSIGNED NOT NULL DEFAULT '0',
  `bestRatingOfSeason` INT UNSIGNED NOT NULL DEFAULT '0',
  `matchMakerRating` INT UNSIGNED NOT NULL DEFAULT '0',
  `weekGames` INT UNSIGNED NOT NULL DEFAULT '0',
  `weekWins` INT UNSIGNED NOT NULL DEFAULT '0',
  `prevWeekGames` INT UNSIGNED NOT NULL DEFAULT '0',
  `prevWeekWins` INT UNSIGNED NOT NULL DEFAULT '0',
  `seasonGames` INT UNSIGNED NOT NULL DEFAULT '0',
  `seasonWins` INT UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`slot`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Legion personal arena/RBG rating, one row per MAX_PVP_SLOT per character';
