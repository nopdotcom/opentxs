// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/ServerAction.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/cash/Purse.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/Utility.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/ext/OTPayment.hpp"

#include "client/OTAPI_Func.hpp"

#include "ServerAction.hpp"

#define OT_METHOD "opentxs::api::client::implementation::ServerAction::"

namespace opentxs
{
api::client::ServerAction* Factory::ServerAction(
    const api::client::Manager& api,
    const ContextLockCallback& lockCallback)
{
    return new api::client::implementation::ServerAction(api, lockCallback);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
ServerAction::ServerAction(
    const api::client::Manager& api,
    const ContextLockCallback& lockCallback)
    : api_(api)
    , lock_callback_(lockCallback)
{
    // WARNING: do not access api_.Wallet() during construction
}

ServerAction::Action ServerAction::AcknowledgeBailment(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& recipientID,
    const Identifier& requestID,
    const std::string& instructions) const
{
    return Action(new OTAPI_Func(
        ACKNOWLEDGE_BAILMENT,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        recipientID,
        requestID,
        instructions));
}

ServerAction::Action ServerAction::AcknowledgeConnection(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& recipientID,
    const Identifier& requestID,
    const bool ack,
    const std::string& url,
    const std::string& login,
    const std::string& password,
    const std::string& key) const
{
    return Action(new OTAPI_Func(
        ACKNOWLEDGE_CONNECTION,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        recipientID,
        requestID,
        url,
        login,
        password,
        key,
        ack));
}

ServerAction::Action ServerAction::AcknowledgeNotice(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& recipientID,
    const Identifier& requestID,
    const bool ack) const
{
    return Action(new OTAPI_Func(
        ACKNOWLEDGE_NOTICE,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        recipientID,
        requestID,
        ack));
}

ServerAction::Action ServerAction::AcknowledgeOutbailment(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& recipientID,
    const Identifier& requestID,
    const std::string& details) const
{
    return Action(new OTAPI_Func(
        ACKNOWLEDGE_OUTBAILMENT,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        recipientID,
        requestID,
        details));
}

ServerAction::Action ServerAction::ActivateSmartContract(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const std::string& agentName,
    std::unique_ptr<OTSmartContract>& contract) const
{
    if (!GetTransactionNumbers(localNymID, serverID, 10)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain transaction numbers.")
            .Flush();
    }

    return Action(new OTAPI_Func(
        ACTIVATE_SMART_CONTRACT,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        accountID,
        agentName,
        contract));
}

ServerAction::Action ServerAction::AddServerClaim(
    const Identifier& localNymID,
    const Identifier& serverID,
    const proto::ContactSectionName section,
    const proto::ContactItemType type,
    const std::string& value,
    bool primary) const
{
    return Action(new OTAPI_Func(
        SERVER_ADD_CLAIM,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        primary,
        section,
        type,
        value));
}

ServerAction::Action ServerAction::AdjustUsageCredits(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetNymID,
    const Amount adjustment) const
{
    return Action(new OTAPI_Func(
        ADJUST_USAGE_CREDITS,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        targetNymID,
        adjustment));
}

ServerAction::Action ServerAction::CancelPaymentPlan(
    const Identifier& localNymID,
    const Identifier& serverID,
    std::unique_ptr<OTPaymentPlan>& plan) const
{
    // NOTE: Normally the SENDER (PAYER) is the one who deposits a payment plan.
    // But in this case, the RECIPIENT (PAYEE) deposits it -- which means
    // "Please cancel this plan." It SHOULD fail, since it's only been signed
    // by the recipient, and not the sender. And that failure is what burns
    // the transaction number on the plan, so that it can no longer be used.
    //
    // So how do we know the difference between an ACTUAL "failure" versus a
    // purposeful "failure" ? Because if the failure comes from cancelling the
    // plan, the server reply transaction will have IsCancelled() set to true.
    if (!GetTransactionNumbers(localNymID, serverID, 1)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain transaction numbers.")
            .Flush();
    }

    return Action(new OTAPI_Func(
        DEPOSIT_PAYMENT_PLAN,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        plan->GetRecipientAcctID(),
        plan));
}

ServerAction::Action ServerAction::CreateMarketOffer(
    const Identifier& assetAccountID,
    const Identifier& currencyAccountID,
    const Amount scale,
    const Amount increment,
    const Amount quantity,
    const Amount price,
    const bool selling,
    const std::chrono::seconds lifetime,
    const std::string& stopSign,
    const Amount activationPrice) const
{
    auto notaryID = Identifier::Factory();
    auto nymID = Identifier::Factory();
    const auto assetAccount = api_.Wallet().Account(assetAccountID);

    if (assetAccount) {
        nymID = assetAccount.get().GetNymID();
        notaryID = assetAccount.get().GetPurportedNotaryID();
    }

    if (!GetTransactionNumbers(nymID, notaryID, 10)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain transaction numbers.")
            .Flush();
    }

    return Action(new OTAPI_Func(
        CREATE_MARKET_OFFER,
        lock_callback_({nymID->str(), notaryID->str()}),
        api_,
        nymID,
        notaryID,
        assetAccountID,
        currencyAccountID,
        scale,
        increment,
        quantity,
        price,
        selling,
        lifetime.count(),
        activationPrice,
        stopSign));
}

#if OT_CASH
ServerAction::Action ServerAction::DepositCashPurse(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    std::unique_ptr<Purse>& purse) const
{
    if (!GetTransactionNumbers(localNymID, serverID, 1)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain transaction numbers.")
            .Flush();
    }

    return Action(new OTAPI_Func(
        DEPOSIT_CASH,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        accountID,
        purse));
}
#endif  // OT_CASH

ServerAction::Action ServerAction::DepositCheque(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    std::unique_ptr<Cheque>& cheque) const
{
    if (!GetTransactionNumbers(localNymID, serverID, 1)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain transaction numbers.")
            .Flush();
    }

    return Action(new OTAPI_Func(
        DEPOSIT_CHEQUE,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        accountID,
        cheque));
}

ServerAction::Action ServerAction::DepositPaymentPlan(
    const Identifier& localNymID,
    const Identifier& serverID,
    std::unique_ptr<OTPaymentPlan>& plan) const
{
    return Action(new OTAPI_Func(
        DEPOSIT_PAYMENT_PLAN,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        plan->GetSenderAcctID(),
        plan));
}

bool ServerAction::DownloadAccount(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const bool forceDownload) const
{
    rLock lock(lock_callback_({localNymID.str(), serverID.str()}));
    auto context = api_.Wallet().mutable_ServerContext(localNymID, serverID);
    Utility MsgUtil(context.It(), api_);
    const auto output = MsgUtil.getIntermediaryFiles(
        serverID.str(), localNymID.str(), accountID.str(), forceDownload);

    return output;
}

ServerAction::Action ServerAction::DownloadBoxReceipt(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const RemoteBoxType box,
    const TransactionNumber item) const
{
    return Action(new OTAPI_Func(
        GET_BOX_RECEIPT,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        accountID,
        box,
        item));
}

ServerAction::Action ServerAction::DownloadContract(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& contractID) const
{
    return Action(new OTAPI_Func(
        GET_CONTRACT,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        contractID));
}

ServerAction::Action ServerAction::DownloadMarketList(
    const Identifier& localNymID,
    const Identifier& serverID) const
{
    return Action(new OTAPI_Func(
        GET_MARKET_LIST,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID));
}

ServerAction::Action ServerAction::DownloadMarketOffers(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& marketID,
    const Amount depth) const
{
    return Action(new OTAPI_Func(
        GET_MARKET_OFFERS,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        marketID,
        depth));
}

ServerAction::Action ServerAction::DownloadMarketRecentTrades(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& marketID) const
{
    return Action(new OTAPI_Func(
        GET_MARKET_RECENT_TRADES,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        marketID));
}

#if OT_CASH
ServerAction::Action ServerAction::DownloadMint(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& instrumentDefinitionID) const
{
    return Action(new OTAPI_Func(
        GET_MINT,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        instrumentDefinitionID));
}
#endif  // OT_CASH

bool ServerAction::GetTransactionNumbers(
    const Identifier& localNymID,
    const Identifier& serverID,
    const std::size_t quantity) const
{
    rLock lock(lock_callback_({localNymID.str(), serverID.str()}));
    auto context = api_.Wallet().mutable_ServerContext(localNymID, serverID);
    Utility MsgUtil(context.It(), api_);
    auto available = context.It().AvailableNumbers();

    if (available < quantity) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Asking server for more numbers.")
            .Flush();
        auto action = Action(new OTAPI_Func(
            GET_TRANSACTION_NUMBERS,
            lock_callback_({localNymID.str(), serverID.str()}),
            api_,
            localNymID,
            serverID));
        auto response = action->Run();
        if (response.empty()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to obtain ")(quantity)(
                " numbers.")
                .Flush();
            return false;
        }

        bool msgWasSent{false};
        const auto download = MsgUtil.getAndProcessNymbox_4(
            serverID.str(), localNymID.str(), msgWasSent, false);

        if (0 > download) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to retrieve nym.")
                .Flush();

            return false;
        }

