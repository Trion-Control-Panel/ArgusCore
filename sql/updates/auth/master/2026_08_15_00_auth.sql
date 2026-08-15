-- See ARGUSCORE_FIXES.md for details.

-- BattlePay: account-wide, unredeemed Character Boost credits - the character-select "Boost" entry
-- point (distinct from the in-game ApplyCharacterBoost purchase flow). A row here is what makes
-- WorldSession::SendDisplayPromo's distribution list report a real, redeemable credit instead of the
-- placeholder it used to send. `Id` doubles as the wire-protocol DistributionID (no separate ID
-- generation needed - it's already unique and persistent). Granted via `.battlepay grantboost <account>
-- <level>` or directly by an external site's own database access (same no-webhook precedent as
-- battlepay_account_balance) - INSERT INTO battlepay_pending_boost (BattlenetAccountId, ProductID,
-- TargetLevel) VALUES (?, ?, ?). Redeemed (RedeemedAt/RedeemedCharacterGuid set) the moment the player
-- picks a target character at character select - can't be reused after that.
CREATE TABLE IF NOT EXISTS `battlepay_pending_boost` (
  `Id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  `BattlenetAccountId` INT UNSIGNED NOT NULL,
  `ProductID` INT UNSIGNED NOT NULL,
  `TargetLevel` TINYINT UNSIGNED NOT NULL,
  `CreatedAt` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `RedeemedAt` TIMESTAMP NULL DEFAULT NULL,
  `RedeemedCharacterGuid` BIGINT UNSIGNED NULL DEFAULT NULL,
  PRIMARY KEY (`Id`),
  INDEX `idx_bpay_pending_boost_account` (`BattlenetAccountId`),
  CONSTRAINT `fk_bpay_pending_boost_bnet` FOREIGN KEY (`BattlenetAccountId`) REFERENCES `battlenet_accounts` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='BattlePay: unredeemed Character Boost credits, redeemable at character select';
