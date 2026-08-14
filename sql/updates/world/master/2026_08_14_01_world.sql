-- See ARGUSCORE_FIXES.md for details.

-- BattlePay: the original Phase 1 seed for `battlepay_shop_entry` never set `DisplayInfoID` (column
-- exists, just wasn't in the INSERT's column list), so it stayed at its default 0 for all 3 rows.
-- `BattlePayMgr::SendProductList()` builds the "Shop" array (WorldPackets::BattlePay::
-- BattlePayShopEntry) from this table, and calls WriteDisplayInfo(entry.DisplayInfoID, ...)
-- independently of the "ProductInfo"/"Product" arrays (which use battlepay_product.DisplayInfoID,
-- already correct) - with DisplayInfoID=0, WriteDisplayInfo always returns "no display info" for the
-- Shop array specifically, regardless of the icon/model fix in 2026_08_14_00_world.sql. If the
-- client's product-tile grid renders from this Shop array (suspected but not proven), its per-entry
-- DisplayInfo would always have been empty. Filling it in with the same DisplayInfoId already used by
-- each row's product, to close this gap alongside the earlier icon/model fix.

UPDATE `battlepay_shop_entry` SET `DisplayInfoID` = 1 WHERE `ProductID` = 1; -- Jeweled Onyx Panther
UPDATE `battlepay_shop_entry` SET `DisplayInfoID` = 2 WHERE `ProductID` = 2; -- Reins of the Grove Warden
UPDATE `battlepay_shop_entry` SET `DisplayInfoID` = 3 WHERE `ProductID` = 3; -- Snowball
