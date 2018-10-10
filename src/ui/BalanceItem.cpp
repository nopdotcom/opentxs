// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/Sync.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/ui/BalanceItem.hpp"

#include "InternalUI.hpp"
#include "Row.hpp"

#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include "BalanceItem.hpp"

template class opentxs::SharedPimpl<opentxs::ui::BalanceItem>;

#define OT_METHOD "opentxs::ui::implementation::BalanceItem::"

namespace opentxs
{
ui::implementation::AccountActivityRowInternal* Factory::BalanceItem(
    const ui::implementation::AccountActivityInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const ui::implementation::AccountActivityRowID& rowID,
    const ui::implementation::AccountActivitySortKey& sortKey,
    const ui::implementation::CustomData& custom,
    const Identifier& nymID,
    const Identifier& accountID)
{
    const auto type =
        ui::implementation::BalanceItem::recover_workflow(custom).type();

    switch (type) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE: {
            return new ui::implementation::ChequeBalanceItem(
                parent,
                api,
                publisher,
                rowID,
                sortKey,
                custom,
                nymID,
                accountID);
        }
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER: {
            return new ui::implementation::TransferBalanceItem(
                parent,
                api,
                publisher,
                rowID,
                sortKey,
                custom,
                nymID,
                accountID);
        }
        case proto::PAYMENTWORKFLOWTYPE_ERROR:
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unhandled workflow type (")(
                type)(")")
                .Flush();
        }
    }

    return nullptr;
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
BalanceItem::BalanceItem(
    const AccountActivityInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const AccountActivityRowID& rowID,
    const AccountActivitySortKey& sortKey,
    const CustomData& custom,
    const Identifier& nymID,
    const Identifier& accountID)
    : BalanceItemRow(parent, api, publisher, rowID, true)
    , nym_id_(Identifier::Factory(nymID))
    , workflow_(recover_workflow(custom).id())
    , type_(extract_type(recover_workflow(custom)))
    , text_("")
    , time_(sortKey)
    , contract_(nullptr)
    , startup_(nullptr)
    , account_id_(Identifier::Factory(accountID))
    , contacts_(extract_contacts(recover_workflow(custom)))
{
}

ChequeBalanceItem::ChequeBalanceItem(
    const AccountActivityInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const AccountActivityRowID& rowID,
    const AccountActivitySortKey& sortKey,
    const CustomData& custom,
    const Identifier& nymID,
    const Identifier& accountID)
    : BalanceItem(
          parent,
          api,
          publisher,
          rowID,
          sortKey,
          custom,
          nymID,
          accountID)
    , cheque_(nullptr)
{
    startup_.reset(new std::thread(&ChequeBalanceItem::startup, this, custom));

    OT_ASSERT(startup_)
}

TransferBalanceItem::TransferBalanceItem(
    const AccountActivityInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const AccountActivityRowID& rowID,
    const AccountActivitySortKey& sortKey,
    const CustomData& custom,
    const Identifier& nymID,
    const Identifier& accountID)
    : BalanceItem(
          parent,
          api,
          publisher,
          rowID,
          sortKey,
          custom,
          nymID,
          accountID)
{
    startup_.reset(
        new std::thread(&TransferBalanceItem::startup, this, custom));

    OT_ASSERT(startup_)
}

std::string BalanceItem::DisplayAmount() const
{
    sLock lock(shared_lock_);

    if (get_contract()) {
        const auto amount = effective_amount();
        std::string output{};
        const auto formatted =
            contract_->FormatAmountLocale(amount, output, ",", ".");

        if (formatted) { return output; }

        return std::to_string(amount);
    }

    return std::to_string(0);
}

std::vector<std::string> BalanceItem::extract_contacts(
    const proto::PaymentWorkflow& workflow)
{
    std::vector<std::string> output{};

    for (const auto& party : workflow.party()) { output.emplace_back(party); }

    return output;
}

StorageBox BalanceItem::extract_type(const proto::PaymentWorkflow& workflow)
{
    switch (workflow.type()) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE: {

            return StorageBox::OUTGOINGCHEQUE;
        }
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE: {

            return StorageBox::INCOMINGCHEQUE;
        }
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER: {

            return StorageBox::OUTGOINGTRANSFER;
        }
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER: {

            return StorageBox::INCOMINGTRANSFER;
        }
        case proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER: {

            return StorageBox::INTERNALTRANSFER;
        }
        case proto::PAYMENTWORKFLOWTYPE_ERROR:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE:
        default: {

            return StorageBox::UNKNOWN;
        }
    }
}

