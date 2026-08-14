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

// See ARGUSCORE_FIXES.md for details. BattlePay (in-game Shop) port: Phase 1 (read-only catalog) and
// Phase 2 (purchase flow: StartPurchase/ConfirmPurchase/delivery). Serialization ported verbatim
// from DestinyCore/AshamaneCore's raw ByteBuffer WriteBit/WriteBits style rather than translated to
// ArgusCore's newer PacketOperators idiom - both compile against the same underlying ByteBuffer
// primitives, and correctness against the real client (this exact bit layout is proven working in
// two independent reference cores) takes priority over stylistic consistency for wire-format code.

#ifndef BattlePayPackets_h__
#define BattlePayPackets_h__

#include "Packet.h"
#include "ObjectGuid.h"
#include "Optional.h"
#include <string>
#include <vector>

namespace WorldPackets
{
    namespace BattlePay
    {
        class GetProductList final : public ClientPacket
        {
        public:
            GetProductList(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PAY_GET_PRODUCT_LIST, std::move(packet)) { }

            void Read() override { }
        };

        // The Shop UI queries this immediately on open, alongside GetProductList - leaving it
        // unanswered (STATUS_UNHANDLED/Handle_NULL) left the client stuck on its loading screen
        // even after the product list itself started rendering correctly. Purchase.size() is
        // always 0 until the purchase flow (a later phase) exists to populate real entries.
        class GetPurchaseListQuery final : public ClientPacket
        {
        public:
            GetPurchaseListQuery(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PAY_GET_PURCHASE_LIST, std::move(packet)) { }

            void Read() override { }
        };

        // Sent alongside GetProductList/GetPurchaseListQuery when the Shop UI opens - despite the
        // name not matching the CMSG_BATTLE_PAY_* prefix, this is a third query the client waits on
        // before dismissing its loading screen. 2/2 reference cores treat it as a pure ack with no
        // response and never decode the body, so Read() is intentionally empty here too.
        class UpdateVasPurchaseStates final : public ClientPacket
        {
        public:
            UpdateVasPurchaseStates(WorldPacket&& packet) : ClientPacket(CMSG_UPDATE_VAS_PURCHASE_STATES, std::move(packet)) { }

            void Read() override { }
        };

        struct BattlePayPurchase
        {
            uint64 PurchaseID = 0;
            uint64 UnkLong = 0;
            uint64 UnkLong2 = 0;
            uint32 Status = 0;
            uint32 ResultCode = 0;
            uint32 ProductID = 0;
            uint32 UnkInt = 0;
            std::string WalletName;
        };

        class PurchaseListResponse final : public ServerPacket
        {
        public:
            PurchaseListResponse() : ServerPacket(SMSG_BATTLE_PAY_GET_PURCHASE_LIST_RESPONSE, 8) { }

            WorldPacket const* Write() override;

            uint32 Result = 0;
            std::vector<BattlePayPurchase> Purchase;
        };

        // Phase 2 (purchase flow). Field order/types ported verbatim from the reference cores'
        // Read()/Write() implementations, not guessed at - see ARGUSCORE_FIXES.md.

        class StartPurchase final : public ClientPacket
        {
        public:
            StartPurchase(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PAY_START_PURCHASE, std::move(packet)) { }

            void Read() override;

            ObjectGuid TargetCharacter;
            uint32 ClientToken = 0;
            uint32 ProductID = 0;
        };

        // Second CMSG opcode the client can send for the same "player clicked Buy" action - 2/2
        // reference cores wire both this and StartPurchase to identical purchase-start logic. The
        // real client build sends this one (C_StoreSecure.PurchaseProduct in the client Lua), not
        // StartPurchase - confirmed by ArgusCore only having StartPurchase wired (to the wrong
        // opcode value, see Opcodes.h) and purchases never reaching the server at all. See
        // ARGUSCORE_FIXES.md.
        class PurchaseProduct final : public ClientPacket
        {
        public:
            PurchaseProduct(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PAY_PURCHASE_PRODUCT, std::move(packet)) { }

            void Read() override;

            ObjectGuid TargetCharacter;
            uint32 ClientToken = 0;
            uint32 ProductID = 0;
            std::string WowSytem;
            std::string PublicKey;
        };

        class StartPurchaseResponse final : public ServerPacket
        {
        public:
            StartPurchaseResponse() : ServerPacket(SMSG_BATTLE_PAY_START_PURCHASE_RESPONSE, 16) { }

            WorldPacket const* Write() override;

            uint64 PurchaseID = 0;
            uint32 ClientToken = 0;
            uint32 PurchaseResult = 0;
        };

        class PurchaseUpdate final : public ServerPacket
        {
        public:
            PurchaseUpdate() : ServerPacket(SMSG_BATTLE_PAY_PURCHASE_UPDATE, 4) { }

            WorldPacket const* Write() override;

            std::vector<BattlePayPurchase> Purchase;
        };

        // Prompts the client's purchase-confirmation dialog. ServerToken must be echoed back
        // unchanged in ConfirmPurchaseResponse - the anti-replay check.
        class ConfirmPurchase final : public ServerPacket
        {
        public:
            ConfirmPurchase() : ServerPacket(SMSG_BATTLE_PAY_CONFIRM_PURCHASE, 20) { }

            WorldPacket const* Write() override;

            uint64 PurchaseID = 0;
            uint32 ServerToken = 0;
        };

        class ConfirmPurchaseResponse final : public ClientPacket
        {
        public:
            ConfirmPurchaseResponse(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PAY_CONFIRM_PURCHASE_RESPONSE, std::move(packet)) { }

            // NOTE: the ConfirmPurchase bit is read FIRST, before the two integer fields - verbatim
            // from the reference, easy to get backwards.
            void Read() override;

