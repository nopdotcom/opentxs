// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_ACCOUNTLIST_HPP
#define OPENTXS_CORE_ACCOUNTLIST_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Account.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/String.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <string>

namespace opentxs
{
/** The server needs to store a list of accounts, by instrument definition ID,
 * to store the backing funds for vouchers. The below class is useful for that.
 * It's also useful for the same purpose for stashes, in smart contracts.
 * Eventually will add expiration dates, possibly, to this class. (To have
 * series, just like cash already does now.) */
class AccountList
{
public:
    std::int32_t GetCountAccountIDs() const
    {
        return static_cast<std::int32_t>(mapAcctIDs_.size());
    }
    void Release();
    void Release_AcctList();
    void Serialize(Tag& parent) const;
    std::int32_t ReadFromXMLNode(
        irr::io::IrrXMLReader*& xml,
        const String& acctType,
        const String& acctCount);
    void SetType(Account::AccountType acctType) { acctType_ = acctType; }
    ExclusiveAccount GetOrRegisterAccount(
        const Nym& serverNym,
        const Identifier& ACCOUNT_OWNER_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& NOTARY_ID,
        bool& wasAcctCreated,  // this will be set to true if the acct is
                               // created here. Otherwise set to false;
        std::int64_t stashTransNum = 0);

    explicit AccountList(const api::Core& core);
    explicit AccountList(const api::Core& core, Account::AccountType acctType);

    ~AccountList();

private:
    typedef std::map<std::string, std::weak_ptr<Account>> MapOfWeakAccounts;

    const api::Core& api_;
    Account::AccountType acctType_;

    /** AcctIDs as second mapped by ASSET TYPE ID as first. */
    String::Map mapAcctIDs_;

    AccountList() = delete;
};
}  // namespace opentxs
#endif