        // Try again.
        available = context.It().AvailableNumbers();

        if (available < quantity) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to obtain ")(quantity)(
                " numbers.")
                .Flush();

            return false;
        }
    }

    return true;
}

ServerAction::Action ServerAction::DownloadNym(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetNymID) const
{
    return Action(new OTAPI_Func(
        CHECK_NYM,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        targetNymID));
}

bool ServerAction::DownloadNymbox(
    const Identifier& localNymID,
    const Identifier& serverID) const
{
    rLock lock(lock_callback_({localNymID.str(), serverID.str()}));
    auto context = api_.Wallet().mutable_ServerContext(localNymID, serverID);
    Utility util(context.It(), api_);

    if (0 >= context.It().UpdateRequestNumber()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed calling update request number.")
            .Flush();

        return false;
    }

    bool msgWasSent{false};
    const auto download = util.getAndProcessNymbox_4(
        serverID.str(), localNymID.str(), msgWasSent, true);

    if (0 > download) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to retrieve nymbox.")
            .Flush();

        return false;
    }

    return true;
}

ServerAction::Action ServerAction::DownloadNymMarketOffers(
    const Identifier& localNymID,
    const Identifier& serverID) const
{
    return Action(new OTAPI_Func(
        GET_NYM_MARKET_OFFERS,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID));
}

