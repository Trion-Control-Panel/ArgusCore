-- See ARGUSCORE_FIXES.md for details.

-- BattlePay: real icons + 3D "View in 3D" preview for the 3 seeded shop products. The original
-- Phase 1 seed left `battlepay_display_info.FileDataID` at its default (0), so the client fell back
-- to the placeholder "?" icon for every tile, and left `battlepay_display_info_visuals` empty, so no
-- product ever had a 3D model card to show.
--
-- FileDataID values are each item's own icon, read directly from Item.db2 (`IconFileDataID` column,
-- keyed by ItemID - logs/db2csv/Item.csv), not guessed. DisplayId/VisualId values for the two mounts
-- are each mount's CreatureDisplayInfoID (from MountXDisplay.csv, keyed by MountID) and UiModelSceneID
-- (from Mount.csv, keyed by the item's on-use SpellID from ItemEffect.csv) - the same chain the real
-- client walks (Item -> on-use Spell -> Mount -> MountXDisplay) to know what to render when the mount
-- is actually summoned. Snowball (a thrown consumable, not a mount) intentionally gets an icon only,
-- no Visuals row - matches how the real Shop displays non-mount items.

UPDATE `battlepay_display_info` SET `FileDataID` = 603364  WHERE `DisplayInfoId` = 1; -- Jeweled Onyx Panther (ItemID 82453)
UPDATE `battlepay_display_info` SET `FileDataID` = 1129627 WHERE `DisplayInfoId` = 2; -- Reins of the Grove Warden (ItemID 128422)
UPDATE `battlepay_display_info` SET `FileDataID` = 132387  WHERE `DisplayInfoId` = 3; -- Snowball (ItemID 17202)

DELETE FROM `battlepay_display_info_visuals` WHERE `DisplayInfoId` IN (1, 2);
INSERT INTO `battlepay_display_info_visuals` (`DisplayInfoId`, `DisplayId`, `VisualId`, `ProductName`) VALUES
(1, 42185, 4, 'Jeweled Onyx Panther'), -- Mount ID 451, CreatureDisplayInfoID 42185, UiModelSceneID 4
(2, 64583, 4, 'Reins of the Grove Warden'); -- Mount ID 764, CreatureDisplayInfoID 64583, UiModelSceneID 4
