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

#include "BattlePayMgr.h"
#include "Containers.h"
#include "DatabaseEnv.h"
#include "Item.h"
#include "ItemEnchantmentMgr.h"
#include "ItemTemplate.h"
#include "Log.h"
#include "Mail.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "World.h"
#include "WorldSession.h"

namespace
{
    // ArgusCore's Trinity::Containers has no MapGetValuePtr helper (unlike some reference cores) -
    // this is the same plain std::map::find idiom already used throughout LFGListMgr.cpp.
    template <typename MapType>
    typename MapType::mapped_type const* FindOrNull(MapType const& map, typename MapType::key_type const& key)
    {
        auto itr = map.find(key);
        return itr != map.end() ? &itr->second : nullptr;
    }
}

BattlePayMgr* BattlePayMgr::instance()
{
    static BattlePayMgr instance;
    return &instance;
}

BattlePayMgr::BattlePayMgr() = default;

bool BattlePayMgr::IsAvailable() const
{
    return sWorld->getBoolConfig(CONFIG_BATTLEPAY_ENABLED);
}

void BattlePayMgr::LoadProductCatalog()
{
    std::lock_guard<std::mutex> lock(_lock);

    LoadDisplayInfos();
    LoadDisplayInfoVisuals();
    LoadProducts();
    LoadProductGroups();
    LoadShopEntries();
    LoadProductGroupLocales();
    LoadDisplayInfoLocales();
}

void BattlePayMgr::Update(uint32 /*diff*/)
{
    // Nothing to tick - Shop Points balances are credited by writing directly to
    // battlepay_account_balance (an external site with DB access does this itself), and
    // StartPurchase/ConfirmPurchase's async DB callbacks are pumped by the calling session's own
    // WorldSession::Update(), not by this manager. See ARGUSCORE_FIXES.md.
}

void BattlePayMgr::LoadDisplayInfos()
{
    uint32 oldMSTime = getMSTime();
    _displayInfos.clear();

    QueryResult result = WorldDatabase.Query("SELECT DisplayInfoId, CreatureDisplayInfoID, FileDataID, Flags, Name1, Name2, Name3, Name4 FROM battlepay_display_info");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 battlepay display infos. DB table `battlepay_display_info` is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        Battlepay::DisplayInfo displayInfo;
        displayInfo.CreatureDisplayInfoID = fields[1].GetUInt32();
        displayInfo.VisualsId = fields[2].GetUInt32();
        displayInfo.Flags = fields[3].GetUInt32();
        displayInfo.Name1 = fields[4].GetString();
        displayInfo.Name2 = fields[5].GetString();
        displayInfo.Name3 = fields[6].GetString();
        displayInfo.Name4 = fields[7].GetString();
        _displayInfos[fields[0].GetUInt32()] = displayInfo;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded {} battlepay display infos in {} ms", _displayInfos.size(), GetMSTimeDiffToNow(oldMSTime));
}

void BattlePayMgr::LoadDisplayInfoVisuals()
{
    uint32 oldMSTime = getMSTime();
    _visuals.clear();

    QueryResult result = WorldDatabase.Query("SELECT DisplayInfoId, DisplayId, VisualId, ProductName FROM battlepay_display_info_visuals");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 battlepay display info visuals. DB table `battlepay_display_info_visuals` is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        WorldPackets::BattlePay::ProductDisplayVisualData visual;
        visual.DisplayId = fields[1].GetUInt32();
        visual.VisualId = fields[2].GetUInt32();
        visual.ProductName = fields[3].GetString();
        _visuals[fields[0].GetUInt32()].push_back(visual);
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded {} battlepay display info visual sets in {} ms", _visuals.size(), GetMSTimeDiffToNow(oldMSTime));
}