ServerAction::Action ServerAction::ExchangeBasketCurrency(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& instrumentDefinitionID,
    const Identifier& accountID,
    const Identifier& basketID,
    const bool direction) const
{
    if (!GetTransactionNumbers(localNymID, serverID, 1)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain transaction numbers.")
            .Flush();
    }

    return Action(new OTAPI_Func(
        EXCHANGE_BASKET,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        instrumentDefinitionID,
        basketID,
        accountID,
        direction,
        api_.OTAPI().GetBasketMemberCount(basketID)));
}

#if OT_CASH
ServerAction::Action ServerAction::ExchangeCash(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& instrumentDefinitionID,
    std::unique_ptr<Purse>& purse) const
{
    if (!GetTransactionNumbers(localNymID, serverID, 1)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain transaction numbers.")
            .Flush();
    }

    return Action(new OTAPI_Func(
        EXCHANGE_CASH,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        instrumentDefinitionID,
        purse));
}
#endif  // OT_CASH

ServerAction::Action ServerAction::InitiateBailment(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetNymID,
    const Identifier& instrumentDefinitionID) const
{
    return Action(new OTAPI_Func(
        INITIATE_BAILMENT,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        targetNymID,
        instrumentDefinitionID));
}

ServerAction::Action ServerAction::InitiateOutbailment(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetNymID,
    const Identifier& instrumentDefinitionID,
    const Amount amount,
    const std::string& message) const
{
    return Action(new OTAPI_Func(
        INITIATE_OUTBAILMENT,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        targetNymID,
        instrumentDefinitionID,
        amount,
        message));
}

ServerAction::Action ServerAction::InitiateRequestConnection(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetNymID,
    const proto::ConnectionInfoType& type) const
{
    return Action(new OTAPI_Func(
        REQUEST_CONNECTION,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        targetNymID,
        type));
}

