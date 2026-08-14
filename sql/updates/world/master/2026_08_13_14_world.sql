-- See ARGUSCORE_FIXES.md for details.

-- Phase 2: collapse the 3-currency pricing model (GoldPrice/VotePointsPrice/DonatePointsPrice) into
-- a single account-wide Shop Points price. The real 7.3.5 client purchase flow
-- (C_StoreSecure.PurchaseProduct/PurchaseProductConfirm) has no currency-selector field at all, so
-- per-purchase currency choice was never actually possible through the native Store UI.

-- Plain ADD/DROP COLUMN (no IF [NOT] EXISTS) - that clause isn't reliably supported across the
-- MySQL/MariaDB versions this might run against. The DB updater tool tracks applied migrations
-- itself and won't re-run this file under normal use, so idempotency here isn't required the way
-- it is for files sometimes applied directly via a raw mysql client.
ALTER TABLE `battlepay_product`
  ADD COLUMN `ShopPointsPrice` BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER `CustomValue`;

UPDATE `battlepay_product` SET `ShopPointsPrice` = 500  WHERE `ProductID` = 1; -- Jeweled Onyx Panther
UPDATE `battlepay_product` SET `ShopPointsPrice` = 1000 WHERE `ProductID` = 2; -- Reins of the Grove Warden
UPDATE `battlepay_product` SET `ShopPointsPrice` = 50   WHERE `ProductID` = 3; -- Snowball

ALTER TABLE `battlepay_product`
  DROP COLUMN `GoldPrice`,
  DROP COLUMN `VotePointsPrice`,
  DROP COLUMN `DonatePointsPrice`;
