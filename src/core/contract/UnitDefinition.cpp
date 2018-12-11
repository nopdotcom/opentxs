// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/UnitDefinition.hpp"

#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/contract/CurrencyContract.hpp"
#include "opentxs/core/contract/SecurityContract.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/basket/BasketContract.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/AccountVisitor.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/Proto.hpp"

#include <ctype.h>
#include <stddef.h>
#include <cmath>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#define OT_METHOD "opentxs::UnitDefinition::"

namespace opentxs
{
UnitDefinition::UnitDefinition(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const std::string& shortname,
    const std::string& name,
    const std::string& symbol,
    const std::string& terms)
    : ot_super(nym)
    , primary_unit_name_(name)
    , short_name_(shortname)
    , wallet_{wallet}
    , primary_unit_symbol_(symbol)
{
    version_ = 1;
    conditions_ = terms;
}

UnitDefinition::UnitDefinition(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const proto::UnitDefinition serialized)
    : ot_super(nym)
    , wallet_{wallet}
{
    if (serialized.has_id()) { id_ = Identifier::Factory(serialized.id()); }
    if (serialized.has_signature()) {
        signatures_.push_front(SerializedSignature(
            std::make_shared<proto::Signature>(serialized.signature())));
    }
    if (serialized.has_version()) { version_ = serialized.version(); }
    if (serialized.has_terms()) { conditions_ = serialized.terms(); }
    if (serialized.has_name()) { primary_unit_name_ = serialized.name(); }
    if (serialized.has_symbol()) { primary_unit_symbol_ = serialized.symbol(); }
    if (serialized.has_shortname()) { short_name_ = serialized.shortname(); }
}

bool UnitDefinition::ParseFormatted(
    std::int64_t& lResult,
    const std::string& str_input,
    std::int32_t nFactor,
    std::int32_t nPower,
    const char* szThousandSeparator,
    const char* szDecimalPoint)
{
    OT_ASSERT(nullptr != szThousandSeparator);
    OT_ASSERT(nullptr != szDecimalPoint);

    lResult = 0;

    char theSeparator = szThousandSeparator[0];
    char theDecimalPoint = szDecimalPoint[0];

    std::int64_t lDollars = 0;
    std::int64_t lCents = 0;
    std::int64_t lOutput = 0;
    std::int64_t lSign = 1;

    bool bHasEnteredDollars = false;
    bool bHasEnteredCents = false;

    std::int32_t nDigitsCollectedBeforeDot = 0;
    std::int32_t nDigitsCollectedAfterDot = 0;

    // BUG: &mp isn't used.
    // const std::moneypunct<char, false> &mp = std::use_facet<
    // std::moneypunct<char, false> >(std::locale ());

    std::deque<std::int64_t> deque_cents;

    for (std::uint32_t uIndex = 0; uIndex < str_input.length(); ++uIndex) {
        char theChar = str_input[uIndex];

        if (iscntrl(theChar))  // Break at any newline or other control
                               // character.
            break;

        if (0 == isdigit(theChar))  // if it's not a numerical digit.
        {
            if (theSeparator == theChar) continue;

            if (theDecimalPoint == theChar) {
                if (bHasEnteredCents) {
                    // There shouldn't be ANOTHER decimal point if we are
                    // already in the cents.
                    // Therefore, we're done here. Break.
                    //
                    break;
                }

                // If we HAVEN'T entered the cents yet, then this decimal point
                // marks the spot where we DO.
                //
                bHasEnteredDollars = true;
                bHasEnteredCents = true;
                continue;
            }  // theChar is the decimal point

            // Once a negative sign appears, it's negative, period.
            // If you put two or three negative signs in a row, it's STILL
            // negative.

            if ('-' == theChar) {
                lSign = -1;
                continue;
            }

            // Okay, by this point, we know it's not numerical, and it's not a
            // separator or decimal point, or sign. We allow letters and symbols
            // BEFORE the numbers start, but not AFTER (that would terminate the
            // number.) Therefore we need to see if the dollars or cents have
            // started yet. If they have, then this is the end, and we break.
            // Otherwise if they haven't, then we're still at the beginning, so
            // we continue.
            if (bHasEnteredDollars || bHasEnteredCents)
                break;
            else
                continue;
        }  // not numerical

        // By this point, we KNOW that it's a numeric digit. Are we collecting
        // cents yet? How about dollars? Also, if nPower is 2, then we only
        // collect 2 digits after the decimal point. If we've already collected
        // those, then we need to break.
        if (bHasEnteredCents) {
            ++nDigitsCollectedAfterDot;

            // If "cents" occupy 2 digits after the decimal point, and we are
            // now on the THIRD digit -- then we're done.
            if (nDigitsCollectedAfterDot > nPower) break;

            // Okay, we're in the cents, so let's add this digit...
            deque_cents.push_back(static_cast<std::int64_t>(theChar - '0'));

            continue;
        }

        // Okay, it's a digit, and we haven't started processing cents yet. How
        // about dollars?
        if (!bHasEnteredDollars) bHasEnteredDollars = true;

        ++nDigitsCollectedBeforeDot;

        // Let's add this digit...
        lDollars *=
            10;  // Multiply existing dollars by 10, and then add the new digit.
        lDollars += static_cast<std::int64_t>(theChar - '0');
    }

    // Time to put it all together...
    lOutput += lDollars;
    lOutput *=
        static_cast<std::int64_t>(nFactor);  // 1 dollar becomes 100 cents.

    std::int32_t nTempPower = nPower;

    while (nTempPower > 0) {
        --nTempPower;

        if (deque_cents.size() > 0) {
            lCents += deque_cents.front();
            deque_cents.pop_front();
        }

        lCents *= 10;
    }
    lCents /= 10;  // There won't be any rounding errors here, since the last
                   // thing we did in the loop was multiply by 10.

    lOutput += lCents;

    lResult = (lOutput * lSign);

    return true;
}

inline void separateThousands(
    std::stringstream& sss,
    std::int64_t value,
    const char* szSeparator)
{
    if (value < 1000) {
        sss << value;
        return;
    }

    separateThousands(sss, value / 1000, szSeparator);
    sss << szSeparator << std::setfill('0') << std::setw(3) << value % 1000;
}

std::string UnitDefinition::formatLongAmount(
    std::int64_t lValue,
    std::int32_t nFactor,
    std::int32_t nPower,
    const char* szCurrencySymbol,
    const char* szThousandSeparator,
    const char* szDecimalPoint)
{
    std::stringstream sss;

    // Handle negative values
    if (lValue < 0) {
        sss << "-";
        lValue = -lValue;
    }

    if (NULL != szCurrencySymbol) sss << szCurrencySymbol << " ";

    // For example, if 506 is supposed to be $5.06, then dividing by a factor of
    // 100 results in 5 dollars (integer value) and 6 cents (fractional value).

    // Handle integer value with thousand separaters
    separateThousands(sss, lValue / nFactor, szThousandSeparator);

    // Handle fractional value
    if (1 < nFactor) {
        sss << szDecimalPoint << std::setfill('0') << std::setw(nPower)
            << (lValue % nFactor);
    }

    std::string str_result(sss.str());

    return str_result;
}

bool UnitDefinition::DisplayStatistics(String& strContents) const
{
    std::string type = "error";

    switch (Type()) {
        case proto::UNITTYPE_CURRENCY:
            type = "error";

            break;
        case proto::UNITTYPE_SECURITY:
            type = "security";

            break;
        case proto::UNITTYPE_BASKET:
            type = "basket currency";

            break;
        default:
            break;
    }

    strContents.Concatenate(
        " Asset Type:  %s\n"
        " InstrumentDefinitionID: %s\n"
        "\n",
        type.c_str(),
        id_->str().c_str());
    return true;
}

// currently only "user" accounts (normal user asset accounts) are added to
// this list Any "special" accounts, such as basket reserve accounts, or voucher
// reserve accounts, or cash reserve accounts, are not included on this list.
bool UnitDefinition::VisitAccountRecords(
    const std::string& dataFolder,
    AccountVisitor& visitor) const
{
    Lock lock(lock_);

    const auto strInstrumentDefinitionID = String::Factory(id(lock));
    auto strAcctRecordFile = String::Factory();
    strAcctRecordFile->Format("%s.a", strInstrumentDefinitionID->Get());

    std::unique_ptr<OTDB::Storable> pStorable(OTDB::QueryObject(
        OTDB::STORED_OBJ_STRING_MAP,
        dataFolder,
        OTFolders::Contract().Get(),
        strAcctRecordFile->Get(),
        "",
        ""));

    OTDB::StringMap* pMap = dynamic_cast<OTDB::StringMap*>(pStorable.get());

    // There was definitely a StringMap loaded from local storage.
    // (Even an empty one, possibly.) This is the only block that matters in
    // this function.
    //
    if (nullptr != pMap) {
        const auto pNotaryID = visitor.GetNotaryID();
        OT_ASSERT(false == pNotaryID->empty());

        auto& theMap = pMap->the_map;

        // todo: optimize: will probably have to use a database for this,
        // std::int64_t term.
        // (What if there are a million acct IDs in this flat file? Not
        // scaleable.)
        //
        for (auto& it : theMap) {
            const std::string& str_acct_id =
                it.first;  // Containing the account ID.
            const std::string& str_instrument_definition_id =
                it.second;  // Containing the instrument definition ID. (Just in
                            // case
                            // someone copied the wrong file here...)

            if (!strInstrumentDefinitionID->Compare(
                    str_instrument_definition_id.c_str())) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error: wrong "
                    "instrument definition ID (")(str_instrument_definition_id)(
                    ") when expecting: ")(strInstrumentDefinitionID)(".")
                    .Flush();
            } else {
                const auto& wallet = wallet_;
                const auto accountID = Identifier::Factory(str_acct_id);
                auto account = wallet.Account(accountID);

                if (false == bool(account)) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Unable to load account ")(str_acct_id)(".")
                        .Flush();

                    continue;
                }

                if (false == visitor.Trigger(account.get())) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Error: Trigger failed for account ")(str_acct_id)(
                        ".")
                        .Flush();
                }
            }
        }
        return true;
    }
    return true;
}

