// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/LegacySymmetric.hpp"
#include "opentxs/OT.hpp"

#include "Data.hpp"

#include <set>
#include <map>

#include "Identifier.hpp"

template class opentxs::Pimpl<opentxs::Identifier>;
template class std::set<opentxs::OTIdentifier>;
template class std::map<opentxs::OTIdentifier, std::set<opentxs::OTIdentifier>>;

namespace std
{
bool less<opentxs::Pimpl<opentxs::Identifier>>::operator()(
    const opentxs::OTIdentifier& lhs,
    const opentxs::OTIdentifier& rhs) const
{
    return lhs.get() < rhs.get();
}
}  // namespace std

namespace opentxs
{
bool operator==(const OTIdentifier& lhs, const Identifier& rhs)
{
    return lhs.get() == rhs;
}

bool operator!=(const OTIdentifier& lhs, const Identifier& rhs)
{
    return lhs.get() != rhs;
}

bool operator<(const OTIdentifier& lhs, const OTIdentifier& rhs)
{
    return lhs.get() < rhs.get();
}

OTIdentifier Identifier::Factory()
{
    return OTIdentifier(new implementation::Identifier());
}

OTIdentifier Identifier::Factory(const Identifier& rhs)
{
    return OTIdentifier(rhs.clone());
}

OTIdentifier Identifier::Factory(const std::string& rhs)
{
    return OTIdentifier(new implementation::Identifier(rhs));
}

OTIdentifier Identifier::Factory(const String& rhs)
{
    return OTIdentifier(new implementation::Identifier(rhs));
}

OTIdentifier Identifier::Factory(const Nym& nym)
{
    return OTIdentifier(new implementation::Identifier(nym));
}

OTIdentifier Identifier::Factory(const Contract& contract)
{
    return OTIdentifier(new implementation::Identifier(contract));
}

OTIdentifier Identifier::Factory(const Cheque& cheque)
{
    return OTIdentifier(
        implementation::Identifier::contract_contents_to_identifier(cheque));
}

OTIdentifier Identifier::Factory(const Item& item)
{
    return OTIdentifier(
        implementation::Identifier::contract_contents_to_identifier(item));
}

OTIdentifier Identifier::Factory(const crypto::key::LegacySymmetric& key)
{
    return OTIdentifier(new implementation::Identifier(key));
}

OTIdentifier Identifier::Factory(const OTCachedKey& key)
{
    return OTIdentifier(new implementation::Identifier(key));
}

OTIdentifier Identifier::Factory(
    const proto::ContactItemType type,
    const proto::HDPath& path)
{
    return OTIdentifier(new implementation::Identifier(type, path));
}

OTIdentifier Identifier::Random()
{
    OTIdentifier output{new implementation::Identifier};
    auto nonce = Data::Factory();
    OT::App().Crypto().Encode().Nonce(32, nonce);

    OT_ASSERT(32 == nonce->size());

    output->CalculateDigest(nonce);

    OT_ASSERT(false == output->empty());

    return output;
}

bool Identifier::Validate(const std::string& input)
{
    if (input.empty()) { return false; }

    const auto id = Factory(input);

    return (0 < id->size());
}
}  // namespace opentxs

