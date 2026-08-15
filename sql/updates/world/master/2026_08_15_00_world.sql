-- See ARGUSCORE_FIXES.md for details.

-- BattlePay: a real product row for the character-select Character Boost credit
-- (battlepay_pending_boost) to reference. WorldSession::SendDisplayPromo needs a genuine,
-- DisplayInfo-backed product to embed in the distribution object it sends - the client's
-- "you have a boost credit" recognition (C_SharedCharacterServices.GetUpgradeDistributions) appears
-- to require this nested product data, not just a bare ProductID number (confirmed by testing: a
-- ProductID with no real product behind it did not make the Boost option appear).
--
-- No battlepay_product_item rows - delivery for this product is NOT the normal item-delivery path
-- (ProductFilter/DeliverAndDeduct); it's handled entirely by BattlePayMgr::ApplyCharacterBoostOffline,
-- triggered separately via the battlepay_pending_boost/DistributionAssignToTarget flow. This row exists
-- purely so the client has something real to look up and display.

DELETE FROM `battlepay_display_info` WHERE `DisplayInfoId` = 200;
INSERT INTO `battlepay_display_info` (`DisplayInfoId`, `Name1`, `Name3`) VALUES
(200, 'Character Boost', 'Instantly level up a character, fully geared and ready to go.');

DELETE FROM `battlepay_product` WHERE `ProductID` = 200;
INSERT INTO `battlepay_product` (`ProductID`, `Type`, `WebsiteType`, `DisplayInfoID`, `ShopPointsPrice`) VALUES
(200, 0, 29, 200, 0); -- WebsiteType 29 = CharacterBoost (Battlepay::WebsiteType in BattlePayData.h)