ServerAction::Action ServerAction::InitiateStoreSecret(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetNymID,
    const proto::SecretType& type,
    const std::string& primary,
    const std::string& secondary) const
{
    return Action(new OTAPI_Func(
        STORE_SECRET,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        targetNymID,
        primary,
        secondary,
        type));
}

ServerAction::Action ServerAction::IssueBasketCurrency(
    const Identifier& localNymID,
    const Identifier& serverID,
    const proto::UnitDefinition& basket,
    const std::string& label) const
{
    return Action(new OTAPI_Func(
        ISSUE_BASKET,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        basket,
        label));
}

ServerAction::Action ServerAction::IssueUnitDefinition(
    const Identifier& localNymID,
    const Identifier& serverID,
    const proto::UnitDefinition& contract,
    const std::string& label) const
{
    return Action(new OTAPI_Func(
        ISSUE_ASSET_TYPE,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        contract,
        label));
}

ServerAction::Action ServerAction::KillMarketOffer(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const TransactionNumber number) const
{
    if (!GetTransactionNumbers(localNymID, serverID, 1)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain transaction numbers.")
            .Flush();
    }

    return Action(new OTAPI_Func(
        KILL_MARKET_OFFER,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        accountID,
        number));
}

ServerAction::Action ServerAction::KillPaymentPlan(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const TransactionNumber number) const
{
    if (!GetTransactionNumbers(localNymID, serverID, 1)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain transaction numbers.")
            .Flush();
    }

    return Action(new OTAPI_Func(
        KILL_PAYMENT_PLAN,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        accountID,
        number));
}

ServerAction::Action ServerAction::NotifyBailment(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetNymID,
    const Identifier& instrumentDefinitionID,
    const Identifier& requestID,
    const std::string& txid,
    const Amount amount) const
{
    return Action(new OTAPI_Func(
        NOTIFY_BAILMENT,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        targetNymID,
        requestID,
        instrumentDefinitionID,
        txid,
        amount));
}

ServerAction::Action ServerAction::PayDividend(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& instrumentDefinitionID,
    const Identifier& accountID,
    const std::string& memo,
    const Amount amountPerShare) const
{
    if (!GetTransactionNumbers(localNymID, serverID, 1)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain transaction numbers.")
            .Flush();
    }

    return Action(new OTAPI_Func(
        PAY_DIVIDEND,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        accountID,
        instrumentDefinitionID,
        amountPerShare,
        memo));
}

ServerAction::Action ServerAction::ProcessInbox(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    std::unique_ptr<Ledger>& ledger) const
{
    if (!GetTransactionNumbers(localNymID, serverID, 1)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain transaction numbers.")
            .Flush();
    }

    return Action(new OTAPI_Func(
        PROCESS_INBOX,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        accountID,
        ledger));
}

ServerAction::Action ServerAction::PublishNym(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetNymID) const
{
    return Action(new OTAPI_Func(
        REGISTER_CONTRACT_NYM,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        targetNymID));
}

ServerAction::Action ServerAction::PublishServerContract(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetServerID) const
{
    return Action(new OTAPI_Func(
        REGISTER_CONTRACT_SERVER,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        targetServerID));
}

ServerAction::Action ServerAction::PublishUnitDefinition(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& unitDefinitionID) const
{
    return Action(new OTAPI_Func(
        REGISTER_CONTRACT_UNIT,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        unitDefinitionID));
}

ServerAction::Action ServerAction::RegisterAccount(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& instrumentDefinitionID,
    const std::string& label) const
{
    return Action(new OTAPI_Func(
        CREATE_ASSET_ACCT,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        instrumentDefinitionID,
        label));
}

ServerAction::Action ServerAction::RegisterNym(
    const Identifier& localNymID,
    const Identifier& serverID) const
{
    return Action(new OTAPI_Func(
        REGISTER_NYM,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        false));
}

ServerAction::Action ServerAction::RequestAdmin(
    const Identifier& localNymID,
    const Identifier& serverID,
    const std::string& password) const
{
    return Action(new OTAPI_Func(
        REQUEST_ADMIN,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        password));
}