std::string BalanceItem::get_contact_name(const Identifier& nymID) const
{
    if (nymID.empty()) { return {}; }

    std::string output{nymID.str()};
    const auto contactID = api_.Contacts().ContactID(nymID);

    if (false == contactID->empty()) {
        output = api_.Contacts().ContactName(contactID);
    }

    return output;
}

const proto::PaymentEvent& BalanceItem::recover_event(const CustomData& custom)
{
    OT_ASSERT(2 == custom.size())

    const auto& input = custom.at(1);

    OT_ASSERT(nullptr != input)

    return *static_cast<const proto::PaymentEvent*>(input);
}

const proto::PaymentWorkflow& BalanceItem::recover_workflow(
    const CustomData& custom)
{
    OT_ASSERT(2 == custom.size())

    const auto& input = custom.at(0);

    OT_ASSERT(nullptr != input)

    return *static_cast<const proto::PaymentWorkflow*>(input);
}

void BalanceItem::reindex(
    const implementation::AccountActivitySortKey& key,
    const implementation::CustomData&)
{
    eLock lock(shared_lock_);
    time_ = key;
    lock.unlock();
}

std::string BalanceItem::Text() const
{
    sLock lock(shared_lock_);

    return text_;
}

std::chrono::system_clock::time_point BalanceItem::Timestamp() const
{
    sLock lock(shared_lock_);

    return time_;
}

BalanceItem::~BalanceItem()
{
    if (startup_ && startup_->joinable()) {
        startup_->join();
        startup_.reset();
    }
}

opentxs::Amount ChequeBalanceItem::effective_amount() const
{
    sLock lock(shared_lock_);
    auto amount{0};
    opentxs::Amount sign{0};

    if (cheque_) { amount = cheque_->GetAmount(); }

    switch (type_) {
        case StorageBox::OUTGOINGCHEQUE: {
            sign = -1;
        } break;
        case StorageBox::INCOMINGCHEQUE: {
            sign = 1;
        } break;
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::SENTPEERREPLY:
        case StorageBox::INCOMINGPEERREPLY:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::FINISHEDPEERREPLY:
        case StorageBox::PROCESSEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREPLY:
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX:
        case StorageBox::INCOMINGBLOCKCHAIN:
        case StorageBox::OUTGOINGBLOCKCHAIN:
        case StorageBox::DRAFT:
        case StorageBox::UNKNOWN:
        default: {
        }
    }

    return amount * sign;
}

bool ChequeBalanceItem::get_contract() const
{
    if (contract_) { return true; }

    eLock lock(shared_lock_);
    const auto& contractID = cheque_->GetInstrumentDefinitionID();
    contract_ = api_.Wallet().UnitDefinition(contractID);

    if (contract_) { return true; }

    api_.Sync().ScheduleDownloadContract(
        nym_id_, api_.Sync().IntroductionServer(), contractID);

    return false;
}

std::string ChequeBalanceItem::Memo() const
{
    sLock lock(shared_lock_);

    if (cheque_) { return cheque_->GetMemo().Get(); }

    return {};
}

void ChequeBalanceItem::reindex(
    const implementation::AccountActivitySortKey& key,
    const implementation::CustomData& custom)
{
    BalanceItem::reindex(key, custom);
    startup(custom);
}