bool UnitDefinition::AddAccountRecord(
    const std::string& dataFolder,
    const Account& theAccount) const  // adds
                                      // the
// account
// to the
// list.
// (When
// account
// is
// created.)
{
    //  Load up account list StringMap. Create it if doesn't already exist.
    //  See if account is already there in the map. Add it otherwise.
    //  Save the StringMap back again. (The account records list for a given
    // instrument definition.)

    Lock lock(lock_);

    if (theAccount.GetInstrumentDefinitionID() != id_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: theAccount doesn't have the same asset "
            "type ID as *this does.")
            .Flush();
        return false;
    }

    const auto theAcctID = Identifier::Factory(theAccount);
    const auto strAcctID = String::Factory(theAcctID);

    const auto strInstrumentDefinitionID = String::Factory(id(lock));
    auto strAcctRecordFile = String::Factory();
    strAcctRecordFile->Format("%s.a", strInstrumentDefinitionID->Get());

    OTDB::Storable* pStorable = nullptr;
    std::unique_ptr<OTDB::Storable> theAngel;
    OTDB::StringMap* pMap = nullptr;

    if (OTDB::Exists(
            dataFolder,
            OTFolders::Contract().Get(),
            strAcctRecordFile->Get(),
            "",
            ""))  // the file already exists; let's
                  // try to load it up.
        pStorable = OTDB::QueryObject(
            OTDB::STORED_OBJ_STRING_MAP,
            dataFolder,
            OTFolders::Contract().Get(),
            strAcctRecordFile->Get(),
            "",
            "");
    else  // the account records file (for this instrument definition) doesn't
          // exist.
        pStorable = OTDB::CreateObject(
            OTDB::STORED_OBJ_STRING_MAP);  // this asserts already, on failure.

    theAngel.reset(pStorable);
    pMap = (nullptr == pStorable) ? nullptr
                                  : dynamic_cast<OTDB::StringMap*>(pStorable);

    // It exists.
    //
    if (nullptr == pMap) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Failed trying to load or create the account records "
            "file for instrument definition: ")(strInstrumentDefinitionID)(".")
            .Flush();
        return false;
    }

    auto& theMap = pMap->the_map;
    auto map_it = theMap.find(strAcctID->Get());

    if (theMap.end() != map_it)  // we found it.
    {                            // We were ADDING IT, but it was ALREADY THERE.
        // (Thus, we're ALREADY DONE.)
        // Let's just make sure the right instrument definition ID is associated
        // with this
        // account
        // (it better be, since we loaded the account records file based on the
        // instrument definition ID as its filename...)
        //
        const std::string& str2 = map_it->second;  // Containing the instrument
                                                   // definition ID. (Just in
                                                   // case
        // someone copied the wrong file here,
        // --------------------------------          // every account should map
        // to the SAME instrument definition id.)

        if (false ==
            strInstrumentDefinitionID->Compare(str2.c_str()))  // should
                                                               // never
        // happen.
        {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error: wrong instrument definition found in "
                "account records "
                "file. For instrument definition: ")(strInstrumentDefinitionID)(
                ". For account: ")(strAcctID)(
                ". Found wrong instrument definition: ")(str2)(".")
                .Flush();
            return false;
        }

        return true;  // already there (no need to add.) + the instrument
                      // definition ID
                      // matches.
    }

    // it wasn't already on the list...

    // ...so add it.
    //
    theMap[strAcctID->Get()] = strInstrumentDefinitionID->Get();

    // Then save it back to local storage:
    //
    if (!OTDB::StoreObject(
            *pMap,
            dataFolder,
            OTFolders::Contract().Get(),
            strAcctRecordFile->Get(),
            "",
            "")) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to StoreObject, while saving updated "
            "account records file for instrument definition: ")(
            strInstrumentDefinitionID)(" to contain account ID: ")(strAcctID)(
            ".")
            .Flush();
        return false;
    }

    // Okay, we saved the updated file, with the account added. (done, success.)
    //
    return true;
}