void BattlePayMgr::LoadProducts()
{
    uint32 oldMSTime = getMSTime();
    _products.clear();

    QueryResult result = WorldDatabase.Query("SELECT ProductID, Type, ChoiceType, Flags, DisplayInfoID, ScriptName, ClassMask, WebsiteType, CustomValue, ShopPointsPrice FROM battlepay_product");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            Battlepay::Product product;
            product.ProductID = fields[0].GetUInt32();
            product.Type = fields[1].GetUInt8();
            product.ChoiceType = fields[2].GetUInt8();
            product.Flags = fields[3].GetUInt32();
            product.DisplayInfoID = fields[4].GetUInt32();
            product.ScriptName = fields[5].GetString();
            product.ClassMask = fields[6].GetUInt32();
            product.WebsiteType = fields[7].GetUInt8();

            if (product.WebsiteType >= Battlepay::MaxWebsiteType)
            {
                TC_LOG_ERROR("sql.sql", "battlepay_product: ProductID {} has WebsiteType {} >= MaxWebsiteType, skipped.", product.ProductID, product.WebsiteType);
                continue;
            }

            product.CustomValue = fields[8].GetUInt32();
            product.ShopPointsPrice = fields[9].GetUInt64();

            _products[product.ProductID] = product;
        } while (result->NextRow());
    }

    QueryResult itemResult = WorldDatabase.Query("SELECT ID, ProductID, ItemID, Quantity, DisplayID, PetResult FROM battlepay_product_item");
    if (itemResult)
    {
        do
        {
            Field* fields = itemResult->Fetch();

            uint32 productId = fields[1].GetUInt32();
            auto productItr = _products.find(productId);
            if (productItr == _products.end())
                continue;

            Battlepay::ProductItem productItem;
            productItem.DisplayInfoID = fields[4].GetUInt32();
            if (productItem.DisplayInfoID != 0 && !_displayInfos.contains(productItem.DisplayInfoID))
            {
                TC_LOG_ERROR("sql.sql", "battlepay_product_item: Id {} (ProductID {}) references non-existing DisplayID {}, skipped.", fields[0].GetUInt32(), productId, productItem.DisplayInfoID);
                continue;
            }

            productItem.ItemID = fields[2].GetUInt32();
            if (!sObjectMgr->GetItemTemplate(productItem.ItemID))
            {
                TC_LOG_ERROR("sql.sql", "battlepay_product_item: Id {} (ProductID {}) references non-existing ItemID {}, skipped.", fields[0].GetUInt32(), productId, productItem.ItemID);
                continue;
            }

            productItem.ID = fields[0].GetUInt32();
            productItem.Quantity = fields[3].GetUInt32();
            productItem.PetResult = fields[5].GetUInt8();
            productItr->second.Items.push_back(productItem);
        } while (itemResult->NextRow());
    }

    TC_LOG_INFO("server.loading", ">> Loaded {} battlepay products in {} ms", _products.size(), GetMSTimeDiffToNow(oldMSTime));
}

void BattlePayMgr::LoadProductGroups()
{
    uint32 oldMSTime = getMSTime();
    _groups.clear();

    QueryResult result = WorldDatabase.Query("SELECT GroupID, Name, IconFileDataID, DisplayType, Ordering FROM battlepay_product_group");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 battlepay product groups. DB table `battlepay_product_group` is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        Battlepay::ProductGroup group;
        group.GroupID = fields[0].GetUInt32();
        group.Name = fields[1].GetString();
        group.IconFileDataID = fields[2].GetInt32();
        group.DisplayType = fields[3].GetUInt8();
        group.Ordering = fields[4].GetInt32();
        _groups.push_back(group);
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded {} battlepay product groups in {} ms", _groups.size(), GetMSTimeDiffToNow(oldMSTime));
}

void BattlePayMgr::LoadShopEntries()
{
    uint32 oldMSTime = getMSTime();
    _shopEntries.clear();

    QueryResult result = WorldDatabase.Query("SELECT EntryID, GroupID, ProductID, Ordering, Flags, BannerType, DisplayInfoID FROM battlepay_shop_entry");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 battlepay shop entries. DB table `battlepay_shop_entry` is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        Battlepay::ShopEntry entry;
        entry.EntryID = fields[0].GetUInt32();
        entry.GroupID = fields[1].GetUInt32();
        entry.ProductID = fields[2].GetUInt32();
        entry.Ordering = fields[3].GetInt32();
        entry.Flags = fields[4].GetUInt32();
        entry.BannerType = fields[5].GetUInt8();
        entry.DisplayInfoID = fields[6].GetUInt32();
        _shopEntries.push_back(entry);
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded {} battlepay shop entries in {} ms", _shopEntries.size(), GetMSTimeDiffToNow(oldMSTime));
}

void BattlePayMgr::LoadProductGroupLocales()
{
    _productGroupLocales.clear();

    QueryResult result = WorldDatabase.Query("SELECT Id, Locale, Name FROM battlepay_product_group_locales");
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();

        LocaleConstant locale = GetLocaleByName(fields[1].GetString());
        if (locale == LOCALE_none)
            continue;

        Battlepay::ProductGroupLocale& data = _productGroupLocales[fields[0].GetUInt32()];
        ObjectMgr::AddLocaleString(fields[2].GetString(), locale, data.Name);
    } while (result->NextRow());
}

