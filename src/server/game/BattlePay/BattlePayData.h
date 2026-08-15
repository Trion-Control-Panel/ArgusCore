/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
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

#ifndef BattlePayData_h__
#define BattlePayData_h__

#include "Define.h"
#include "ObjectGuid.h"
#include <string>
#include <vector>

// See ARGUSCORE_FIXES.md for details.
namespace Battlepay
{
    constexpr float g_CurrencyPrecision = 10000.0f;

    // Ported from DestinyCore/AshamaneCore's Battlepay::WebsiteType. Only a subset is meaningfully
    // implemented in ArgusCore's delivery logic (see BattlePayMgr::ProcessDelivery) - the rest are
    // kept so battlepay_product.WebsiteType values match the real client-visible enum 1:1, in case a
    // future phase implements more of them.
    enum WebsiteType : uint8
    {
        Category = 0,
        Spell = 1,
        Title = 2,
        Item = 3,
        Currency = 4,
        CharacterRename = 5,
        GuildRename = 6,
        Gold = 7,
        Level = 8,
        CharacterFactionChange = 9,
        CharacterRaceChange = 10,
        PremadeCharacter = 11,
        RealmTransfer = 12,
        ExpansionTransfer = 13,
        Premium = 14,
        DeletedCharacter = 15,
        ItemProfession = 16,
        Transmogrification = 17,
        PackItems = 18,
        CategoryProfession = 19,
        CategoryPremade = 20,
        Mount = 21,
        CharacterCustomization = 22,
        CategoryCharacterManagement = 23,
        CategoryRealmTransfer = 24,
        CategoryExpansionTransfer = 25,
        CategoryGold = 26,
        CharacterBoost = 29,
        BattlePet = 30,
        AppareanceArtifac = 31,
        Currency1 = 38,
        Currency2 = 39,
        Promo = 55,
        RepClassic = 56,
        RepBurnig = 57,
        RepTLK = 58,
        RepCata = 59,
        RepPanda = 60,
        RepDraenor = 61,
        RepLegion = 62,
        Unbinall = 63,
        PremadePve = 64,
        PremadePvp = 65,
        VueloD = 66,
        ArtifactPower = 67,
        RacesAlliedVoidElf = 68,
        RacesAlliedLighForgedDraenei = 69,
        RacesAlliedNightborne = 70,
        RacesAlliedHighmountainTauren = 71,
        ArtifactPower101 = 72,
        HonorLvl = 73,
        PremadePve100 = 74,
        MaxWebsiteType
    };

    // Ported from the reference cores' Battlepay::Error - real wire-protocol result codes the client
    // recognizes, not invented. Only the subset ArgusCore's purchase flow actually sends is used.
    enum Error : uint32
    {
        Ok = 0,
        PurchaseDenied = 1,
        PaymentFailed = 2,
        Other = 3,
        InsufficientBalance = 28,
    };

    // Ported from the reference cores' Battlepay::UpdateStatus - real values sent in
    // BattlePayPurchase::Status, recognized by the client's purchase-progress UI.
    namespace UpdateStatus
    {
        enum : uint32
        {
            Loading = 9,
            Ready = 6,
            Finish = 3
        };
    }

    // Ported from the reference cores' Battlepay::DistributionStatus - real values sent in
    // BattlePayDistributionObject::Status. Corroborated identically in DestinyCore and AshamaneCore.
    namespace DistributionStatus
    {
        enum : uint32
        {
            None = 0,
            Available = 1,
            AddToProcess = 2,
            ProcessComplete = 3,
            Finished = 4
        };
    }

    namespace ProductGroups
    {
        enum : uint32
        {
            Mount = 1,
            Pets = 2,
            Services = 22, // WOW_SERVICES_CATEGORY_ID - client-hardcoded, never reuse for other content
            Golds = 4,
            Professions = 5,
            ProfessionItems = 6,
            ProfessionRecipes = 7,
            Armors = 8,
            Weapons = 9,
            Toys = 10,
            Boosts = 11,
            Bags = 12,
            Heirlooms = 13,
            ItemBundles = 14,
            Transmogs = 15,
            SpecialOffers = 16,
            Featured = 17
        };
    }

