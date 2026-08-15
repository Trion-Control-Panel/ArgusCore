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

// See ARGUSCORE_FIXES.md for details.

#include "BattlePayPackets.h"

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::ProductDisplayInfo const& displayInfo)
{
    data.WriteBit(displayInfo.CreatureDisplayInfoID.has_value());
    data.WriteBit(displayInfo.VisualsId.has_value());

    data.WriteBits(displayInfo.Name1.length(), 10);
    data.WriteBits(displayInfo.Name2.length(), 10);
    data.WriteBits(displayInfo.Name3.length(), 13);
    data.WriteBits(displayInfo.Name4.length(), 13);

    data.WriteBit(displayInfo.Flags.has_value());
    data.WriteBit(displayInfo.UnkInt1.has_value());
    data.WriteBit(displayInfo.UnkInt2.has_value());
    data.WriteBit(displayInfo.UnkInt3.has_value());
    data.FlushBits();

    data << static_cast<uint32>(displayInfo.Visuals.size());

    if (displayInfo.CreatureDisplayInfoID)
        data << *displayInfo.CreatureDisplayInfoID;

    if (displayInfo.VisualsId)
        data << *displayInfo.VisualsId;

    data.WriteString(displayInfo.Name1);
    data.WriteString(displayInfo.Name2);
    data.WriteString(displayInfo.Name3);
    data.WriteString(displayInfo.Name4);

    if (displayInfo.Flags)
        data << *displayInfo.Flags;

    if (displayInfo.UnkInt1)
        data << *displayInfo.UnkInt1;

    if (displayInfo.UnkInt2)
        data << *displayInfo.UnkInt2;

    if (displayInfo.UnkInt3)
        data << *displayInfo.UnkInt3;

    for (WorldPackets::BattlePay::ProductDisplayVisualData const& itr : displayInfo.Visuals)
    {
        data.WriteBits(itr.ProductName.length(), 10);
        data.FlushBits();
        data << itr.DisplayId;
        data << itr.VisualId;
        data.WriteString(itr.ProductName);
    }

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::BattlePayProduct const& product)
{
    data << product.ProductID;

    data << product.Type;
    data << product.Flags;
    data << product.UnkInt1;
    data << product.DisplayId;
    data << product.ItemId;
    data << product.UnkInt4;
    data << product.UnkInt5;

    data.WriteBits(product.UnkString.size(), 8);
    data.WriteBit(product.UnkBit);
    data.WriteBit(product.UnkBits.has_value());
    data.WriteBits(product.Items.size(), 7);
    data.WriteBit(product.DisplayInfo.has_value());

    if (product.UnkBits)
        data.WriteBits(*product.UnkBits, 4);

    data.FlushBits();

    for (WorldPackets::BattlePay::ProductItem const& productItem : product.Items)
    {
        data << productItem.ID;
        data << productItem.UnkByte;
        data << productItem.ItemID;
        data << productItem.Quantity;
        data << productItem.UnkInt1;
        data << productItem.UnkInt2;

        data.WriteBit(productItem.HasPet);
        data.WriteBit(productItem.PetResult.has_value());
        data.WriteBit(productItem.DisplayInfo.has_value());

        if (productItem.PetResult)
            data.WriteBits(*productItem.PetResult, 4);

        data.FlushBits();

        if (productItem.DisplayInfo)
            data << *productItem.DisplayInfo;
    }

    if (product.DisplayInfo)
        data << *product.DisplayInfo;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::ProductInfoStruct const& info)
{
    data << info.ProductID;
    data << info.NormalPriceFixedPoint;
    data << info.CurrentPriceFixedPoint;
    data << static_cast<uint32>(info.ProductIDs.size());
    data << info.UnkInt2;
    data << static_cast<uint32>(info.UnkInts.size());

    for (uint32 z : info.ProductIDs)
        data << z;

    for (uint32 z : info.UnkInts)
        data << z;

    data.WriteBits(info.ChoiceType, 7);
    data.WriteBit(info.DisplayInfo.has_value());
    data.FlushBits();

    if (info.DisplayInfo)
        data << *info.DisplayInfo;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::BattlePayPurchase const& purchase)
{
    data << purchase.PurchaseID;
    data << purchase.Status;
    data << purchase.ResultCode;
    data << purchase.ProductID;
    data << purchase.UnkLong;
    data << purchase.UnkLong2;
    data << purchase.UnkInt;

    data.WriteBits(purchase.WalletName.length(), 8);
    data.FlushBits();
    data.WriteString(purchase.WalletName);

    return data;
}

