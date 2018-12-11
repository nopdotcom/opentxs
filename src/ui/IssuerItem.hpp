// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
template <>
struct make_blank<IssuerItemRowID> {
    static IssuerItemRowID value()
    {
        return {Identifier::Factory(), proto::CITEMTYPE_ERROR};
    }
};

using IssuerItemList = List<
    IssuerItemExternalInterface,
    IssuerItemInternalInterface,
    IssuerItemRowID,
    IssuerItemRowInterface,
    IssuerItemRowInternal,
    IssuerItemRowBlank,
    IssuerItemSortKey>;
using IssuerItemRow = RowType<
    AccountSummaryRowInternal,
    AccountSummaryInternalInterface,
    AccountSummaryRowID>;

class IssuerItem final : public IssuerItemList, public IssuerItemRow
{
public:
    bool ConnectionState() const override { return connection_.load(); }
    std::string Debug() const override { return *issuer_; }
    std::string Name() const override;
    bool Trusted() const override { return issuer_->Paired(); }

    void reindex(const AccountSummarySortKey& key, const CustomData& custom)
        override;

    ~IssuerItem();

private:
    friend opentxs::Factory;

    const ListenerDefinitions listeners_;
    AccountSummarySortKey key_;
    const std::string& name_;
    std::atomic<bool> connection_{false};
    const std::shared_ptr<const api::client::Issuer> issuer_{nullptr};
    const proto::ContactItemType currency_;

    void construct_row(
        const IssuerItemRowID& id,
        const IssuerItemSortKey& index,
        const CustomData& custom) const override;

    void process_account(const Identifier& accountID);
    void process_account(const network::zeromq::Message& message);
    void refresh_accounts();
    void startup();

    IssuerItem(
        const AccountSummaryInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const AccountSummaryRowID& rowID,
        const AccountSummarySortKey& sortKey,
        const CustomData& custom,
        const proto::ContactItemType currency);
    IssuerItem() = delete;
    IssuerItem(const IssuerItem&) = delete;
    IssuerItem(IssuerItem&&) = delete;
    IssuerItem& operator=(const IssuerItem&) = delete;
    IssuerItem& operator=(IssuerItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