void BattlePayMgr::LoadDisplayInfoLocales()
{
    _displayInfoLocales.clear();

    QueryResult result = WorldDatabase.Query("SELECT Id, Locale, Name1, Name2, Name3, Name4 FROM battlepay_display_info_locales");
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();

        LocaleConstant locale = GetLocaleByName(fields[1].GetString());
        if (locale == LOCALE_none)
            continue;

        Battlepay::DisplayInfoLocale& data = _displayInfoLocales[fields[0].GetUInt32()];
        ObjectMgr::AddLocaleString(fields[2].GetString(), locale, data.Name1);
        ObjectMgr::AddLocaleString(fields[3].GetString(), locale, data.Name2);
        ObjectMgr::AddLocaleString(fields[4].GetString(), locale, data.Name3);
        ObjectMgr::AddLocaleString(fields[5].GetString(), locale, data.Name4);
    } while (result->NextRow());
}

Battlepay::Product const* BattlePayMgr::GetProduct(uint32 productId) const
{
    return FindOrNull(_products, productId);
}

uint32 BattlePayMgr::GetProductGroupId(uint32 productId) const
{
    for (Battlepay::ShopEntry const& entry : _shopEntries)
        if (entry.ProductID == productId)
            return entry.GroupID;
    return 0;
}

Battlepay::DisplayInfo const* BattlePayMgr::GetDisplayInfo(uint32 id) const
{
    return FindOrNull(_displayInfos, id);
}

std::vector<WorldPackets::BattlePay::ProductDisplayVisualData> const* BattlePayMgr::GetDisplayInfoVisuals(uint32 id) const
{
    return FindOrNull(_visuals, id);
}

Battlepay::ProductGroupLocale const* BattlePayMgr::GetProductGroupLocale(uint32 id) const
{
    return FindOrNull(_productGroupLocales, id);
}

Battlepay::DisplayInfoLocale const* BattlePayMgr::GetDisplayInfoLocale(uint32 id) const
{
    return FindOrNull(_displayInfoLocales, id);
}

bool BattlePayMgr::AlreadyOwnProduct(Player const* player, uint32 itemId) const
{
    if (!player)
        return false;

    ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);
    if (!itemTemplate)
        return true;

    for (ItemEffectEntry const* effect : itemTemplate->Effects)
        if (effect->TriggerType == ITEM_SPELLTRIGGER_ON_LEARN && player->HasSpell(uint32(effect->SpellID)))
            return true;

    return player->GetItemCount(itemId) > 0;
}

bool BattlePayMgr::ProductFilter(Player const* player, Battlepay::Product const& product) const
{
    for (Battlepay::ProductItem const& item : product.Items)
    {
        if (!item.ItemID || !item.Quantity)
            return false;

        if (!sObjectMgr->GetItemTemplate(item.ItemID))
            return false;
    }

    if (!player)
        return true;

    if (product.ClassMask && (player->GetClassMask() & product.ClassMask) == 0)
        return false;

    for (Battlepay::ProductItem const& item : product.Items)
        if (AlreadyOwnProduct(player, item.ItemID))
            return false;

    return true;
}