ServerAction::Action ServerAction::ResyncContext(
    const Identifier& localNymID,
    const Identifier& serverID) const
{
    return Action(new OTAPI_Func(
        REGISTER_NYM,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        true));
}

#if OT_CASH
ServerAction::Action ServerAction::SendCash(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& recipientNymID,
    std::shared_ptr<const Purse>& recipientCopy,
    std::shared_ptr<const Purse>& senderCopy) const
{
    auto strRecip = String::Factory();
    auto strSend = String::Factory();

    if (recipientCopy) strRecip = String::Factory(*recipientCopy);
    if (senderCopy) strSend = String::Factory(*senderCopy);

    std::unique_ptr<const Purse> pRecip(api_.Factory().Purse(strRecip));

    OT_ASSERT(false != bool(pRecip));

    std::unique_ptr<const Purse> pSend(api_.Factory().Purse(strSend));

    OT_ASSERT(false != bool(pSend));

    return Action(new OTAPI_Func(
        SEND_USER_INSTRUMENT,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        recipientNymID,
        pRecip,
        pSend));
}
#endif  // OT_CASH

ServerAction::Action ServerAction::SendMessage(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& recipientNymID,
    const std::string& message) const
{
    return Action(new OTAPI_Func(
        SEND_USER_MESSAGE,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        recipientNymID,
        message));
}

ServerAction::Action ServerAction::SendPayment(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& recipientNymID,
    std::shared_ptr<const OTPayment>& payment) const
{
    auto strPayment = String::Factory();

    if (false == bool(payment) || !payment->GetPaymentContents(strPayment)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Empty payment argument - "
                                           "should never happen!")
            .Flush();
        OT_FAIL;
    }

    auto pPayment{api_.Factory().Payment(strPayment)};
    std::unique_ptr<const OTPayment> cPayment{pPayment.release()};

    OT_ASSERT(false != bool(cPayment));

    return Action(new OTAPI_Func(
        SEND_USER_INSTRUMENT,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        recipientNymID,
        cPayment));
}

ServerAction::Action ServerAction::SendTransfer(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& senderAccountID,
    const Identifier& recipientAccountID,
    const Amount amount,
    const std::string& memo) const
{
    if (!GetTransactionNumbers(localNymID, serverID, 1)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain transaction numbers.")
            .Flush();
    }

    return Action(new OTAPI_Func(
        SEND_TRANSFER,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        senderAccountID,
        recipientAccountID,
        amount,
        memo));
}

ServerAction::Action ServerAction::TriggerClause(
    const Identifier& localNymID,
    const Identifier& serverID,
    const TransactionNumber transactionNumber,
    const std::string& clause,
    const std::string& parameter) const
{
    return Action(new OTAPI_Func(
        TRIGGER_CLAUSE,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        transactionNumber,
        clause,
        parameter));
}

ServerAction::Action ServerAction::UnregisterAccount(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID) const
{
    return Action(new OTAPI_Func(
        DELETE_ASSET_ACCT,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        accountID));
}

ServerAction::Action ServerAction::UnregisterNym(
    const Identifier& localNymID,
    const Identifier& serverID) const
{
    return Action(new OTAPI_Func(
        DELETE_NYM,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID));
}

#if OT_CASH
ServerAction::Action ServerAction::WithdrawCash(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const Amount amount) const
{
    if (!GetTransactionNumbers(localNymID, serverID, 1)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain transaction numbers.")
            .Flush();
    }

    return Action(new OTAPI_Func(
        WITHDRAW_CASH,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        accountID,
        amount));
}
#endif  // OT_CASH

ServerAction::Action ServerAction::WithdrawVoucher(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const Identifier& recipientNymID,
    const Amount amount,
    const std::string& memo) const
{
    if (!GetTransactionNumbers(localNymID, serverID, 1)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain transaction numbers.")
            .Flush();
    }

    return Action(new OTAPI_Func(
        WITHDRAW_VOUCHER,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID,
        accountID,
        recipientNymID,
        amount,
        memo));
}
}  // namespace opentxs::api::client::implementation
