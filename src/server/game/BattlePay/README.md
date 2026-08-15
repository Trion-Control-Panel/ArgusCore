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

-- 3. What it actually delivers - AT LEAST ONE ROW REQUIRED (unless GrantsBoost is set, see below), or
--    the product takes payment and delivers nothing. ProductFilter/DeliverAndDeduct's item path is
--    entirely item-driven - there is no working delivery path for Gold/Currency/etc. WebsiteTypes
--    despite the enum listing them (see the comment on Battlepay::WebsiteType in BattlePayData.h).
--    GrantsBoost (below) is a separate mechanism, independent of WebsiteType/Items.
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

## Bundling a free Character Boost with a product

Any product (regardless of `WebsiteType`) can grant a free Character Boost as a bonus alongside
whatever it normally delivers, via `battlepay_product.GrantsBoost` + the existing `CustomValue` column
(reused here as the target level, e.g. `110`):

```sql
UPDATE `battlepay_product` SET `GrantsBoost` = 1, `CustomValue` = 110 WHERE `ProductID` = 4;
```

The boost is applied to `purchase.TargetCharacter` - the same character the player already picks when
buying anything - via `BattlePayMgr::ApplyCharacterBoost`. **Important limitations of this first pass**
(see `ARGUSCORE_FIXES.md` for the full reasoning):

- **The target character must be online at purchase time.** Boost mechanics (leveling, gear, talents)
  need a live `Player*` - there's no offline/mail-style fallback the way normal items have. A
  `GrantsBoost` purchase targeting an offline character is denied outright, not silently partial.
- **No interactive spec-choice wizard.** The real retail Character Boost popup lets you pick a
  spec/loadout before applying - not built here (see the character-select section below for what *is*
  built). The boost auto-applies to the target's *current* class and spec (falling back to the class's
  real default spec, `Player::GetDefaultSpecId()`, only if the character never picked one).
- What it grants: level (`CustomValue`), a full gear loadout for the character's class (real Blizzard
  data - `CharacterLoadout.db2`/`CharacterLoadoutItem.db2`, `Purpose == 3`, the same DB2 this server
  already uses for character-creation starting items at `Purpose == 9`), a talent reset, the 4 flying-
  mount spells + Pathfinder (if `CustomValue >= 110`), and a flat starting gold amount (currently a
  placeholder 1000g in `ApplyCharacterBoost` - tune freely, it's a product decision).
- Professions are **not** granted - no mechanism exists anywhere (local or reference) that grants actual
  profession recipes automatically, only raw skill values, which wouldn't be very useful on its own.

## Character-select Character Boost (create/redeem a boosted character) - PARKED, button doesn't appear

This is the *other*, separate boost entry point - the real retail "Boost" button at character select,
letting a player boost a brand-new or existing character without ever being logged into it first. It's
independent of the in-game `GrantsBoost` bonus above (different trigger, different mechanics - the
target is guaranteed offline here) and uses its own credit system rather than a purchase.

**Status: the redemption backend is fully built and confirmed correct, but the Boost button never
appears on the real client, and there is currently no known way to fix that.** Confirmed via direct
testing + diagnostics: the server sends a real, non-empty, non-crashing distribution object (credit ID,
a real `DisplayInfo`-backed product, the same `UnkBits` CharacterBoost marker `SendProductList` uses) -
the client just never recognizes it as a redeemable credit. Root cause (see `ARGUSCORE_FIXES.md` for
the full investigation): the button's visibility is gated by two **native, client-side-only** calls,
`C_SharedCharacterServices.GetUpgradeDistributions()` and `C_CharacterServices.
GetCharacterServiceDisplayOrder()`, which key against something Blizzard's client code expects
internally - not anything visible in the wire data we control. None of the three reference cores
checked (`logs/DestinyCore`, `logs/AshamaneCore`, and a real production Legion 7.3.5 fork,
`dufernst/LegionCore-7.3.5`) ever implemented this specific feature end to end, and no public packet
capture of it exists. Unblocking this needs a real capture or documentation of that native keying -
not something guessable from source code alone.

**What *is* real and working**, if the button problem ever gets solved (don't rip this out):
- `battlepay_pending_boost` (auth DB) - credit tracking, granted via `.battlepay grantboost <account>
  <level>` or direct DB access (`INSERT INTO battlepay_pending_boost (BattlenetAccountId, ProductID,
  TargetLevel) VALUES (?, 200, ?)` - `ProductID` must reference a real, `DisplayInfo`-backed
  `battlepay_product` row, `200` is the placeholder one seeded by `2026_08_15_00_world.sql`).
- `BattlePayMgr::SendDistributionList` - sends the (correctly-formed, non-crashing) distribution list.
- `BattlePayMgr::AssignDistributionToCharacter`, wired to `CMSG_BATTLE_PAY_DISTRIBUTION_ASSIGN_TO_TARGET`
  - validates the credit and target character ownership, then calls `ApplyCharacterBoostOffline`
  (raw-SQL mirror of the in-game `ApplyCharacterBoost` - level, gear via mail, flight, talents, gold,
  since the target is always offline here) and marks the credit redeemed. This code path is untestable
  end to end without the button working, but it's a straightforward, reviewable extension of the
  already-proven-working in-game boost mechanics, not a leap of faith.

**Recommendation:** use the in-game `GrantsBoost` purchase flow (above) for actual boosting today - it
works. Only revisit this section if a real 7.3.5.26972 packet capture or documentation of
`GetUpgradeDistributions`/`GetCharacterServiceDisplayOrder`'s native keying ever turns up.