bool UnitDefinition::EraseAccountRecord(
    const std::string& dataFolder,
    const Identifier& theAcctID) const  // removes the account from the list.
                                        // (When account is deleted.)
{
    //  Load up account list StringMap. Create it if doesn't already exist.
    //  See if account is already there in the map. Erase it, if it is.
    //  Save the StringMap back again. (The account records list for a given
    // instrument definition.)

    Lock lock(lock_);

    const auto strAcctID = String::Factory(theAcctID);

    const auto strInstrumentDefinitionID = String::Factory(id(lock));
    auto strAcctRecordFile = String::Factory();
    strAcctRecordFile->Format("%s.a", strInstrumentDefinitionID->Get());

    OTDB::Storable* pStorable = nullptr;
    std::unique_ptr<OTDB::Storable> theAngel;
    OTDB::StringMap* pMap = nullptr;

    if (OTDB::Exists(
            dataFolder,
            OTFolders::Contract().Get(),
            strAcctRecordFile->Get(),
            "",
            ""))  // the file already exists; let's
                  // try to load it up.
        pStorable = OTDB::QueryObject(
            OTDB::STORED_OBJ_STRING_MAP,
            dataFolder,
            OTFolders::Contract().Get(),
            strAcctRecordFile->Get(),
            "",
            "");
    else  // the account records file (for this instrument definition) doesn't
          // exist.
        pStorable = OTDB::CreateObject(
            OTDB::STORED_OBJ_STRING_MAP);  // this asserts already, on failure.

    theAngel.reset(pStorable);
    pMap = (nullptr == pStorable) ? nullptr
                                  : dynamic_cast<OTDB::StringMap*>(pStorable);

    // It exists.
    //
    if (nullptr == pMap) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Failed trying to load or create the account records "
            "file for instrument definition: ")(strInstrumentDefinitionID)(".")
            .Flush();
        return false;
    }

    // Before we can erase it, let's see if it's even there....
    //
    auto& theMap = pMap->the_map;
    auto map_it = theMap.find(strAcctID->Get());

    // we found it!
    if (theMap.end() != map_it)  //  Acct ID was already there...
    {
        theMap.erase(map_it);  // remove it
    }

    // it wasn't already on the list...
    // (So it's like success, since the end result is, acct ID will not appear
    // on this list--whether
    // it was there or not beforehand, it's definitely not there now.)

    // Then save it back to local storage:
    //
    if (!OTDB::StoreObject(
            *pMap,
            dataFolder,
            OTFolders::Contract().Get(),
            strAcctRecordFile->Get(),
            "",
            "")) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to StoreObject, while saving updated "
            "account records file for instrument definition: ")(
            strInstrumentDefinitionID)(" to erase account ID: ")(strAcctID)(".")
            .Flush();
        return false;
    }

    // Okay, we saved the updated file, with the account removed. (done,
    // success.)
    //
    return true;
}

