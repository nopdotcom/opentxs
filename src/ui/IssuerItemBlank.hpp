// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/ui/IssuerItem.hpp"
#include "opentxs/ui/Widget.hpp"

#include "InternalUI.hpp"

#include "AccountSummaryItemBlank.hpp"

namespace opentxs::ui::implementation
{
class IssuerItemBlank final : public AccountSummaryRowInternal
{
public:
    // IssuerItem
    bool ConnectionState() const override { return {}; }
    std::string Debug() const override { return {}; }
    OTUIAccountSummaryItem First() const override
    {
        return OTUIAccountSummaryItem{
            std::make_shared<AccountSummaryItemBlank>()};
    }
    std::string Name() const override { return {}; }
    OTUIAccountSummaryItem Next() const override
    {
        return OTUIAccountSummaryItem{
            std::make_shared<AccountSummaryItemBlank>()};
    }
    bool Trusted() const override { return {}; }

    void reindex(const AccountSummarySortKey&, const CustomData&) override {}

    bool last(const IssuerItemRowID&) const override { return false; }

    // ListRow
    bool Last() const override { return {}; }
    bool Valid() const override { return {}; }

    // Widget
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }

    IssuerItemBlank() = default;
    ~IssuerItemBlank() = default;

private:
    IssuerItemBlank(const IssuerItemBlank&) = delete;
    IssuerItemBlank(IssuerItemBlank&&) = delete;
    IssuerItemBlank& operator=(const IssuerItemBlank&) = delete;
    IssuerItemBlank& operator=(IssuerItemBlank&&) = delete;
};
}  // namespace opentxs::ui::implementation
