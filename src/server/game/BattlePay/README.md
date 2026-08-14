# BattlePay (in-game Shop) - adding tabs and items

All shop content lives in the **world** database, loaded once at server startup
(`BattlePayMgr::LoadProductCatalog()`). After editing any of these tables, restart the worldserver (or
add a `.reload` path if one gets built later) - there is currently no hot-reload.

## The tables, and how they relate

```
battlepay_product_group  (a tab, e.g. "Mounts")
        |
        | GroupID
        v
battlepay_shop_entry      (one row = one tile placement of a product inside a tab)
        |
        | ProductID
        v
battlepay_product          (the purchasable thing: price, flags, WebsiteType)
        |
        | ProductID
        v
battlepay_product_item     (one row per item the product delivers, quantity included)
```

`battlepay_product`, `battlepay_shop_entry`, and `battlepay_display_info` all separately reference a
**display info** row (`battlepay_display_info.DisplayInfoId`) for the name/description text. Point
all three at the *same* `DisplayInfoId` for a given product - the loader doesn't require it, but
nothing stops the name from disagreeing across the two, which reads as a bug to a player.

## Known limitation: real icons / 3D model preview

Per-product icons and the "View in 3D" mount preview are **not implemented** - an attempt was made and
reverted after it caused a real client crash ("Not enough memory"). See `ARGUSCORE_FIXES.md`
("BattlePay Phase 2" -> Follow-up 3/4/5) for the full investigation before trying again; it needs a
real packet capture or protocol documentation to move forward safely, neither of which is available
locally. New products will show the client's placeholder "?" icon and no 3D model - this is expected,
not a regression.

Leave `battlepay_display_info.FileDataID` at 0 and don't add `battlepay_display_info_visuals` rows for
new products, matching what's currently shipped.

## Adding a new tab (product group)

```sql
INSERT INTO `battlepay_product_group` (`GroupID`, `Name`, `IconFileDataID`, `DisplayType`, `Ordering`) VALUES
(100, 'Toys and Trinkets', 237429, 0, 2);
```

- `GroupID`: pick an unused id. **Never use 22** - it's client-hardcoded as the Services/character-boost
  tab (`Battlepay::ProductGroups::Services` in `BattlePayData.h`). The other values in
  `Battlepay::ProductGroups` (Mount=1, Pets=2, Golds=4, Professions=5, ..., Featured=17) are just this
  port's own convention for the IDs already seeded (matching what the real Blizzard shop historically
  used) - not client-enforced like 22 is, but worth avoiding for a genuinely new tab to prevent
  confusion. Picking something clearly outside that range (e.g. 100+) sidesteps the question entirely.
- `IconFileDataID`: this one **does** work (unlike per-product icons above) - it's a plain,
  unconditionally-sent field, confirmed rendering correctly on the real client. Look up a real
  FileDataID for the tab icon from `logs/db2csv/Item.csv`'s `IconFileDataID` column (keyed by an
  ItemID whose icon you want to reuse) or an existing UI atlas icon.
- `DisplayType`: `0` = default grid, `1` = splash, `2` = double-wide (`Battlepay::GroupDisplayType`).
- `Ordering`: lower shows first.

## Adding a new item to an existing (or new) tab

Four inserts, using product ID `4` and display info ID `4` as an example (pick unused IDs):

```sql
-- 1. Display info: the name/description text shown on the tile.
INSERT INTO `battlepay_display_info` (`DisplayInfoId`, `Name1`, `Name3`) VALUES
(4, 'Bag of Fireworks', 'A bundle of festival fireworks.');

-- 2. The product itself: price and classification.
INSERT INTO `battlepay_product` (`ProductID`, `Type`, `WebsiteType`, `DisplayInfoID`, `ShopPointsPrice`) VALUES
(4, 0, 3, 4, 250); -- WebsiteType 3 = Item (see Battlepay::WebsiteType in BattlePayData.h)

-- 3. What it actually delivers - AT LEAST ONE ROW REQUIRED, or the product takes payment and
--    delivers nothing (ProductFilter/DeliverAndDeduct are both entirely item-driven; there is no
--    working delivery path for Gold/Currency/CharacterBoost/etc. WebsiteTypes despite the enum
--    listing them - see the comment on Battlepay::WebsiteType in BattlePayData.h).
INSERT INTO `battlepay_product_item` (`ProductID`, `ItemID`, `Quantity`) VALUES
(4, 21100, 5); -- 5x Firework - Blue

-- 4. Where it shows up: which tab, in what order.
INSERT INTO `battlepay_shop_entry` (`GroupID`, `ProductID`, `Ordering`) VALUES
(100, 4, 0);
```

Notes:
- `ShopPointsPrice` is the single account-wide currency price (see the "BattlePay Phase 2" entry in
  `ARGUSCORE_FIXES.md` for why - the real client purchase flow has no per-purchase currency choice).
- Leave `battlepay_shop_entry.DisplayInfoID` at its default (0). Setting it is what caused the client
  crash referenced above.
- A multi-item bundle just means multiple `battlepay_product_item` rows with the same `ProductID` -
  the client only shows a per-item tooltip breakdown when there's more than one (see
  `BattlePayMgr::SendProductList()`'s `packetItem.ItemID` handling).
- `ClassMask` on `battlepay_product` (0 = all classes) restricts a product to specific classes -
  matches `Player::GetClassMask()`'s bitmask, not raw class IDs.
- To credit a test account with Shop Points to buy the new item: `.battlepay credit <account> <amount>`
  (see `src/server/scripts/Commands/cs_battlepay.cpp`).
