// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_LIBRARY_ASYMMETRICPROVIDER_HPP
#define OPENTXS_CRYPTO_LIBRARY_ASYMMETRICPROVIDER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
namespace crypto
{
class AsymmetricProvider
{
public:
    EXPORT static proto::AsymmetricKeyType CurveToKeyType(
        const EcdsaCurve& curve);
    EXPORT static EcdsaCurve KeyTypeToCurve(
        const proto::AsymmetricKeyType& type);

    EXPORT virtual bool Sign(
        const Data& plaintext,
        const key::Asymmetric& theKey,
        const proto::HashType hashType,
        Data& signature,  // output
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* exportPassword = nullptr) const = 0;
    EXPORT virtual bool SignContract(
        const String& strContractUnsigned,
        const key::Asymmetric& theKey,
        Signature& theSignature,  // output
        const proto::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const = 0;
    EXPORT virtual bool Verify(
        const Data& plaintext,
        const key::Asymmetric& theKey,
        const Data& signature,
        const proto::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const = 0;
    EXPORT virtual bool VerifyContractSignature(
        const String& strContractToVerify,
        const key::Asymmetric& theKey,
        const Signature& theSignature,
        const proto::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const = 0;

    EXPORT virtual ~AsymmetricProvider() = default;

protected:
    AsymmetricProvider() = default;

private:
    AsymmetricProvider(const AsymmetricProvider&) = delete;
    AsymmetricProvider(AsymmetricProvider&&) = delete;
    AsymmetricProvider& operator=(const AsymmetricProvider&) = delete;
    AsymmetricProvider& operator=(AsymmetricProvider&&) = delete;
};
}  // namespace crypto
}  // namespace opentxs
#endif