std::tuple<bool, WorldPackets::BattlePay::ProductDisplayInfo> BattlePayMgr::WriteDisplayInfo(uint32 displayInfoID, LocaleConstant localeIndex, uint32 productId /*= 0*/) const
{
    WorldPackets::BattlePay::ProductDisplayInfo info;
    if (!displayInfoID)
        return std::make_tuple(false, info);

    Battlepay::DisplayInfo const* displayInfo = GetDisplayInfo(displayInfoID);
    if (!displayInfo)
        return std::make_tuple(false, info);

    Battlepay::DisplayInfoLocale const* displayLocale = GetDisplayInfoLocale(displayInfoID);

    info.Name1 = displayInfo->Name1;
    if (displayLocale)
        ObjectMgr::GetLocaleString(displayLocale->Name1, localeIndex, info.Name1);

    info.Name2 = displayInfo->Name2;
    if (displayLocale)
        ObjectMgr::GetLocaleString(displayLocale->Name2, localeIndex, info.Name2);

    info.Name3 = displayInfo->Name3;
    if (productId)
    {
        // Auto-generate a colored item list description for multi-item bundle products,
        // matching the reference's pack-description behavior.
        if (Battlepay::Product const* product = GetProduct(productId))
        {
            if (!product->Items.empty())
            {
                std::string packDescription;
                for (Battlepay::ProductItem const& item : product->Items)
                {
                    if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(item.ItemID))
                    {
                        char const* qualityColor;
                        switch (itemTemplate->GetQuality())
                        {
                            case ITEM_QUALITY_POOR:      qualityColor = "|cff9d9d9d"; break;
                            case ITEM_QUALITY_NORMAL:    qualityColor = "|cffffffff"; break;
                            case ITEM_QUALITY_UNCOMMON:  qualityColor = "|cff1eff00"; break;
                            case ITEM_QUALITY_RARE:      qualityColor = "|cff0070dd"; break;
                            case ITEM_QUALITY_EPIC:      qualityColor = "|cffa335ee"; break;
                            case ITEM_QUALITY_LEGENDARY: qualityColor = "|cffff8000"; break;
                            default:                     qualityColor = "|cffe5cc80"; break;
                        }
                        packDescription += qualityColor;
                        packDescription += itemTemplate->GetName(localeIndex);
                        packDescription += "\n";
                    }
                }
                info.Name3 = packDescription;
            }
        }
    }
    else if (displayLocale)
        ObjectMgr::GetLocaleString(displayLocale->Name3, localeIndex, info.Name3);

    info.Name4 = displayInfo->Name4;
    if (displayLocale)
        ObjectMgr::GetLocaleString(displayLocale->Name4, localeIndex, info.Name4);

    if (displayInfo->CreatureDisplayInfoID != 0)
        info.CreatureDisplayInfoID = displayInfo->CreatureDisplayInfoID;

    if (displayInfo->VisualsId)
    {
        if (auto const* visuals = GetDisplayInfoVisuals(displayInfoID))
        {
            info.VisualsId = displayInfo->VisualsId;
            for (WorldPackets::BattlePay::ProductDisplayVisualData const& visual : *visuals)
                info.Visuals.push_back(visual);
        }
    }

    if (displayInfo->Flags != 0)
        info.Flags = displayInfo->Flags;

    return std::make_tuple(true, info);
}

void BattlePayMgr::SendProductList(WorldSession* session) const
{
    WorldPackets::BattlePay::ProductListResponse response;
    if (!IsAvailable())
    {
        response.Result = 1; // Battlepay::ProductListResult::LockUnk1
        session->SendPacket(response.Write());
        return;
    }

    std::lock_guard<std::mutex> lock(_lock);

    Player const* player = session->GetPlayer();
    LocaleConstant localeIndex = session->GetSessionDbLocaleIndex();

    response.Result = 0; // Battlepay::ProductListResult::Available
    // Required client-side currency-symbol lookup key (CurrencyTypes.db2 ID), not a real-money
    // billing concept - 0 isn't a registered entry in that DB2 and left the Shop UI's loading
    // spinner stuck forever even though the rest of the product list parsed fine. See ARGUSCORE_FIXES.md.
    response.ProductList.CurrencyID = sWorld->getIntConfig(CONFIG_BATTLEPAY_CURRENCY);

    for (Battlepay::ProductGroup const& group : _groups)
    {
        WorldPackets::BattlePay::BattlePayProductGroup packetGroup;
        packetGroup.GroupID = group.GroupID;
        packetGroup.IconFileDataID = group.IconFileDataID;
        packetGroup.Ordering = group.Ordering;
        packetGroup.DisplayType = group.DisplayType;

        std::string name = group.Name;
        if (Battlepay::ProductGroupLocale const* locale = GetProductGroupLocale(group.GroupID))
            ObjectMgr::GetLocaleString(locale->Name, localeIndex, name);
        packetGroup.Name = name;

        response.ProductList.ProductGroup.push_back(packetGroup);
    }

    for (Battlepay::ShopEntry const& entry : _shopEntries)
    {
        WorldPackets::BattlePay::BattlePayShopEntry packetEntry;
        packetEntry.EntryID = entry.EntryID;
        packetEntry.GroupID = entry.GroupID;
        packetEntry.ProductID = entry.ProductID;
        packetEntry.Ordering = entry.Ordering;
        packetEntry.VasServiceType = entry.Flags;
        packetEntry.StoreDeliveryType = entry.BannerType;

        auto [hasDisplay, display] = WriteDisplayInfo(entry.DisplayInfoID, localeIndex);
        if (hasDisplay)
            packetEntry.DisplayInfo = display;

        response.ProductList.Shop.push_back(packetEntry);
    }

    for (auto const& [productId, product] : _products)
    {
        if (!ProductFilter(player, product))
            continue;

        WorldPackets::BattlePay::ProductInfoStruct info;
        info.NormalPriceFixedPoint = uint64(product.ShopPointsPrice * Battlepay::g_CurrencyPrecision);
        info.CurrentPriceFixedPoint = info.NormalPriceFixedPoint;
        info.ProductID = product.ProductID;
        info.ChoiceType = product.ChoiceType;
        info.ProductIDs.push_back(product.ProductID);
        // The reference cores hardcode this to 47 on every product (with their own "// 2 ?"
        // comment admitting they never identified what it actually represents) - this port's
        // rewrite dropped it entirely, leaving the struct default of 0. Restoring it verbatim:
        // found via careful re-comparison after every other avenue (wire-level byte tracing, a
        // real historical packet capture, the real 7.3.5 client Lua, the actual 7.3.5-targeted
        // upstream project) failed to explain why groups rendered empty despite byte-correct data.
        // See ARGUSCORE_FIXES.md.
        info.UnkInt2 = 47;

        auto [hasProductDisplay, productDisplay] = WriteDisplayInfo(product.DisplayInfoID, localeIndex);
        if (hasProductDisplay)
            info.DisplayInfo = productDisplay;

        response.ProductList.ProductInfo.push_back(info);

        WorldPackets::BattlePay::BattlePayProduct packetProduct;
        packetProduct.ProductID = product.ProductID;
        packetProduct.Flags = product.Flags;
        packetProduct.Type = product.Type;

        if (product.WebsiteType == Battlepay::CharacterBoost)
            packetProduct.UnkBits = product.ScriptName.find("level90") != std::string::npos ? 1 : 2;

        for (Battlepay::ProductItem const& item : product.Items)
        {
            WorldPackets::BattlePay::ProductItem packetItem;
            packetItem.ID = item.ID;
            packetItem.ItemID = product.Items.size() > 1 ? 0 : item.ItemID; // client only tooltips a single item for packs
            packetItem.Quantity = item.Quantity;
            packetItem.HasPet = AlreadyOwnProduct(player, item.ItemID);
            packetItem.PetResult = item.PetResult;

            auto [hasItemDisplay, itemDisplay] = WriteDisplayInfo(item.DisplayInfoID, localeIndex);
            if (hasItemDisplay)
                packetItem.DisplayInfo = itemDisplay;

            packetProduct.Items.push_back(packetItem);
        }

        auto [hasProductDisplay2, productDisplay2] = WriteDisplayInfo(product.DisplayInfoID, localeIndex);
        if (hasProductDisplay2)
            packetProduct.DisplayInfo = productDisplay2;

        response.ProductList.Product.push_back(packetProduct);
    }

    session->SendPacket(response.Write());
}