UnitDefinition* UnitDefinition::Create(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const std::string& shortname,
    const std::string& name,
    const std::string& symbol,
    const std::string& terms,
    const std::string& tla,
    const std::uint32_t& power,
    const std::string& fraction)
{
    std::unique_ptr<UnitDefinition> contract(new CurrencyContract(
        wallet, nym, shortname, name, symbol, terms, tla, power, fraction));

    if (!contract) { return nullptr; }

    Lock lock(contract->lock_);

    if (!contract->CalculateID(lock)) { return nullptr; }

    if (contract->nym_) {
        auto serialized = contract->SigVersion(lock);
        std::shared_ptr<proto::Signature> sig =
            std::make_shared<proto::Signature>();

        if (!contract->update_signature(lock)) { return nullptr; }
    }

    if (!contract->validate(lock)) { return nullptr; }

    contract->alias_ = contract->short_name_;

    return contract.release();
}

UnitDefinition* UnitDefinition::Create(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const std::string& shortname,
    const std::string& name,
    const std::string& symbol,
    const std::string& terms)
{
    std::unique_ptr<UnitDefinition> contract(
        new SecurityContract(wallet, nym, shortname, name, symbol, terms));

    if (!contract) { return nullptr; }

    Lock lock(contract->lock_);

    if (!contract->CalculateID(lock)) { return nullptr; }

    if (contract->nym_) {
        proto::UnitDefinition serialized = contract->SigVersion(lock);
        std::shared_ptr<proto::Signature> sig =
            std::make_shared<proto::Signature>();

        if (!contract->update_signature(lock)) { return nullptr; }
    }

    if (!contract->validate(lock)) { return nullptr; }

    contract->alias_ = contract->short_name_;

    return contract.release();
}

