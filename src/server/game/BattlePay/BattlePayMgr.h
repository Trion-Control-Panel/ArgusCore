/*
 * This file is part of the ArgusCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// BattlePay (in-game Shop) manager. Business logic ported from DestinyCore/AshamaneCore's
// BattlePayDataStoreMgr + BattlepayManager (both real, working reference implementations) and
// consolidated into a single global singleton, matching ArgusCore's own manager idiom
// (LFGListMgr::instance() pattern) rather than the reference's two-manager,
// per-WorldSession design. See ARGUSCORE_FIXES.md for the full port writeup and deviations.
// Phase 1: read-only product catalog + product list display. Phase 2: purchase flow (single
// account-wide "Shop Points" currency, deliver-then-deduct with a crash-safe outbox). Balances are
// credited by writing directly to battlepay_account_balance (an external site with DB access can do
// this itself) - there is deliberately no in-server queue/poll mechanism for this.

#pragma once

#include "BattlePayData.h"
#include "BattlePayPackets.h"
#include "Common.h"
#include <atomic>
#include <map>
#include <mutex>
#include <tuple>
#include <vector>

class Player;
class WorldSession;

class BattlePayMgr
{
public:
    static BattlePayMgr* instance();

    BattlePayMgr();

    void LoadProductCatalog();
    void Update(uint32 diff);

    bool IsAvailable() const;

    Battlepay::Product const* GetProduct(uint32 productId) const;
    std::map<uint32, Battlepay::Product> const& GetProducts() const { return _products; }
    std::vector<Battlepay::ProductGroup> const& GetGroups() const { return _groups; }
    std::vector<Battlepay::ShopEntry> const& GetShopEntries() const { return _shopEntries; }
    uint32 GetProductGroupId(uint32 productId) const;

    Battlepay::DisplayInfo const* GetDisplayInfo(uint32 id) const;
    std::vector<WorldPackets::BattlePay::ProductDisplayVisualData> const* GetDisplayInfoVisuals(uint32 id) const;
    Battlepay::ProductGroupLocale const* GetProductGroupLocale(uint32 id) const;
    Battlepay::DisplayInfoLocale const* GetDisplayInfoLocale(uint32 id) const;

    bool AlreadyOwnProduct(Player const* player, uint32 itemId) const;
    bool ProductFilter(Player const* player, Battlepay::Product const& product) const;

    void SendProductList(WorldSession* session) const;

    // Character-select "you have a Character Boost credit" signal - one real
    // BattlePayDistributionObject per unredeemed battlepay_pending_boost row for this account,
    // including a full nested product (real DisplayInfo) - confirmed by testing that the client
    // doesn't recognize a credit without one. See ARGUSCORE_FIXES.md.
    void SendDistributionList(WorldSession* session) const;

    // Phase 2: purchase flow. See ARGUSCORE_FIXES.md for the full design writeup (deliver-then-
    // deduct with a crash-safe outbox, anti-exploit checks, why purchase state lives here globally
    // rather than on WorldSession).
    //
    // Takes the common fields directly (not a specific packet type) because the client can start a
    // purchase via either of two CMSG opcodes (StartPurchase or PurchaseProduct - 2/2 reference cores
    // wire both to identical purchase-start logic) - matches the reference's own SendMakePurchase
    // helper shape. See ARGUSCORE_FIXES.md.
    void StartPurchase(WorldSession* session, ObjectGuid targetCharacter, uint32 clientToken, uint32 productID);
    void ConfirmPurchase(WorldSession* session, WorldPackets::BattlePay::ConfirmPurchaseResponse const& confirmPurchase);

    // Character-select Character Boost redemption (CMSG_BATTLE_PAY_DISTRIBUTION_ASSIGN_TO_TARGET) -
    // see ARGUSCORE_FIXES.md and src/server/game/BattlePay/README.md for the full design writeup.
    void AssignDistributionToCharacter(WorldSession* session, WorldPackets::BattlePay::DistributionAssignToTarget const& packet);

private:
    void LoadDisplayInfos();
    void LoadDisplayInfoVisuals();
    void LoadProducts();
    void LoadProductGroups();
    void LoadShopEntries();
    void LoadProductGroupLocales();
    void LoadDisplayInfoLocales();

    std::tuple<bool, WorldPackets::BattlePay::ProductDisplayInfo> WriteDisplayInfo(uint32 displayInfoID, LocaleConstant localeIndex, uint32 productId = 0) const;

    // Shared by ApplyCharacterBoost/ApplyCharacterBoostOffline - real client data
    // (CharacterLoadout.db2/CharacterLoadoutItem.db2, Purpose == 3), see ApplyCharacterBoost's comment
    // for how that value was confirmed rather than guessed.
    std::vector<uint32> GetBoostGearItems(uint8 classId) const;

    // Phase 2 purchase-flow helpers
    void SendPurchaseUpdate(WorldSession* session, Battlepay::Purchase const& purchase, uint32 resultCode) const;
    void DeliverAndDeduct(WorldSession* session, Battlepay::Purchase purchase, Battlepay::Product product);

    // Character Boost bonus (Product::GrantsBoost) - see ARGUSCORE_FIXES.md and
    // src/server/game/BattlePay/README.md for scope/limitations (online target only, no interactive
    // spec/gear wizard - auto-applies to the target's current class/spec).
    void ApplyCharacterBoost(Player* target, uint32 toLevel);

    // Character-select Character Boost redemption (battlepay_pending_boost credits) - the target is
    // guaranteed offline here (character select, nothing logged in yet), so this is a raw-SQL
    // equivalent of ApplyCharacterBoost rather than a reuse of it. See ARGUSCORE_FIXES.md.
    void ApplyCharacterBoostOffline(ObjectGuid targetGuid, uint32 toLevel);

    std::map<uint32, Battlepay::Product> _products;
    std::vector<Battlepay::ProductGroup> _groups;
    std::vector<Battlepay::ShopEntry> _shopEntries;
    std::map<uint32, Battlepay::DisplayInfo> _displayInfos;
    std::map<uint32, Battlepay::ProductGroupLocale> _productGroupLocales;
    std::map<uint32, Battlepay::DisplayInfoLocale> _displayInfoLocales;
    std::map<uint32, std::vector<WorldPackets::BattlePay::ProductDisplayVisualData>> _visuals;

    // Phase 2: in-flight purchase state, keyed by BattlenetAccountId - global here (not a
    // WorldSession member) to match LFGListMgr's own precedent for per-player transient state, and
    // Phase 1's explicit choice of one global manager over the reference's per-session design.
    std::map<uint32, Battlepay::Purchase> _purchases;
    std::atomic<uint64> _purchaseIdCounter { 1 };
    std::atomic<uint64> _distributionIdCounter { 1 };

    mutable std::mutex _lock;
};

#define sBattlePayMgr BattlePayMgr::instance()
