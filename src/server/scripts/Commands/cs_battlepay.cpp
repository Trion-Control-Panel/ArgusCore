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

// GM testing command for BattlePay Shop Points, letting an admin credit a balance manually without
// touching an external site's own database writes. Uses the exact same
// INSERT ... ON DUPLICATE KEY UPDATE statement documented for external sites to use directly
// against battlepay_account_balance - not a separate balance-mutation code path. See
// ARGUSCORE_FIXES.md.

#include "AccountMgr.h"
#include "Chat.h"
#include "ChatCommand.h"
#include "DatabaseEnv.h"
#include "Language.h"
#include "RBAC.h"
#include "ScriptMgr.h"

using namespace Trinity::ChatCommands;

class battlepay_commandscript : public CommandScript
{
public:
    battlepay_commandscript() : CommandScript("battlepay_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable battlepayCommandTable =
        {
            // Reuses RBAC_PERM_COMMAND_SEND_MONEY rather than a dedicated new permission - both are
            // "admin grants currency to a player" in spirit; a standalone permission can be split
            // out later if finer-grained access control is ever needed for this specific command.
            { "credit", HandleBattlePayCreditCommand, rbac::RBAC_PERM_COMMAND_SEND_MONEY, Console::Yes },
        };

        static ChatCommandTable commandTable =
        {
            { "battlepay", battlepayCommandTable },
        };
        return commandTable;
    }

    static bool HandleBattlePayCreditCommand(ChatHandler* handler, std::string accountName, int64 amount)
    {
        if (amount <= 0)
        {
            handler->PSendSysMessage("Amount must be positive.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 accountId = AccountMgr::GetId(accountName);
        if (!accountId)
        {
            handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, accountName.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_BATTLENET_ACCOUNT_BY_ACCOUNT_ID);
        stmt->setUInt32(0, accountId);
        PreparedQueryResult result = LoginDatabase.Query(stmt); // sync: rare admin-triggered op, matches the codebase's existing GM-command precedent (not the async purchase-flow hot path)
        if (!result || result->Fetch()[0].IsNull())
        {
            handler->PSendSysMessage("Account '%s' has no linked Battle.net account.", accountName.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 bnetAccountId = result->Fetch()[0].GetUInt32();

        LoginDatabasePreparedStatement* creditStmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_BATTLEPAY_BALANCE_CREDIT);
        creditStmt->setUInt32(0, bnetAccountId);
        creditStmt->setUInt64(1, uint64(amount));
        LoginDatabase.Execute(creditStmt);

        handler->PSendSysMessage("Credited %lld Shop Points to account '%s' (BattlenetAccountId %u).", (long long)amount, accountName.c_str(), bnetAccountId);
        return true;
    }
};

void AddSC_battlepay_commandscript()
{
    new battlepay_commandscript();
}