void BattlePayMgr::SendPurchaseUpdate(WorldSession* session, Battlepay::Purchase const& purchase, uint32 resultCode) const
{
    WorldPackets::BattlePay::BattlePayPurchase data;
    data.PurchaseID = purchase.PurchaseID;
    data.Status = purchase.Status;
    data.ResultCode = resultCode;
    data.ProductID = purchase.ProductID;
    data.UnkInt = purchase.ServerToken;
    data.WalletName = "Shop Points";

    WorldPackets::BattlePay::PurchaseUpdate packet;
    packet.Purchase.push_back(data);
    session->SendPacket(packet.Write());
}

void BattlePayMgr::StartPurchase(WorldSession* session, ObjectGuid targetCharacter, uint32 clientToken, uint32 productID)
{
    uint32 battlenetAccountId = session->GetBattlenetAccountId();

    auto deny = [session, clientToken]()
    {
        WorldPackets::BattlePay::StartPurchaseResponse response;
        response.ClientToken = clientToken;
        response.PurchaseResult = Battlepay::PurchaseDenied;
        session->SendPacket(response.Write());
    };

    if (!IsAvailable())
    {
        deny();
        return;
    }

    Battlepay::Purchase purchase;
    {
        std::lock_guard<std::mutex> lock(_lock);

        // Reject re-entry while a confirm is already being processed for this account - a second
        // StartPurchase before that finishes must not reset the in-flight entry out from under
        // delivery. A second StartPurchase while UNLOCKED (no confirm in progress yet) is legitimate
        // (retry, or picked a different item) and is allowed to overwrite below.
        auto it = _purchases.find(battlenetAccountId);
        if (it != _purchases.end() && it->second.Lock)
        {
            deny();
            return;
        }

        auto productIt = _products.find(productID);
        if (productIt == _products.end())
        {
            deny();
            return;
        }
        Battlepay::Product const& product = productIt->second;

        if (!ProductFilter(session->GetPlayer(), product))
        {
            deny();
            return;
        }

        purchase.ProductID = productID;
        purchase.ClientToken = clientToken;
        purchase.TargetCharacter = targetCharacter;
        purchase.CurrentPrice = product.ShopPointsPrice;
        purchase.PurchaseID = _purchaseIdCounter.fetch_add(1);
        purchase.DistributionId = _distributionIdCounter.fetch_add(1);
        purchase.ServerToken = urand(0, 0xFFFFFFF);
        purchase.Status = Battlepay::UpdateStatus::Loading;
        purchase.Lock = false;

        _purchases[battlenetAccountId] = purchase;
    }

    // Balance MUST be checked here, before ever telling the client Ok - not deferred to
    // ConfirmPurchase. Confirmed from the real client Lua (Blizzard_StoreUISecure.lua): the
    // confirmation dialog's own Buy button sets JustOrderedProduct=true, and once any
    // SMSG_BATTLE_PAY_PURCHASE_UPDATE arrives after that, the client unconditionally shows
    // "Purchase Sent" (JustFinishedOrdering) regardless of Status/result code - there is no
    // error-surfacing path tied to PurchaseUpdate at all. The client's only real error path
    // (STORE_ORDER_INITIATION_FAILED) is tied to a nonzero StartPurchaseResponse, which only works
    // if it arrives before the confirmation dialog is ever shown - exactly how the reference does
    // it (synchronously, at StartPurchase time). Kept async here (unlike the reference's blocking
    // LoginDatabase.Query()) by deferring the StartPurchaseResponse/PurchaseUpdate/ConfirmPurchase
    // send into the query callback instead. See ARGUSCORE_FIXES.md.
    LoginDatabasePreparedStatement* balanceStmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_BATTLEPAY_BALANCE);
    balanceStmt->setUInt32(0, battlenetAccountId);
    session->GetQueryProcessor().AddCallback(LoginDatabase.AsyncQuery(balanceStmt).WithPreparedCallback(
        [this, session, purchase, battlenetAccountId](PreparedQueryResult result)
        {
            uint64 balance = result ? result->Fetch()[0].GetUInt64() : 0;
            if (balance < purchase.CurrentPrice)
            {
                WorldPackets::BattlePay::StartPurchaseResponse response;
                response.ClientToken = purchase.ClientToken;
                response.PurchaseResult = Battlepay::InsufficientBalance;
                session->SendPacket(response.Write());

                std::lock_guard<std::mutex> lock(_lock);
                _purchases.erase(battlenetAccountId);
                return;
            }

            WorldPackets::BattlePay::StartPurchaseResponse response;
            response.PurchaseID = purchase.PurchaseID;
            response.ClientToken = purchase.ClientToken;
            response.PurchaseResult = Battlepay::Ok;
            session->SendPacket(response.Write());

            SendPurchaseUpdate(session, purchase, Battlepay::Ok);

            WorldPackets::BattlePay::ConfirmPurchase confirmPacket;
            confirmPacket.PurchaseID = purchase.PurchaseID;
            confirmPacket.ServerToken = purchase.ServerToken;
            session->SendPacket(confirmPacket.Write());
        }));
}

