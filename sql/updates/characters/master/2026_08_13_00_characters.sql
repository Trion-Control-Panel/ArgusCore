-- See ARGUSCORE_FIXES.md for details.

-- BattlePay Phase 2: per-purchase audit trail. Realm-local (characters DB, not world/auth) because
-- delivery (bags/mailbox) is realm-local, even though the Shop Points balance itself is account-wide
-- (auth DB). DeliveryState is the crash-recovery ledger for the deliver-then-deduct purchase flow:
-- 0 = delivered, balance not yet deducted (should always resolve to 1 within the same transaction
--     chain - a row stuck at 0 indicates the server died mid-purchase; nothing to reconcile, since
--     delivery and this row are written in the same characters-DB transaction)
-- 1 = delivered and paid for (the normal end state)
-- 2 = delivered but the balance deduction failed (a race with a concurrent external credit-queue
--     change reduced the balance between the initial check and the deduct) - the server "ate" the
--     cost rather than risk taking currency without delivering; flagged for admin reconciliation.
CREATE TABLE IF NOT EXISTS `battlepay_purchase_log` (
  `Id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  `BattlenetAccountId` INT UNSIGNED NOT NULL,
  `AccountId` INT UNSIGNED NOT NULL COMMENT 'game account (WorldSession::GetAccountId)',
  `CharacterGuid` BIGINT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'ObjectGuid::LowType is uint64 - 0 if delivered by mail to an offline TargetCharacter whose guid was only known from the client packet',
  `ProductID` INT UNSIGNED NOT NULL,
  `PurchaseID` BIGINT UNSIGNED NOT NULL COMMENT 'BattlePayMgr-generated id, unique - used to update DeliveryState later without needing the AUTO_INCREMENT Id back across an async transaction boundary',
  `PricePaid` BIGINT UNSIGNED NOT NULL COMMENT 'Shop Points, denormalized at purchase time so later price changes do not rewrite history',
  `DeliveryState` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `CreatedAt` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`Id`),
  UNIQUE KEY `uk_purchase_id` (`PurchaseID`),
  KEY `idx_character` (`CharacterGuid`),
  KEY `idx_bnet_account` (`BattlenetAccountId`),
  KEY `idx_delivery_state` (`DeliveryState`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='BattlePay: per-purchase audit trail';
