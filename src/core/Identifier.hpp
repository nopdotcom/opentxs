// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::implementation
{
class Identifier final : virtual public opentxs::Identifier,
                         virtual public opentxs::identifier::Nym,
                         public Data
{
public:
    using ot_super = Data;

    using ot_super::operator==;
    bool operator==(const opentxs::Identifier& rhs) const override;
    using ot_super::operator!=;
    bool operator!=(const opentxs::Identifier& rhs) const override;
    bool operator>(const opentxs::Identifier& rhs) const override;
    bool operator<(const opentxs::Identifier& rhs) const override;
    bool operator<=(const opentxs::Identifier& rhs) const override;
    bool operator>=(const opentxs::Identifier& rhs) const override;

    void GetString(String& theStr) const override;
    std::string str() const override;
    const ID& Type() const override { return type_; }

    bool CalculateDigest(
        const opentxs::Data& dataInput,
        const ID type = DefaultType) override;
    bool CalculateDigest(const String& strInput, const ID type = DefaultType)
        override;
    void SetString(const std::string& encoded) override;
    void SetString(const String& encoded) override;
    using ot_super::swap;
    void swap(opentxs::Identifier& rhs) override;

    ~Identifier() = default;

private:
    friend opentxs::Identifier;
    friend opentxs::identifier::Nym;

    static const ID DefaultType{ID::BLAKE2B};
    static const std::size_t MinimumSize{10};

    ID type_{DefaultType};

    Identifier* clone() const override;

    static Identifier* contract_contents_to_identifier(const Contract& in);
    static proto::HashType IDToHashType(const ID type);
    static OTData path_to_data(
        const proto::ContactItemType type,
        const proto::HDPath& path);

    explicit Identifier(const std::string& rhs);
    explicit Identifier(const String& rhs);
    explicit Identifier(const opentxs::Nym& nym);
    explicit Identifier(const Contract& contract);
    explicit Identifier(const crypto::key::LegacySymmetric& key);
    explicit Identifier(const OTCachedKey& key);
    explicit Identifier(
        const Vector& data,
        const std::size_t size,
        const ID type);
    Identifier(const proto::ContactItemType type, const proto::HDPath& path);
    Identifier();
    Identifier(const Identifier& rhs) = delete;
    Identifier(Identifier&& rhs) = delete;
    Identifier& operator=(const Identifier& rhs) = delete;
    Identifier& operator=(Identifier&& rhs) = delete;
};
}  // namespace opentxs::implementation