            uint64 ClientCurrentPriceFixedPoint = 0;
            uint32 ServerToken = 0;
            bool ConfirmPurchase = false;
        };

        struct ProductDisplayVisualData
        {
            uint32 DisplayId = 0;
            uint32 VisualId = 0;
            std::string ProductName;
        };

        struct ProductDisplayInfo
        {
            std::vector<ProductDisplayVisualData> Visuals;
            Optional<uint32> CreatureDisplayInfoID;
            Optional<uint32> VisualsId;
            Optional<uint32> Flags;
            // Genuine wire-format unknowns in the reference (DestinyCore/AshamaneCore never
            // identified these either) - ported as-is rather than guessed at or omitted.
            Optional<uint32> UnkInt1;
            Optional<uint32> UnkInt2;
            Optional<uint32> UnkInt3;
            std::string Name1;
            std::string Name2;
            std::string Name3;
            std::string Name4;
        };

        struct ProductItem
        {
            Optional<ProductDisplayInfo> DisplayInfo;
            Optional<uint16> PetResult;
            uint32 ID = 0;
            uint32 ItemID = 0;
            uint32 Quantity = 0;
            uint32 UnkInt1 = 0;
            uint32 UnkInt2 = 0;
            uint8 UnkByte = 0;
            bool HasPet = false;
        };

        struct BattlePayProduct
        {
            std::vector<ProductItem> Items;
            Optional<ProductDisplayInfo> DisplayInfo;
            Optional<uint16> UnkBits;
            uint32 ProductID = 0;
            uint32 Flags = 0;
            uint32 UnkInt1 = 0;
            uint32 DisplayId = 0;
            uint32 ItemId = 0;
            uint32 UnkInt4 = 0;
            uint32 UnkInt5 = 0;
            std::string UnkString;
            uint8 Type = 0;
            bool UnkBit = false;
        };

        // The client's Shop frame stays on its "Loading, please wait" screen until it has all
        // THREE of: a product list, a purchase list, AND a distribution list
        // (C_StoreSecure.HasProductList/HasPurchaseList/HasDistributionList - confirmed from the
        // real client Lua, Blizzard_StoreUISecure.lua). The reference cores send this once at
        // login (WorldSession::SendDisplayPromo, called from InitializeSessionCallback) - ArgusCore's
        // port never carried that call over, so PRODUCT_DISTRIBUTIONS_UPDATED never fired and the
        // Shop stayed stuck even though the product/purchase lists were both already correct. We
        // don't have any real promo/distribution data to offer, so this sends a genuinely empty
        // list (Result=Ok, 0 objects) rather than fabricate the reference's hardcoded product 109.
        // See ARGUSCORE_FIXES.md.
        struct BattlePayDistributionObject
        {
            Optional<BattlePayProduct> Product;
            ObjectGuid TargetPlayer;
            uint64 DistributionID = 0;
            uint64 PurchaseID = 0;
            uint32 Status = 0;
            uint32 ProductID = 0;
            uint32 TargetVirtualRealm = 0;
            uint32 TargetNativeRealm = 0;
            bool Revoked = false;
        };

        class DistributionListResponse final : public ServerPacket
        {
        public:
            DistributionListResponse() : ServerPacket(SMSG_BATTLE_PAY_GET_DISTRIBUTION_LIST_RESPONSE, 8) { }

            WorldPacket const* Write() override;

            uint32 Result = 0;
            std::vector<BattlePayDistributionObject> DistributionObject;
        };

        class DisplayPromotion final : public ServerPacket
        {
        public:
            DisplayPromotion(uint32 ID) : ServerPacket(SMSG_DISPLAY_PROMOTION, 4), PromotionID(ID) { }

            WorldPacket const* Write() override;

            uint32 PromotionID = 0;
        };

        struct BattlePayProductGroup
        {
            uint32 GroupID = 0;
            uint32 IconFileDataID = 0;
            uint32 Ordering = 0;
            uint32 UnkInt = 0;
            std::string Name;
            std::string IsAvailableDescription;
            uint8 DisplayType = 0;
        };

        struct BattlePayShopEntry
        {
            Optional<ProductDisplayInfo> DisplayInfo;
            uint32 EntryID = 0;
            uint32 GroupID = 0;
            uint32 ProductID = 0;
            int32 Ordering = 0;
            uint32 VasServiceType = 0;
            uint8 StoreDeliveryType = 0;
        };

        struct ProductInfoStruct
        {
            std::vector<uint32> ProductIDs;
            std::vector<uint32> UnkInts; // genuine wire-format unknown in the reference, kept as-is
            Optional<ProductDisplayInfo> DisplayInfo;
            uint64 NormalPriceFixedPoint = 0;
            uint64 CurrentPriceFixedPoint = 0;
            uint32 ProductID = 0;
            uint32 UnkInt2 = 0;
            uint32 ChoiceType = 0;
        };

        struct ProductListData
        {
            std::vector<ProductInfoStruct> ProductInfo;
            std::vector<BattlePayProduct> Product;
            std::vector<BattlePayProductGroup> ProductGroup;
            std::vector<BattlePayShopEntry> Shop;
            uint32 CurrencyID = 0;
        };

        class ProductListResponse final : public ServerPacket
        {
        public:
            ProductListResponse() : ServerPacket(SMSG_BATTLE_PAY_GET_PRODUCT_LIST_RESPONSE, 20) { }

            WorldPacket const* Write() override;

            ProductListData ProductList;
            uint32 Result = 0;
        };
    }
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::ProductDisplayInfo const& displayInfo);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::BattlePayProduct const& product);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::ProductInfoStruct const& productInfo);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::BattlePayPurchase const& purchase);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::BattlePayDistributionObject const& object);

#endif // BattlePayPackets_h__
