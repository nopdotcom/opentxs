// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/peer/NoticeAcknowledgement.hpp"

#include "opentxs/core/Identifier.hpp"

#define CURRENT_VERSION 4

namespace opentxs
{
NoticeAcknowledgement::NoticeAcknowledgement(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const proto::PeerReply& serialized)
    : ot_super(wallet, nym, serialized)
    , ack_(serialized.notice().ack())
{
}

NoticeAcknowledgement::NoticeAcknowledgement(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const Identifier& initiator,
    const Identifier& request,
    const Identifier& server,
    const proto::PeerRequestType type,
    const bool& ack)
    : ot_super(wallet, nym, CURRENT_VERSION, initiator, server, type, request)
    , ack_(ack)
{
}

proto::PeerReply NoticeAcknowledgement::IDVersion(const Lock& lock) const
{
    auto contract = ot_super::IDVersion(lock);

    auto& notice = *contract.mutable_notice();
    notice.set_version(version_);
    notice.set_ack(ack_);

    return contract;
}
}  // namespace opentxs
