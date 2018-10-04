// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::implementation
{
class ManagedNumber : virtual public opentxs::ManagedNumber
{
public:
    void SetSuccess(const bool value = true) const override;
    bool Valid() const override;
    TransactionNumber Value() const override;

    ~ManagedNumber();

private:
    friend opentxs::Factory;

    ServerContext& context_;
    const TransactionNumber number_;
    mutable OTFlag success_;
    bool managed_{true};

    ManagedNumber(const TransactionNumber number, ServerContext& context);
    ManagedNumber() = delete;
    ManagedNumber(const ManagedNumber&) = delete;
    ManagedNumber(ManagedNumber&& rhs) = delete;
    ManagedNumber& operator=(const ManagedNumber&) = delete;
    ManagedNumber& operator=(ManagedNumber&&) = delete;
};
}  // namespace opentxs::implementation