void ChequeBalanceItem::startup(const CustomData& custom)
{
    OT_ASSERT(2 == custom.size())

    const auto workflow = extract_custom<proto::PaymentWorkflow>(custom, 0);
    const auto event = extract_custom<proto::PaymentEvent>(custom, 1);
    eLock lock(shared_lock_);
    cheque_ = api::client::Workflow::InstantiateCheque(api_, workflow).second;

    OT_ASSERT(cheque_)

    lock.unlock();
    get_contract();
    std::string name{""};
    std::string text{""};
    auto number = std::to_string(cheque_->GetTransactionNum());
    auto otherNymID = Identifier::Factory();

    switch (type_) {
        case StorageBox::INCOMINGCHEQUE: {
            otherNymID = Identifier::Factory(cheque_->GetSenderNymID());

            if (otherNymID->empty()) { otherNymID = nym_id_; }

            switch (event.type()) {
                case proto::PAYMENTEVENTTYPE_CONVEY: {
                    text = "Received cheque #" + number + " from " +
                           get_contact_name(otherNymID);
                } break;
                case proto::PAYMENTEVENTTYPE_ERROR:
                case proto::PAYMENTEVENTTYPE_CREATE:
                case proto::PAYMENTEVENTTYPE_ACCEPT:
                case proto::PAYMENTEVENTTYPE_CANCEL:
                case proto::PAYMENTEVENTTYPE_COMPLETE:
                default: {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Invalid event state (")(event.type())(")")
                        .Flush();
                }
            }
        } break;
        case StorageBox::OUTGOINGCHEQUE: {
            otherNymID = Identifier::Factory(cheque_->GetRecipientNymID());

            switch (event.type()) {
                case proto::PAYMENTEVENTTYPE_CREATE: {
                    text = "Wrote cheque #" + number;

                    if (false == otherNymID->empty()) {
                        text += " for " + get_contact_name(otherNymID);
                    }
                } break;
                case proto::PAYMENTEVENTTYPE_ACCEPT: {
                    text = "Cheque #" + number + " cleared";
                } break;
                case proto::PAYMENTEVENTTYPE_ERROR:
                case proto::PAYMENTEVENTTYPE_CONVEY:
                case proto::PAYMENTEVENTTYPE_CANCEL:
                case proto::PAYMENTEVENTTYPE_COMPLETE:
                default: {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Invalid event state (")(event.type())(")")
                        .Flush();
                }
            }
        } break;
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::SENTPEERREPLY:
        case StorageBox::INCOMINGPEERREPLY:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::FINISHEDPEERREPLY:
        case StorageBox::PROCESSEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREPLY:
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX:
        case StorageBox::INCOMINGBLOCKCHAIN:
        case StorageBox::OUTGOINGBLOCKCHAIN:
        case StorageBox::OUTGOINGTRANSFER:
        case StorageBox::INCOMINGTRANSFER:
        case StorageBox::INTERNALTRANSFER:
        case StorageBox::DRAFT:
        case StorageBox::UNKNOWN:
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid item type (")(
                static_cast<std::uint8_t>(type_))(")")
                .Flush();
        }
    }

    lock.lock();
    text_ = text;
    lock.unlock();
    UpdateNotify();
}

opentxs::Amount TransferBalanceItem::effective_amount() const
{
    sLock lock(shared_lock_);
    auto amount{0};
    opentxs::Amount sign{0};

    if (transfer_) { amount = transfer_->GetAmount(); }

    switch (type_) {
        case StorageBox::OUTGOINGTRANSFER: {
            sign = -1;
        } break;
        case StorageBox::INCOMINGTRANSFER: {
            sign = 1;
        } break;
        case StorageBox::INTERNALTRANSFER: {
            const auto in =
                parent_.AccountID() == transfer_->GetDestinationAcctID();

            if (in) {
                sign = 1;
            } else {
                sign = -1;
            }
        } break;
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::SENTPEERREPLY:
        case StorageBox::INCOMINGPEERREPLY:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::FINISHEDPEERREPLY:
        case StorageBox::PROCESSEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREPLY:
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX:
        case StorageBox::INCOMINGBLOCKCHAIN:
        case StorageBox::OUTGOINGBLOCKCHAIN:
        case StorageBox::INCOMINGCHEQUE:
        case StorageBox::OUTGOINGCHEQUE:
        case StorageBox::DRAFT:
        case StorageBox::UNKNOWN:
        default: {
        }
    }

    return amount * sign;
}

bool TransferBalanceItem::get_contract() const
{
    if (contract_) { return true; }

    auto contractID = Identifier::Factory();
    const auto in = parent_.AccountID() == transfer_->GetDestinationAcctID();

    if (in) {
        contractID =
            api_.Storage().AccountContract(transfer_->GetDestinationAcctID());
    } else {
        contractID =
            api_.Storage().AccountContract(transfer_->GetPurportedAccountID());
    }

    eLock lock(shared_lock_);
    contract_ = api_.Wallet().UnitDefinition(contractID);
    lock.unlock();

    if (contract_) { return true; }

    api_.Sync().ScheduleDownloadContract(
        nym_id_, api_.Sync().IntroductionServer(), contractID);

    return false;
}

