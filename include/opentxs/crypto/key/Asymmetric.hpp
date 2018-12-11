// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_KEY_ASYMMETRIC_HPP
#define OPENTXS_CRYPTO_KEY_ASYMMETRIC_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/Proto.hpp"

#include <cstdint>
#include <list>
#include <memory>
#include <string>

namespace opentxs
{
namespace crypto
{
namespace key
{
typedef std::list<Asymmetric*> listOfAsymmetricKeys;

class Asymmetric
{
public:
    EXPORT static OTAsymmetricKey Factory();
    EXPORT static OTAsymmetricKey Factory(
        const proto::AsymmetricKeyType keyType,
        const String& pubkey);
    EXPORT static OTAsymmetricKey Factory(
        const NymParameters& nymParameters,
        const proto::KeyRole role);
    EXPORT static OTAsymmetricKey Factory(
        const proto::AsymmetricKey& serializedKey);
    EXPORT static OTString KeyTypeToString(
        const proto::AsymmetricKeyType keyType);
    EXPORT static proto::AsymmetricKeyType StringToKeyType(
        const String& keyType);

    /** Only works for public keys. */
    EXPORT virtual bool CalculateID(Identifier& theOutput) const = 0;
    EXPORT virtual const opentxs::crypto::AsymmetricProvider& engine()
        const = 0;
    EXPORT virtual const OTSignatureMetadata* GetMetadata() const = 0;
    EXPORT virtual bool GetPublicKey(String& strKey) const = 0;
    EXPORT virtual bool hasCapability(
        const NymCapability& capability) const = 0;
    EXPORT virtual bool IsEmpty() const = 0;
    EXPORT virtual bool IsPrivate() const = 0;
    EXPORT virtual bool IsPublic() const = 0;
    EXPORT virtual proto::AsymmetricKeyType keyType() const = 0;
    EXPORT virtual const std::string Path() const = 0;
    EXPORT virtual bool Path(proto::HDPath& output) const = 0;
    EXPORT virtual bool ReEncryptPrivateKey(
        const OTPassword& theExportPassword,
        bool bImporting) const = 0;
    EXPORT virtual const proto::KeyRole& Role() const = 0;
    EXPORT virtual std::shared_ptr<proto::AsymmetricKey> Serialize() const = 0;
    EXPORT virtual OTData SerializeKeyToData(
        const proto::AsymmetricKey& rhs) const = 0;
    EXPORT virtual proto::HashType SigHashType() const = 0;
    EXPORT virtual bool Sign(
        const Data& plaintext,
        proto::Signature& sig,
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* exportPassword = nullptr,
        const String& credID = String::Factory(""),
        const proto::SignatureRole role = proto::SIGROLE_ERROR) const = 0;
    template <class C>
    EXPORT bool SignProto(
        C& serialized,
        proto::Signature& signature,
        const String& credID = String::Factory(""),
        const OTPasswordData* pPWData = nullptr) const
    {
        if (IsPublic()) {
            LogOutput(": You must use private keys to create signatures.")
                  .Flush();

            return false;
        }

        if (0 == signature.version()) { signature.set_version(1); }

        signature.set_credentialid(credID.Get());

        if ((proto::HASHTYPE_ERROR == signature.hashtype()) ||
            !signature.has_hashtype()) {
            signature.set_hashtype(SigHashType());
        }

        auto sig = Data::Factory();
        bool goodSig = engine().Sign(
            proto::ProtoAsData<C>(serialized),
            *this,
            signature.hashtype(),
            sig,
            pPWData,
            nullptr);

        if (goodSig) { signature.set_signature(sig->data(), sig->size()); }

        return goodSig;
    }
    EXPORT virtual bool TransportKey(Data& publicKey, OTPassword& privateKey)
        const = 0;
    EXPORT virtual bool Verify(
        const Data& plaintext,
        const proto::Signature& sig) const = 0;

    // Only used for RSA keys
    EXPORT[[deprecated]] virtual void Release() = 0;
    EXPORT[[deprecated]] virtual void ReleaseKey() = 0;
    /** Don't use this, normally it's not necessary. */
    EXPORT virtual void SetAsPublic() = 0;
    /** (Only if you really know what you are doing.) */
    EXPORT virtual void SetAsPrivate() = 0;

    EXPORT virtual operator bool() const = 0;
    EXPORT virtual bool operator==(const proto::AsymmetricKey&) const = 0;

    EXPORT virtual ~Asymmetric() = default;

protected:
    Asymmetric() = default;

private:
    friend OTAsymmetricKey;

    virtual Asymmetric* clone() const = 0;

    Asymmetric(const Asymmetric&) = delete;
    Asymmetric(Asymmetric&&) = delete;
    Asymmetric& operator=(const Asymmetric&) = delete;
    Asymmetric& operator=(Asymmetric&&) = delete;
};
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
#endif
