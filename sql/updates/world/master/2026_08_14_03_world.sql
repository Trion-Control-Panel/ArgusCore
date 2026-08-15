-- See ARGUSCORE_FIXES.md for details.

-- BattlePay: lets a product bundle a free Character Boost alongside whatever it normally delivers
-- (e.g. buy an item bundle, the target character also gets boosted to CustomValue's level). Reuses
-- the existing CustomValue column (already documented in Battlepay::Product as "CharacterBoost ->
-- target level") rather than adding a second column for the target level. Independent of WebsiteType -
-- this is a bonus bolted onto any product, not a distinct product category. See
-- src/server/game/BattlePay/README.md for how to use this.
ALTER TABLE `battlepay_product`
  ADD COLUMN `GrantsBoost` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `CustomValue`;