namespace opentxs::implementation
{
Identifier::Identifier()
    : ot_super()
{
}

Identifier::Identifier(const std::string& theStr)
    : ot_super()
{
    SetString(theStr);
}

Identifier::Identifier(const String& theStr)
    : ot_super()
{
    SetString(theStr);
}

Identifier::Identifier(const Contract& theContract)
    : ot_super()  // Get the contract's ID into this identifier.
{
    (const_cast<Contract&>(theContract)).GetIdentifier(*this);
}

Identifier::Identifier(const Nym& theNym)
    : ot_super()  // Get the Nym's ID into this identifier.
{
    (const_cast<Nym&>(theNym)).GetIdentifier(*this);
}

Identifier::Identifier(const crypto::key::LegacySymmetric& theKey)
    : ot_super()  // Get the Symmetric Key's ID into *this. (It's a hash of the
                  // encrypted form of the symmetric key.)
{
    (const_cast<crypto::key::LegacySymmetric&>(theKey)).GetIdentifier(*this);
}

Identifier::Identifier(const OTCachedKey& theKey)
    : ot_super()  // Cached Key stores a symmetric key inside, so this actually
                  // captures the ID for that symmetrickey.
{
    const bool bSuccess =
        (const_cast<OTCachedKey&>(theKey)).GetIdentifier(*this);

    OT_ASSERT(bSuccess);  // should never fail. If it does, then we are calling
    // this function at a time we shouldn't, when we aren't
    // sure the master key has even been generated yet. (If
    // this asserts, need to examine the line of code that
    // tried to do this, and figure out where its logic
    // went wrong, since it should have made sure this
    // would not happen, before constructing like this.)
}

Identifier::Identifier(
    const Vector& data,
    const std::size_t size,
    const ID type)
    : ot_super(data, size)
    , type_(type)
{
}

Identifier::Identifier(
    const proto::ContactItemType type,
    const proto::HDPath& path)
    : ot_super()
{
    CalculateDigest(path_to_data(type, path), DefaultType);
}

bool Identifier::operator==(const opentxs::Identifier& s2) const
{
    const auto ots1 = String::Factory(*this), ots2 = String::Factory(s2);
    return ots1->Compare(ots2);
}

bool Identifier::operator!=(const opentxs::Identifier& s2) const
{
    const auto ots1 = String::Factory(*this), ots2 = String::Factory(s2);
    return !(ots1->Compare(ots2));
}

bool Identifier::operator>(const opentxs::Identifier& s2) const
{
    const auto ots1 = String::Factory(*this), ots2 = String::Factory(s2);
    return ots1->operator>(ots2);
}

bool Identifier::operator<(const opentxs::Identifier& s2) const
{
    const auto ots1 = String::Factory(*this), ots2 = String::Factory(s2);
    return ots1->operator<(ots2);
}

bool Identifier::operator<=(const opentxs::Identifier& s2) const
{
    const auto ots1 = String::Factory(*this), ots2 = String::Factory(s2);
    return ots1->operator<=(ots2);
}

bool Identifier::operator>=(const opentxs::Identifier& s2) const
{
    const auto ots1 = String::Factory(*this), ots2 = String::Factory(s2);
    return ots1->operator>=(ots2);
}

bool Identifier::CalculateDigest(const String& strInput, const ID type)
{
    type_ = type;

    return OT::App().Crypto().Hash().Digest(
        IDToHashType(type_), strInput, *this);
}

bool Identifier::CalculateDigest(const opentxs::Data& dataInput, const ID type)
{
    type_ = type;

    return OT::App().Crypto().Hash().Digest(
        IDToHashType(type_), dataInput, *this);
}

Identifier* Identifier::clone() const
{
    return new Identifier(data_, position_, type_);
}

Identifier* Identifier::contract_contents_to_identifier(const Contract& in)
{
    auto output = new Identifier();

    OT_ASSERT(nullptr != output);

    output->CalculateDigest(String::Factory(in));

    return output;
}

// This Identifier is stored in binary form.
// But what if you want a pretty string version of it?
// Just call this function.
void Identifier::GetString(String& id) const
{
    auto data = Data::Factory();
    data->Assign(&type_, sizeof(type_));

    OT_ASSERT(1 == data->size());

    if (0 == size()) { return; }

    data->Concatenate(this->data(), size());

    auto output = String::Factory("ot");
    output->Concatenate(String::Factory(
        OT::App().Crypto().Encode().IdentifierEncode(data).c_str()));
    id.swap(output);
}

proto::HashType Identifier::IDToHashType(const ID type)
{
    switch (type) {
        case (ID::SHA256): {
            return proto::HASHTYPE_SHA256;
        }
        case (ID::BLAKE2B): {
            return proto::HASHTYPE_BLAKE2B160;
        }
        default: {
            return proto::HASHTYPE_NONE;
        }
    }
}

OTData Identifier::path_to_data(
    const proto::ContactItemType type,
    const proto::HDPath& path)
{
    auto output = Data::Factory(static_cast<const void*>(&type), sizeof(type));
    output += Data::Factory(path.root().c_str(), path.root().size());

    for (const auto& child : path.child()) {
        output += Data::Factory(&child, sizeof(child));
    }

    return output;
}

// SET (binary id) FROM ENCODED STRING
void Identifier::SetString(const String& encoded)
{
    return SetString(std::string(encoded.Get()));
}

void Identifier::SetString(const std::string& encoded)
{
    empty();

    if (MinimumSize > encoded.size()) { return; }

    if ('o' != encoded.at(0)) { return; }
    if ('t' != encoded.at(1)) { return; }

    std::string input(encoded.data() + 2, encoded.size() - 2);
    auto data = OT::App().Crypto().Encode().IdentifierDecode(input);

    if (!data.empty()) {
        type_ = static_cast<ID>(data[0]);

        switch (type_) {
            case (ID::SHA256): {
            } break;
            case (ID::BLAKE2B): {
            } break;
            default: {
                type_ = ID::ERROR;

                return;
            }
        }

        Assign(
            (reinterpret_cast<const std::uint8_t*>(data.data()) + 1),
            (data.size() - 1));
    }
}

std::string Identifier::str() const
{
    auto output = String::Factory();
    GetString(output);

    return output->Get();
}

void Identifier::swap(opentxs::Identifier& rhs)
{
    auto& input = dynamic_cast<Identifier&>(rhs);
    ot_super::swap(std::move(input));
    std::swap(type_, input.type_);
}
}  // namespace opentxs::implementation