WorldPacket const* WorldPackets::BattlePay::PurchaseListResponse::Write()
{
    _worldPacket << Result;
    _worldPacket << static_cast<uint32>(Purchase.size());

    for (BattlePayPurchase const& purchaseData : Purchase)
        _worldPacket << purchaseData;

    return &_worldPacket;
}

void WorldPackets::BattlePay::StartPurchase::Read()
{
    _worldPacket >> ClientToken;
    _worldPacket >> ProductID;
    _worldPacket >> TargetCharacter;
}

void WorldPackets::BattlePay::PurchaseProduct::Read()
{
    _worldPacket >> ClientToken;
    _worldPacket >> ProductID;
    _worldPacket >> TargetCharacter;

    uint32 strlen1 = _worldPacket.ReadBits(6);
    uint32 strlen2 = _worldPacket.ReadBits(12);
    WowSytem = _worldPacket.ReadString(strlen1);
    PublicKey = _worldPacket.ReadString(strlen2);
}

WorldPacket const* WorldPackets::BattlePay::StartPurchaseResponse::Write()
{
    _worldPacket << PurchaseID;
    _worldPacket << PurchaseResult;
    _worldPacket << ClientToken;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::PurchaseUpdate::Write()
{
    _worldPacket << static_cast<uint32>(Purchase.size());
    for (BattlePayPurchase const& purchaseData : Purchase)
        _worldPacket << purchaseData;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::ConfirmPurchase::Write()
{
    _worldPacket << PurchaseID;
    _worldPacket << ServerToken;

    return &_worldPacket;
}

void WorldPackets::BattlePay::ConfirmPurchaseResponse::Read()
{
    ConfirmPurchase = _worldPacket.ReadBit();
    _worldPacket >> ServerToken;
    _worldPacket >> ClientCurrentPriceFixedPoint;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::BattlePayDistributionObject const& object)
{
    data << object.DistributionID;
    data << object.Status;
    data << object.ProductID;

    data << object.TargetPlayer;
    data << object.TargetVirtualRealm;
    data << object.TargetNativeRealm;

    data << object.PurchaseID;
    data.WriteBit(object.Product.has_value());
    data.WriteBit(object.Revoked);
    data.FlushBits();

    if (object.Product)
        data << *object.Product;

    return data;
}

WorldPacket const* WorldPackets::BattlePay::DistributionListResponse::Write()
{
    _worldPacket << Result;
    _worldPacket.WriteBits(DistributionObject.size(), 11);
    // NOTE: the reference cores don't FlushBits() here - given this exact codebase's earlier
    // FlushBits-ordering bug (see ARGUSCORE_FIXES.md), an explicit flush is added rather than
    // risk a partial trailing byte never reaching _storage. Harmless no-op if already aligned.
    _worldPacket.FlushBits();

    for (BattlePayDistributionObject const& objectData : DistributionObject)
        _worldPacket << objectData;

    return &_worldPacket;
}

void WorldPackets::BattlePay::DistributionAssignToTarget::Read()
{
    _worldPacket >> ProductID;
    _worldPacket >> DistributionID;
    _worldPacket >> TargetCharacter;
    _worldPacket >> SpecializationID;
    _worldPacket >> ChoiceID;
}

WorldPacket const* WorldPackets::BattlePay::BattlePayStartDistributionAssignToTargetResponse::Write()
{
    _worldPacket << DistributionID;
    _worldPacket << UnkInt1;
    _worldPacket << UnkInt2;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::UpgradeStarted::Write()
{
    _worldPacket << CharacterGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::UpgradeComplete::Write()
{
    _worldPacket << CharacterGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::BattlePayCharacterUpgradeQueued::Write()
{
    _worldPacket << Character;
    _worldPacket << static_cast<uint32>(EquipmentItems.size());
    for (uint32 item : EquipmentItems)
        _worldPacket << item;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::DisplayPromotion::Write()
{
    _worldPacket << PromotionID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::ProductListResponse::Write()
{
    _worldPacket << Result;
    _worldPacket << ProductList.CurrencyID;

    _worldPacket << static_cast<uint32>(ProductList.ProductInfo.size());
    _worldPacket << static_cast<uint32>(ProductList.Product.size());
    _worldPacket << static_cast<uint32>(ProductList.ProductGroup.size());
    _worldPacket << static_cast<uint32>(ProductList.Shop.size());

    for (ProductInfoStruct const& v : ProductList.ProductInfo)
        _worldPacket << v;

    for (BattlePayProduct const& productData : ProductList.Product)
        _worldPacket << productData;

    // Restored to match the reference cores exactly (GroupID/IconFileDataID/DisplayType/Ordering/
    // UnkInt/Name+IsAvailableDescription) after finding real evidence in the actual 7.3.5 client
    // Lua (Blizzard_StoreUISecure.lua) that a group's `flags` field is genuinely read and used
    // (DisableOwnedProducts/EnabledForTrial/EnabledForVeteran checks) - UnkInt is very likely that
    // same field, just never identified/named correctly by the reference cores. An earlier attempt
    // to remove UnkInt + IsAvailableDescription (based on an older WoD-era packet capture that
    // predates this field's addition) made no observable difference either way - see
    // ARGUSCORE_FIXES.md for the full investigation.
    for (BattlePayProductGroup const& productGroupData : ProductList.ProductGroup)
    {
        _worldPacket << productGroupData.GroupID;
        _worldPacket << productGroupData.IconFileDataID;
        _worldPacket << productGroupData.DisplayType;
        _worldPacket << productGroupData.Ordering;
        _worldPacket << productGroupData.UnkInt;

        _worldPacket.WriteBits(productGroupData.Name.length(), 8);
        _worldPacket.WriteBits(productGroupData.IsAvailableDescription.length() + 1, 24);
        _worldPacket.FlushBits();
        _worldPacket.WriteString(productGroupData.Name);
        if (!productGroupData.IsAvailableDescription.empty())
            _worldPacket.WriteString(productGroupData.IsAvailableDescription);
    }

    for (BattlePayShopEntry const& shopData : ProductList.Shop)
    {
        _worldPacket << shopData.EntryID;
        _worldPacket << shopData.GroupID;
        _worldPacket << shopData.ProductID;
        _worldPacket << shopData.Ordering;
        _worldPacket << shopData.VasServiceType;
        _worldPacket << shopData.StoreDeliveryType;

        // NOTE: FlushBits() intentionally comes AFTER the conditional DisplayInfo write, not
        // before - DisplayInfo's own operator<< starts with more WriteBit/WriteBits calls that
        // must pack into the SAME bit-group as the "has value" bit above (matching the
        // reference's real client-observed wire layout). Flushing early here inserts 7 bits of
        // wrong padding and shifts every byte after it - this exact bug caused the client to
        // reject the whole product list ("temporarily unavailable") on the first real test.
        if (_worldPacket.WriteBit(shopData.DisplayInfo.has_value()))
            _worldPacket << *shopData.DisplayInfo;
        _worldPacket.FlushBits();
    }

    return &_worldPacket;
}