// Unlike the other Create functions, this one does not produce a complete,
// valid contract. This is used on the client side to produce a template for
// the server, which actually creates the contract.
UnitDefinition* UnitDefinition::Create(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const std::string& shortname,
    const std::string& name,
    const std::string& symbol,
    const std::string& terms,
    const std::uint64_t weight)
{
    std::unique_ptr<UnitDefinition> contract(new BasketContract(
        wallet, nym, shortname, name, symbol, terms, weight));

    return contract.release();
}

UnitDefinition* UnitDefinition::Factory(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const proto::UnitDefinition& serialized)
{
    if (!proto::Validate<proto::UnitDefinition>(serialized, VERBOSE, true)) {

        return nullptr;
    }

    std::unique_ptr<UnitDefinition> contract;

    switch (serialized.type()) {
        case proto::UNITTYPE_CURRENCY:
            contract.reset(new CurrencyContract(wallet, nym, serialized));

            break;
        case proto::UNITTYPE_BASKET:
            contract.reset(new BasketContract(wallet, nym, serialized));

            break;
        case proto::UNITTYPE_SECURITY:
            contract.reset(new SecurityContract(wallet, nym, serialized));

            break;
        default:

            return nullptr;
    }

    if (!contract) { return nullptr; }

    Lock lock(contract->lock_);

    if (!contract->validate(lock)) { return nullptr; }

    contract->alias_ = contract->short_name_;

    return contract.release();
}