void BattlePayMgr::ConfirmPurchase(WorldSession* session, WorldPackets::BattlePay::ConfirmPurchaseResponse const& confirmPurchase)
{
    uint32 battlenetAccountId = session->GetBattlenetAccountId();

    Battlepay::Purchase purchase;
    Battlepay::Product product;
    bool haveProduct = false;
    bool denied = false;
    {
        std::lock_guard<std::mutex> lock(_lock);

        auto it = _purchases.find(battlenetAccountId);
        if (it == _purchases.end())
            return; // nothing in flight - no session-bound fallback to report against, just ignore

        // Anti-replay/anti-tamper: reject without mutating any state if the confirm doesn't
        // genuinely match the currently in-flight purchase. Ported from the reference's four-way
        // check (single-currency version - no per-currency match needed).
        uint64 clientPrice = confirmPurchase.ClientCurrentPriceFixedPoint / uint64(Battlepay::g_CurrencyPrecision);
        if (it->second.Lock ||
            it->second.ServerToken != confirmPurchase.ServerToken ||
            !confirmPurchase.ConfirmPurchase ||
            it->second.CurrentPrice != clientPrice)
        {
            purchase = it->second;
            denied = true;
        }
        else
        {
            auto productIt = _products.find(it->second.ProductID);
            if (productIt == _products.end())
            {
                purchase = it->second;
                denied = true;
                _purchases.erase(it);
            }
            else
            {
                it->second.Lock = true;
                purchase = it->second;
                product = productIt->second;
                haveProduct = true;
            }
        }
    }

    if (denied)
    {
        // Status is deliberately left as-is (still Loading), not bumped to Finish - matches the
        // reference's own anti-replay/tamper denial exactly. The client reads Status == Finish as
        // "transaction is done" independent of the result code, so marking a denial Finish makes it
        // show as if the purchase succeeded ("item sent") instead of surfacing the denial. See
        // ARGUSCORE_FIXES.md.
        SendPurchaseUpdate(session, purchase, Battlepay::PurchaseDenied);
        return;
    }

    if (!haveProduct)
        return;

    SendPurchaseUpdate(session, purchase, Battlepay::Ok);

    // Async balance check (non-mutating) - matches the codebase's established async
    // LoginDatabase query+callback idiom (CharacterHandler.cpp), not the reference's blocking
    // synchronous LoginDatabase.Query() call. Registered on the SESSION's own query processor (not
    // this manager's) so the captured `session` pointer is only ever dereferenced while that exact
    // session is alive pumping its own queue - see BattlePayMgr.h for why.
    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_BATTLEPAY_BALANCE);
    stmt->setUInt32(0, battlenetAccountId);
    session->GetQueryProcessor().AddCallback(LoginDatabase.AsyncQuery(stmt).WithPreparedCallback(
        [this, session, purchase, product](PreparedQueryResult result)
        {
            uint64 balance = result ? result->Fetch()[0].GetUInt64() : 0;
            if (balance < purchase.CurrentPrice)
            {
                // PurchaseDenied (not InsufficientBalance) and Status left as Loading, not Finish -
                // matches the reference's real confirm-time balance check exactly (InsufficientBalance
                // is only used by the reference's earlier, synchronous StartPurchase-time check, which
                // this port doesn't have - the balance check is deferred to here since it's async).
                // See ARGUSCORE_FIXES.md for why Finish must never be set on a denial.
                SendPurchaseUpdate(session, purchase, Battlepay::PurchaseDenied);

                std::lock_guard<std::mutex> lock(_lock);
                _purchases.erase(session->GetBattlenetAccountId());
                return;
            }

            DeliverAndDeduct(session, purchase, product);
        }));
}

