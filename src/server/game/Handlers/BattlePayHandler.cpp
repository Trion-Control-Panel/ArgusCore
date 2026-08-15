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

// BattlePay (in-game Shop) opcode handlers. Phase 1: product list display. Phase 2: purchase flow
// (StartPurchase/ConfirmPurchase). See ARGUSCORE_FIXES.md for the full design writeup.

#include "WorldSession.h"
#include "BattlePayMgr.h"
#include "BattlePayPackets.h"

void WorldSession::HandleBattlePayGetProductList(WorldPackets::BattlePay::GetProductList& /*getProductList*/)
{
    sBattlePayMgr->SendProductList(this);
}

// The Shop UI queries this immediately on open, alongside GetProductList. Purchase history
// doesn't exist until the purchase flow (a later phase) is implemented, so this always answers
// with an empty list - but it must answer, or the client's loading UI never dismisses.
void WorldSession::HandleBattlePayGetPurchaseList(WorldPackets::BattlePay::GetPurchaseListQuery& /*getPurchaseList*/)
{
    SendPacket(WorldPackets::BattlePay::PurchaseListResponse().Write());
}

// Third query the Shop UI fires on open, found via live opcode-trace logging after the other two
// still left the loading screen stuck - doesn't match the CMSG_BATTLE_PAY_* name pattern, which is
// why it was missed initially. 2/2 reference cores treat it as a pure ack, no response needed.
void WorldSession::HandleUpdateVasPurchaseStates(WorldPackets::BattlePay::UpdateVasPurchaseStates& /*updateVasPurchaseStates*/)
{
}

// Called once at login (WorldSession::InitializeSessionCallback), not in response to any client
// request. The real client Lua (Blizzard_StoreUISecure.lua) keeps the Shop frame on its loading
// screen until it has a product list, a purchase list, AND a distribution list - this satisfies
// that third condition. Also doubles as the character-select "you have a Character Boost credit"
// signal (C_SharedCharacterServices.GetUpgradeDistributions() client-side, confirmed from the real
// 7.3.5 client Lua, GlueXML/CharacterSelect.lua) - one real BattlePayDistributionObject per
// unredeemed battlepay_pending_boost row for this account, built by SendDistributionList. See
// ARGUSCORE_FIXES.md.
void WorldSession::SendDisplayPromo(int32 promotionID /*= 0*/)
{
    SendPacket(WorldPackets::BattlePay::DisplayPromotion(uint32(promotionID)).Write());

    if (!sBattlePayMgr->IsAvailable())
        return;

    sBattlePayMgr->SendDistributionList(this);
}

void WorldSession::HandleBattlePayStartPurchase(WorldPackets::BattlePay::StartPurchase& startPurchase)
{
    sBattlePayMgr->StartPurchase(this, startPurchase.TargetCharacter, startPurchase.ClientToken, startPurchase.ProductID);
}

// Second CMSG opcode the real client actually sends for a purchase (client's
// C_StoreSecure.PurchaseProduct) - CMSG_BATTLE_PAY_START_PURCHASE had been assigned this opcode's
// numeric value (0x36F2) by mistake, and the real CMSG_BATTLE_PAY_START_PURCHASE value (0x36C8) was
// missing entirely, so purchases never reached the server at all. Fixed in Opcodes.h; both reference
// cores (DestinyCore/AshamaneCore) wire both CMSG opcodes to identical purchase-start logic, so this
// is wired the same way here. See ARGUSCORE_FIXES.md.
void WorldSession::HandleBattlePayPurchaseProduct(WorldPackets::BattlePay::PurchaseProduct& purchaseProduct)
{
    sBattlePayMgr->StartPurchase(this, purchaseProduct.TargetCharacter, purchaseProduct.ClientToken, purchaseProduct.ProductID);
}

void WorldSession::HandleBattlePayConfirmPurchase(WorldPackets::BattlePay::ConfirmPurchaseResponse& confirmPurchase)
{
    sBattlePayMgr->ConfirmPurchase(this, confirmPurchase);
}

void WorldSession::HandleBattlePayDistributionAssign(WorldPackets::BattlePay::DistributionAssignToTarget& distributionAssign)
{
    sBattlePayMgr->AssignDistributionToCharacter(this, distributionAssign);
}