proto::UnitDefinition UnitDefinition::IDVersion(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    proto::UnitDefinition contract;
    contract.set_version(version_);
    contract.clear_id();         // reinforcing that this field must be blank.
    contract.clear_signature();  // reinforcing that this field must be blank.
    contract.clear_publicnym();  // reinforcing that this field must be blank.

    if (nym_) {
        auto nymID = String::Factory();
        nym_->GetIdentifier(nymID);
        contract.set_nymid(nymID->Get());
    }

    contract.set_shortname(short_name_);
    contract.set_terms(conditions_);
    contract.set_name(primary_unit_name_);
    contract.set_symbol(primary_unit_symbol_);
    contract.set_type(Type());

    return contract;
}

proto::UnitDefinition UnitDefinition::SigVersion(const Lock& lock) const
{
    auto contract = IDVersion(lock);
    contract.set_id(id(lock)->str().c_str());

    return contract;
}

proto::UnitDefinition UnitDefinition::contract(const Lock& lock) const
{
    auto contract = SigVersion(lock);

    if (1 <= signatures_.size()) {
        *(contract.mutable_signature()) = *(signatures_.front());
    }

    return contract;
}

proto::UnitDefinition UnitDefinition::Contract() const
{
    Lock lock(lock_);

    return contract(lock);
}

OTData UnitDefinition::Serialize() const
{
    Lock lock(lock_);

    return proto::ProtoAsData(contract(lock));
}

OTIdentifier UnitDefinition::GetID(const Lock& lock) const
{
    return GetID(IDVersion(lock));
}

OTIdentifier UnitDefinition::GetID(const proto::UnitDefinition& contract)
{
    auto id = Identifier::Factory();
    id->CalculateDigest(proto::ProtoAsData<proto::UnitDefinition>(contract));
    return id;
}

void UnitDefinition::SetAlias(const std::string& alias)
{
    ot_super::SetAlias(alias);

    wallet_.SetUnitDefinitionAlias(id_, alias);
}

bool UnitDefinition::update_signature(const Lock& lock)
{
    if (!ot_super::update_signature(lock)) { return false; }

    bool success = false;
    signatures_.clear();
    auto serialized = SigVersion(lock);
    auto& signature = *serialized.mutable_signature();
    signature.set_role(proto::SIGROLE_UNITDEFINITION);
    success = nym_->SignProto(serialized, signature);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create signature.")
            .Flush();
    }

    return success;
}

bool UnitDefinition::validate(const Lock& lock) const
{
    bool validNym = false;

    if (nym_) { validNym = nym_->VerifyPseudonym(); }

    const bool validSyntax = proto::Validate(contract(lock), VERBOSE, true);

    if (1 > signatures_.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing signature.").Flush();

        return false;
    }

    bool validSig = false;
    auto& signature = *signatures_.cbegin();

    if (signature) { validSig = verify_signature(lock, *signature); }

    return (validNym && validSyntax && validSig);
}

bool UnitDefinition::verify_signature(
    const Lock& lock,
    const proto::Signature& signature) const
{
    if (!ot_super::verify_signature(lock, signature)) { return false; }

    auto serialized = SigVersion(lock);
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return nym_->VerifyProto(serialized, sigProto);
    ;
}

proto::UnitDefinition UnitDefinition::PublicContract() const
{
    Lock lock(lock_);

    auto serialized = contract(lock);

    if (nym_) {
        auto publicNym = nym_->asPublicNym();
        *(serialized.mutable_publicnym()) = publicNym;
    }

    return serialized;
}

