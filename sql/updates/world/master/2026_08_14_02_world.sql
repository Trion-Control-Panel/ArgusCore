-- See ARGUSCORE_FIXES.md for details.

-- Emergency revert of 2026_08_14_01_world.sql: populating battlepay_shop_entry.DisplayInfoID caused
-- the real client to fail with "Not enough memory" (Battle.net support article 6926) - a much worse
-- regression than the icon/3D-model issue it was meant to fix. Reverting immediately; see
-- ARGUSCORE_FIXES.md "Follow-up 5" for the investigation.

UPDATE `battlepay_shop_entry` SET `DisplayInfoID` = 0 WHERE `ProductID` IN (1, 2, 3);
