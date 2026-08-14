-- See ARGUSCORE_FIXES.md for details.

-- BattlePay Phase 2: account-wide Shop Points balance, lazily created (INSERT ... ON DUPLICATE KEY
-- UPDATE) on first credit rather than pre-populated for every account.
--
-- No in-server crediting queue/poller exists for this table on purpose - any external site with its
-- own database access should write to this table directly, e.g.:
--   INSERT INTO battlepay_account_balance (BattlenetAccountId, Balance) VALUES (?, ?)
--     ON DUPLICATE KEY UPDATE Balance = Balance + VALUES(Balance);
-- (safe to call repeatedly - no separate "does a row exist yet" check needed). Still deliberately
-- NOT a webhook/HTTP endpoint on this server (security preference: no new network-facing surface) -
-- the external site's own code/database access is the only integration point.
CREATE TABLE IF NOT EXISTS `battlepay_account_balance` (
  `BattlenetAccountId` INT UNSIGNED NOT NULL,
  `Balance` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `UpdatedAt` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`BattlenetAccountId`),
  CONSTRAINT `fk_bpay_balance_bnet` FOREIGN KEY (`BattlenetAccountId`) REFERENCES `battlenet_accounts` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='BattlePay: account-wide Shop Points balance';