void BattlePayMgr::DeliverAndDeduct(WorldSession* session, Battlepay::Purchase purchase, Battlepay::Product product)
{
    Player* onlinePlayer = session->GetPlayer();
    bool isTargetCharacterOnline = onlinePlayer && onlinePlayer->GetGUID() == purchase.TargetCharacter;

    // Re-check "already own" here (matching the reference's re-validation at confirm time) to close
    // a race where the player acquired the item some other way between StartPurchase and here (e.g.
    // a previously-purchased mail arriving). Only possible to check when the target character is
    // actually loaded in memory - an offline target's inventory can't be inspected without loading
    // that character, so this is a best-effort close, not a hard guarantee, same limitation the
    // reference itself has.
    if (isTargetCharacterOnline && !ProductFilter(onlinePlayer, product))
    {
        // Status left as Loading, not Finish - see the ConfirmPurchase denial branch above for why.
        SendPurchaseUpdate(session, purchase, Battlepay::PurchaseDenied);

        std::lock_guard<std::mutex> lock(_lock);
        _purchases.erase(session->GetBattlenetAccountId());
        return;
    }

    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

    CharacterDatabasePreparedStatement* logStmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_BATTLEPAY_PURCHASE_LOG);
    logStmt->setUInt32(0, session->GetBattlenetAccountId());
    logStmt->setUInt32(1, session->GetAccountId());
    logStmt->setUInt64(2, purchase.TargetCharacter.GetCounter());
    logStmt->setUInt32(3, purchase.ProductID);
    logStmt->setUInt64(4, purchase.PurchaseID);
    logStmt->setUInt64(5, purchase.CurrentPrice);
    logStmt->setUInt8(6, 0); // DeliveryState: delivered-pending-deduct (see migration file comment)
    trans->Append(logStmt);

    // Per-item, not a single up-front "do we have space for everything" check: checking each item's
    // space independently ahead of time can't account for space consumed by an earlier item in the
    // same bundle, which could otherwise let a later item silently fail to store with nothing
    // noticing. Instead, attempt online storage per item and mail whichever specific items don't
    // fit - nothing is ever silently dropped, and a single mail with the leftovers is sent if any
    // items needed it.
    std::vector<Battlepay::ProductItem> mailFallbackItems;
    if (isTargetCharacterOnline)
    {
        for (Battlepay::ProductItem const& item : product.Items)
        {
            ItemPosCountVec dest;
            InventoryResult msg = onlinePlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, item.ItemID, item.Quantity);
            if (msg == EQUIP_ERR_OK)
            {
                if (Item* newItem = onlinePlayer->StoreNewItem(dest, item.ItemID, true, GenerateItemRandomPropertyId(item.ItemID)))
                {
                    onlinePlayer->SendNewItem(newItem, item.Quantity, false, true);
                    continue;
                }
            }

            mailFallbackItems.push_back(item);
        }
    }
    else
    {
        mailFallbackItems = product.Items;
    }

    if (!mailFallbackItems.empty())
    {
        MailSender sender(MAIL_NORMAL, UI64LIT(0), MAIL_STATIONERY_GM);
        MailDraft draft("Shop Purchase", "Thank you for your purchase.");

        for (Battlepay::ProductItem const& item : mailFallbackItems)
        {
            if (Item* mailItem = Item::CreateItem(item.ItemID, item.Quantity, ItemContext::NONE))
            {
                mailItem->SaveToDB(trans);
                draft.AddItem(mailItem);
            }
        }

        draft.SendMailTo(trans, MailReceiver(isTargetCharacterOnline ? onlinePlayer : nullptr, purchase.TargetCharacter.GetCounter()), sender);
    }

    session->AddTransactionCallback(CharacterDatabase.AsyncCommitTransaction(trans)).AfterComplete(
        [this, session, purchase](bool success)
        {
            if (!success)
            {
                TC_LOG_ERROR("server.battlepay", "BattlePay: delivery transaction failed for BattlenetAccountId {} ProductID {} PurchaseID {} - purchase NOT charged, nothing delivered.",
                    session->GetBattlenetAccountId(), purchase.ProductID, purchase.PurchaseID);

                // Status left as Loading, not Finish - see the ConfirmPurchase denial branch above for
                // why (nothing was delivered here either, so this must not read as success client-side).
                SendPurchaseUpdate(session, purchase, Battlepay::Other);

                std::lock_guard<std::mutex> lock(_lock);
                _purchases.erase(session->GetBattlenetAccountId());
                return;
            }

            // Delivery is committed. Only now deduct the balance - see ARGUSCORE_FIXES.md for why
            // this order (never risk taking currency without having already delivered the item).
            LoginDatabaseTransaction loginTrans = LoginDatabase.BeginTransaction();
            LoginDatabasePreparedStatement* deductStmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_BATTLEPAY_BALANCE_DEDUCT);
            deductStmt->setUInt64(0, purchase.CurrentPrice);
            deductStmt->setUInt32(1, session->GetBattlenetAccountId());
            loginTrans->Append(deductStmt);

            session->AddTransactionCallback(LoginDatabase.AsyncCommitTransaction(loginTrans)).AfterComplete(
                [this, session, purchase](bool deductSuccess)
                {
                    CharacterDatabasePreparedStatement* stateStmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_BATTLEPAY_PURCHASE_LOG_STATE);
                    stateStmt->setUInt8(0, deductSuccess ? 1 : 2); // 1=Delivered/paid, 2=DeliveryFailed (payment uncollected)
                    stateStmt->setUInt64(1, purchase.PurchaseID);
                    CharacterDatabase.Execute(stateStmt);

                    if (!deductSuccess)
                    {
                        // Balance changed underneath us (e.g. a concurrent credit-queue apply raced
                        // this deduct into an underflow, rejected by sql_mode=STRICT_TRANS_TABLES).
                        // The item is genuinely already delivered - the server eats the cost rather
                        // than risk taking currency without delivering. Flagged for admin
                        // reconciliation via the ERROR log and DeliveryState=2 above; still tell the
                        // client Ok, since telling them otherwise after the item already arrived
                        // would just be confusing.
                        TC_LOG_ERROR("server.battlepay", "BattlePay: balance deduct failed for BattlenetAccountId {} ProductID {} Price {} PurchaseID {} (item already delivered) - needs admin reconciliation.",
                            session->GetBattlenetAccountId(), purchase.ProductID, purchase.CurrentPrice, purchase.PurchaseID);
                    }

                    Battlepay::Purchase finished = purchase;
                    finished.Status = Battlepay::UpdateStatus::Finish;
                    SendPurchaseUpdate(session, finished, Battlepay::Ok);
                    SendProductList(session); // refreshes "already owned" filtering, matching the reference's own final call

                    std::lock_guard<std::mutex> lock(_lock);
                    _purchases.erase(session->GetBattlenetAccountId());
                });
        });
}