std::string TransferBalanceItem::Memo() const
{
    sLock lock(shared_lock_);

    if (transfer_) {
        auto note = String::Factory();
        transfer_->GetNote(note);

        return note->Get();
    }

    return {};
}

void TransferBalanceItem::reindex(
    const implementation::AccountActivitySortKey& key,
    const implementation::CustomData& custom)
{
    BalanceItem::reindex(key, custom);
    startup(custom);
}

void TransferBalanceItem::startup(const CustomData& custom)
{
    OT_ASSERT(2 == custom.size())

    const auto workflow = extract_custom<proto::PaymentWorkflow>(custom, 0);
    const auto event = extract_custom<proto::PaymentEvent>(custom, 1);
    eLock lock(shared_lock_);
    transfer_ =
        api::client::Workflow::InstantiateTransfer(api_, workflow).second;

    OT_ASSERT(transfer_)

    lock.unlock();
    get_contract();
    std::string text{""};
    const auto number = std::to_string(transfer_->GetTransactionNum());

    switch (type_) {
        case StorageBox::OUTGOINGTRANSFER: {
            switch (event.type()) {
                case proto::PAYMENTEVENTTYPE_ACKNOWLEDGE: {
                    text = "Sent transfer #" + number + " to ";

                    if (0 < workflow.party_size()) {
                        text += get_contact_name(
                            Identifier::Factory(workflow.party(0)));
                    } else {
                        text += "account " +
                                transfer_->GetDestinationAcctID().str();
                    }
                } break;
                case proto::PAYMENTEVENTTYPE_COMPLETE: {
                    text = "Transfer #" + number + " cleared.";
                } break;
                default: {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Invalid event state (")(event.type())(")")
                        .Flush();
                }
            }
        } break;
        case StorageBox::INCOMINGTRANSFER: {
            switch (event.type()) {
                case proto::PAYMENTEVENTTYPE_CONVEY: {
                    text = "Received transfer #" + number + " from ";

                    if (0 < workflow.party_size()) {
                        text += get_contact_name(
                            Identifier::Factory(workflow.party(0)));
                    } else {
                        text += "account " +
                                transfer_->GetPurportedAccountID().str();
                    }
                } break;
                case proto::PAYMENTEVENTTYPE_COMPLETE: {
                    text = "Transfer #" + number + " cleared.";
                } break;
                default: {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Invalid event state (")(event.type())(")")
                        .Flush();
                }
            }
        } break;
        case StorageBox::INTERNALTRANSFER: {
            const auto in =
                parent_.AccountID() == transfer_->GetDestinationAcctID();

            switch (event.type()) {
                case proto::PAYMENTEVENTTYPE_ACKNOWLEDGE: {
                    if (in) {
                        text = "Received internal transfer #" + number +
                               " from account " +
                               transfer_->GetPurportedAccountID().str();
                    } else {
                        text = "Sent internal transfer #" + number +
                               " to account " +
                               transfer_->GetDestinationAcctID().str();
                    }
                } break;
                case proto::PAYMENTEVENTTYPE_COMPLETE: {
                    text = "Transfer #" + number + " cleared.";
                } break;
                default: {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Invalid event state (")(event.type())(")")
                        .Flush();
                }
            }
        } break;
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::SENTPEERREPLY:
        case StorageBox::INCOMINGPEERREPLY:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::FINISHEDPEERREPLY:
        case StorageBox::PROCESSEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREPLY:
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX:
        case StorageBox::INCOMINGBLOCKCHAIN:
        case StorageBox::OUTGOINGBLOCKCHAIN:
        case StorageBox::INCOMINGCHEQUE:
        case StorageBox::OUTGOINGCHEQUE:
        case StorageBox::DRAFT:
        case StorageBox::UNKNOWN:
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid item type (")(
                static_cast<std::uint8_t>(type_))(")")
                .Flush();
        }
    }

    lock.lock();
    text_ = text;
    lock.unlock();
    UpdateNotify();
}
}  // namespace opentxs::ui::implementation