// Convert 912545 to "$9,125.45"
//
// (Assuming a Factor of 100, Decimal Power of 2, Currency Symbol of "$",
//  separator of "," and decimal point of ".")
//
bool UnitDefinition::FormatAmountLocale(
    std::int64_t amount,
    std::string& str_output,
    const std::string& str_thousand,
    const std::string& str_decimal) const
{
    // Lookup separator and decimal point symbols based on locale.

    // Get a moneypunct facet from the global ("C") locale
    //
    // NOTE: Turns out moneypunct kind of sucks.
    // As a result, for internationalization purposes,
    // these values have to be set here before compilation.
    //
    auto strSeparator =
        String::Factory(str_thousand.empty() ? OT_THOUSANDS_SEP : str_thousand);
    auto strDecimalPoint =
        String::Factory(str_decimal.empty() ? OT_DECIMAL_POINT : str_decimal);

    // NOTE: from web searching, I've determined that locale / moneypunct has
    // internationalization problems. Therefore it looks like if you want to
    // build OT for various languages / regions, you're just going to have to
    // edit stdafx.hpp and change the OT_THOUSANDS_SEP and OT_DECIMAL_POINT
    // variables.
    //
    // The best improvement I can think on that is to check locale and then use
    // it to choose from our own list of hardcoded values. Todo.

    str_output = UnitDefinition::formatLongAmount(
        amount,
        std::pow(10, DecimalPower()),
        DecimalPower(),
        (proto::UNITTYPE_CURRENCY == Type()) ? primary_unit_symbol_.c_str()
                                             : nullptr,
        strSeparator->Get(),
        strDecimalPoint->Get());
    return true;  // Note: might want to return false if str_output is empty.
}

// Convert 912545 to "9,125.45"
//
// (Example assumes a Factor of 100, Decimal Power of 2
//  separator of "," and decimal point of ".")
//
bool UnitDefinition::FormatAmountWithoutSymbolLocale(
    std::int64_t amount,
    std::string& str_output,
    const std::string& str_thousand,
    const std::string& str_decimal) const
{
    // --------------------------------------------------------
    // Lookup separator and decimal point symbols based on locale.
    // --------------------------------------------------------
    // Get a moneypunct facet from the global ("C") locale
    //
    // NOTE: Turns out moneypunct kind of sucks.
    // As a result, for internationalization purposes,
    // these values have to be set here before compilation.
    //
    auto strSeparator =
        String::Factory(str_thousand.empty() ? OT_THOUSANDS_SEP : str_thousand);
    auto strDecimalPoint =
        String::Factory(str_decimal.empty() ? OT_DECIMAL_POINT : str_decimal);

    str_output = UnitDefinition::formatLongAmount(
        amount,
        std::pow(10, DecimalPower()),
        DecimalPower(),
        nullptr,
        strSeparator->Get(),
        strDecimalPoint->Get());
    return true;  // Note: might want to return false if str_output is empty.
}

// Convert "$9,125.45" to 912545.
//
// (Assuming a Factor of 100, Decimal Power of 2, separator of "," and decimal
// point of ".")
//
bool UnitDefinition::StringToAmountLocale(
    std::int64_t& amount,
    const std::string& str_input,
    const std::string& str_thousand,
    const std::string& str_decimal) const
{
    // Lookup separator and decimal point symbols based on locale.

    // Get a moneypunct facet from the global ("C") locale
    //

    // NOTE: from web searching, I've determined that locale / moneypunct has
    // internationalization problems. Therefore it looks like if you want to
    // build OT for various languages / regions, you're just going to have to
    // edit stdafx.hpp and change the OT_THOUSANDS_SEP and OT_DECIMAL_POINT
    // variables.
    //
    // The best improvement I can think on that is to check locale and then use
    // it to choose from our own list of hardcoded values. Todo.

    auto strSeparator =
        String::Factory(str_thousand.empty() ? OT_THOUSANDS_SEP : str_thousand);
    auto strDecimalPoint =
        String::Factory(str_decimal.empty() ? OT_DECIMAL_POINT : str_decimal);

    bool bSuccess = UnitDefinition::ParseFormatted(
        amount,
        str_input,
        std::pow(10, DecimalPower()),
        DecimalPower(),
        strSeparator->Get(),
        strDecimalPoint->Get());

    return bSuccess;
}

}  // namespace opentxs
