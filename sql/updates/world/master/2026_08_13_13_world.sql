-- See ARGUSCORE_FIXES.md for details.

CREATE TABLE IF NOT EXISTS `battlepay_display_info` (
  `DisplayInfoId` INT UNSIGNED NOT NULL,
  `CreatureDisplayInfoID` INT UNSIGNED NOT NULL DEFAULT 0,
  `FileDataID` INT UNSIGNED NOT NULL DEFAULT 0,
  `Flags` INT UNSIGNED NOT NULL DEFAULT 0,
  `Name1` VARCHAR(128) NOT NULL DEFAULT '',
  `Name2` VARCHAR(128) NOT NULL DEFAULT '',
  `Name3` VARCHAR(128) NOT NULL DEFAULT '',
  `Name4` VARCHAR(128) NOT NULL DEFAULT '',
  PRIMARY KEY (`DisplayInfoId`)
);

CREATE TABLE IF NOT EXISTS `battlepay_display_info_locales` (
  `Id` INT UNSIGNED NOT NULL,
  `Locale` VARCHAR(4) NOT NULL,
  `Name1` VARCHAR(128) NULL,
  `Name2` VARCHAR(128) NULL,
  `Name3` VARCHAR(128) NULL,
  `Name4` VARCHAR(128) NULL,
  PRIMARY KEY (`Id`, `Locale`)
);

CREATE TABLE IF NOT EXISTS `battlepay_display_info_visuals` (
  `DisplayInfoId` INT UNSIGNED NOT NULL,
  `DisplayId` INT UNSIGNED NOT NULL DEFAULT 0,
  `VisualId` INT UNSIGNED NOT NULL DEFAULT 0,
  `ProductName` VARCHAR(128) NOT NULL DEFAULT '',
  PRIMARY KEY (`DisplayInfoId`, `DisplayId`)
);

CREATE TABLE IF NOT EXISTS `battlepay_product` (
  `ProductID` INT UNSIGNED NOT NULL,
  `Type` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `ChoiceType` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `Flags` INT UNSIGNED NOT NULL DEFAULT 0,
  `DisplayInfoID` INT UNSIGNED NOT NULL DEFAULT 0,
  `ScriptName` VARCHAR(64) NOT NULL DEFAULT '',
  `ClassMask` INT UNSIGNED NOT NULL DEFAULT 0,
  `WebsiteType` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  -- Fix vs. reference: reference's CustomValue is declared but never populated from any DB
  -- column, silently breaking Gold/Level delivery. Here it's a real, loaded column.
  `CustomValue` INT UNSIGNED NOT NULL DEFAULT 0,
  -- Deviation from reference: three independent prices (0 = not offered via that currency)
  -- replace the reference's single NormalPriceFixedPoint/CurrentPriceFixedPoint, so a player can
  -- pay with Gold, Vote Points, or Donate Points.
  `GoldPrice` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `VotePointsPrice` INT UNSIGNED NOT NULL DEFAULT 0,
  `DonatePointsPrice` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`ProductID`)
);

CREATE TABLE IF NOT EXISTS `battlepay_product_item` (
  `ID` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `ProductID` INT UNSIGNED NOT NULL,
  `ItemID` INT UNSIGNED NOT NULL DEFAULT 0,
  `Quantity` INT UNSIGNED NOT NULL DEFAULT 1,
  `DisplayID` INT UNSIGNED NOT NULL DEFAULT 0,
  `PetResult` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`ID`),
  INDEX `idx_product` (`ProductID`)
);

CREATE TABLE IF NOT EXISTS `battlepay_product_group` (
  -- NOTE: GroupID = 22 is CLIENT-HARDCODED as the Services/character-boost tab. Never assign it
  -- to other content.
  `GroupID` INT UNSIGNED NOT NULL,
  `Name` VARCHAR(64) NOT NULL DEFAULT '',
  `IconFileDataID` INT UNSIGNED NOT NULL DEFAULT 0,
  `DisplayType` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `Ordering` SMALLINT NOT NULL DEFAULT 0,
  PRIMARY KEY (`GroupID`)
);

CREATE TABLE IF NOT EXISTS `battlepay_product_group_locales` (
  `Id` INT UNSIGNED NOT NULL,
  `Locale` VARCHAR(4) NOT NULL,
  `Name` VARCHAR(64) NULL,
  PRIMARY KEY (`Id`, `Locale`)
);

CREATE TABLE IF NOT EXISTS `battlepay_shop_entry` (
  `EntryID` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `GroupID` INT UNSIGNED NOT NULL,
  `ProductID` INT UNSIGNED NOT NULL,
  `Ordering` SMALLINT NOT NULL DEFAULT 0,
  `Flags` INT UNSIGNED NOT NULL DEFAULT 0,
  `BannerType` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `DisplayInfoID` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`EntryID`),
  INDEX `idx_group` (`GroupID`),
  INDEX `idx_product` (`ProductID`)
);

-- Seed data: a couple of real, verified Legion items (confirmed present in this build's
-- ItemSparse.csv export) for Phase 1 verification (product list display only, not purchasable
-- yet - the purchase flow is added in a later phase).

DELETE FROM `battlepay_product_group` WHERE `GroupID` IN (1, 10);
INSERT INTO `battlepay_product_group` (`GroupID`, `Name`, `IconFileDataID`, `DisplayType`, `Ordering`) VALUES
(1, 'Mounts', 132261, 0, 0),
(10, 'Toys', 237429, 0, 1);

DELETE FROM `battlepay_display_info` WHERE `DisplayInfoId` IN (1, 2, 3);
INSERT INTO `battlepay_display_info` (`DisplayInfoId`, `Name1`, `Name3`) VALUES
(1, 'Jeweled Onyx Panther', 'A shimmering onyx panther mount.'),
(2, 'Reins of the Grove Warden', 'A loyal warden of the grove, ready to carry you into battle.'),
(3, 'Snowball', 'Throw it at a friend.');

DELETE FROM `battlepay_product` WHERE `ProductID` IN (1, 2, 3);
INSERT INTO `battlepay_product` (`ProductID`, `Type`, `WebsiteType`, `DisplayInfoID`, `GoldPrice`, `VotePointsPrice`, `DonatePointsPrice`) VALUES
(1, 0, 21, 1, 0,        50,  0),   -- Jeweled Onyx Panther - Vote Points only
(2, 0, 21, 2, 500000,   0,   250), -- Reins of the Grove Warden - Gold or Donate Points
(3, 0, 3,  3, 1000,     5,   0);   -- Snowball - Gold or Vote Points

DELETE FROM `battlepay_product_item` WHERE `ProductID` IN (1, 2, 3);
INSERT INTO `battlepay_product_item` (`ProductID`, `ItemID`, `Quantity`) VALUES
(1, 82453, 1),
(2, 128422, 1),
(3, 17202, 5);

DELETE FROM `battlepay_shop_entry` WHERE `ProductID` IN (1, 2, 3);
INSERT INTO `battlepay_shop_entry` (`GroupID`, `ProductID`, `Ordering`) VALUES
(1, 1, 0),
(1, 2, 1),
(10, 3, 0);