    namespace GroupDisplayType
    {
        enum : uint8
        {
            Default = 0,
            Splash = 1,
            DoubleWide = 2
        };
    }

    struct ProductGroup
    {
        uint32 GroupID = 0;
        int32 IconFileDataID = 0;
        int32 Ordering = 0;
        std::string Name;
        uint8 DisplayType = 0;
    };

    struct ProductGroupLocale
    {
        std::vector<std::string> Name;
    };

    struct DisplayVisualData
    {
        uint32 DisplayID = 0;
        uint32 VisualID = 0;
        std::string ProductName;
    };

    struct DisplayInfo
    {
        uint32 CreatureDisplayInfoID = 0;
        uint32 VisualsId = 0;
        uint32 Flags = 0;
        std::string Name1;
        std::string Name2;
        std::string Name3;
        std::string Name4;
    };

    struct DisplayInfoLocale
    {
        std::vector<std::string> Name1;
        std::vector<std::string> Name2;
        std::vector<std::string> Name3;
        std::vector<std::string> Name4;
    };

    struct ProductItem
    {
        uint32 ID = 0;
        uint32 ItemID = 0;
        uint32 Quantity = 0;
        uint32 DisplayInfoID = 0;
        uint8 PetResult = 0;
    };

    struct Product
    {
        std::vector<ProductItem> Items;
        uint32 ProductID = 0;
        uint32 Flags = 0;
        uint32 DisplayInfoID = 0;
        uint32 ClassMask = 0;
        std::string ScriptName;
        uint8 WebsiteType = 0;
        uint8 Type = 0;
        uint8 ChoiceType = 0;

        // Fix vs. reference: reference's CustomValue is declared but never populated from any DB
        // column, silently breaking Gold/Level delivery. Here it's a real, loaded column - meaning
        // depends on WebsiteType (Gold -> copper amount, CharacterBoost -> target level).
        uint32 CustomValue = 0;

        // Independent of WebsiteType - lets any product bundle a free Character Boost as a bonus
        // alongside its normal delivery. CustomValue doubles as the boost's target level in this case
        // (matches the comment above). See BattlePayMgr::ApplyCharacterBoost.
        bool GrantsBoost = false;

        // Deviation from reference: the reference prices in real-money fixed-point; this port has
        // no real-money concept, so every product has a single Shop Points price instead. (An
        // earlier design had three independent currencies with player choice at purchase time, but
        // the real 7.3.5 client purchase flow - C_StoreSecure.PurchaseProduct/PurchaseProductConfirm
        // - has no currency-selector field at all, so that was never actually implementable through
        // the native Store UI. See ARGUSCORE_FIXES.md.)
        uint64 ShopPointsPrice = 0;
    };

    // In-flight purchase state for one Battle.net account, keyed by BattlenetAccountId in
    // BattlePayMgr::_purchases (global map, not per-WorldSession - matches LFGListMgr's own
    // precedent for per-player transient state, and Phase 1's explicit choice of one global manager
    // over the reference's per-session design). ServerToken is a replay-prevention nonce, not a
    // secret. Lock is set once ConfirmPurchase begins processing and only cleared after delivery
    // fully completes (success or failure), so a second StartPurchase can't reset an in-flight
    // delivery out from under itself - checked at both StartPurchase and ConfirmPurchase, unlike the
    // reference (which only checks at confirm time).
    struct Purchase
    {
        ObjectGuid TargetCharacter;
        uint64 DistributionId = 0;
        uint64 PurchaseID = 0;
        uint64 CurrentPrice = 0;
        uint32 ClientToken = 0;
        uint32 ServerToken = 0;
        uint32 ProductID = 0;
        uint32 Status = 0; // Battlepay::UpdateStatus
        bool Lock = false;
    };

    struct ShopEntry
    {
        uint32 EntryID = 0;
        uint32 GroupID = 0;
        uint32 ProductID = 0;
        uint32 Flags = 0;
        uint32 DisplayInfoID = 0;
        int32 Ordering = 0;
        uint8 BannerType = 0;
    };
}

#endif // BattlePayData_h__
