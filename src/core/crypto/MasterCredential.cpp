// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

//////////////////////////////////////////////////////////////////////

// A nym contains a list of credential sets.
// The whole purpose of a Nym is to be an identity, which can have
// master credentials.
//
// Each CredentialSet contains list of Credentials. One of the
// Credentials is a MasterCredential, and the rest are ChildCredentials
// signed by the MasterCredential.
//
// A Credential may contain keys, in which case it is a KeyCredential.
//
// Credentials without keys might be an interface to a hardware device
// or other kind of external encryption and authentication system.
//
// Non-key Credentials are not yet implemented.
//
// Each KeyCredential has 3 OTKeypairs: encryption, signing, and authentication.
// Each OTKeypair has 2 crypto::key::Asymmetrics (public and private.)
//
// A MasterCredential must be a KeyCredential, and is only used to sign
// ChildCredentials
//
// ChildCredentials are used for all other actions, and never sign other
// Credentials

#include "stdafx.hpp"

#include "opentxs/core/crypto/MasterCredential.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/CredentialSet.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#include "opentxs/core/crypto/PaymentCode.hpp"
#endif
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/NymIDSource.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/Proto.hpp"

#include <memory>
#include <ostream>

#define OT_METHOD "opentxs::MasterCredential::"

namespace opentxs
{
MasterCredential::MasterCredential(
    const api::Core& api,
    CredentialSet& theOwner,
    const proto::Credential& serializedCred)
    : ot_super(api, theOwner, serializedCred)
{
    role_ = proto::CREDROLE_MASTERKEY;
    auto source = std::make_shared<NymIDSource>(
        api_.Factory(), serializedCred.masterdata().source());
    owner_backlink_->SetSource(source);
    source_proof_.reset(
        new proto::SourceProof(serializedCred.masterdata().sourceproof()));
}

MasterCredential::MasterCredential(
    const api::Core& api,
    CredentialSet& theOwner,
    const NymParameters& nymParameters)
    : ot_super(api, theOwner, nymParameters)
{
    role_ = proto::CREDROLE_MASTERKEY;

    std::shared_ptr<NymIDSource> source;
    auto sourceProof = std::make_unique<proto::SourceProof>();

    proto::SourceProofType proofType = nymParameters.SourceProofType();

    if (proto::SOURCETYPE_PUBKEY == nymParameters.SourceType()) {
        OT_ASSERT_MSG(
            proto::SOURCEPROOFTYPE_SELF_SIGNATURE == proofType,
            "non self-signed credentials not yet implemented");

        source = std::make_shared<NymIDSource>(
            api_.Factory(),
            nymParameters,
            *(signing_key_->GetPublicKey().Serialize()));
        sourceProof->set_version(1);
        sourceProof->set_type(proto::SOURCEPROOFTYPE_SELF_SIGNATURE);

    }
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    else if (proto::SOURCETYPE_BIP47 == nymParameters.SourceType()) {
        sourceProof->set_version(1);
        sourceProof->set_type(proto::SOURCEPROOFTYPE_SIGNATURE);

        auto bip47Source = api_.Factory().PaymentCode(
            nymParameters.Seed(), nymParameters.Nym(), PAYMENT_CODE_VERSION);
        source = std::make_shared<NymIDSource>(api_.Factory(), bip47Source);
    }
#endif

    source_proof_.reset(sourceProof.release());
    owner_backlink_->SetSource(source);
    std::string nymID = owner_backlink_->GetNymID();

    nym_id_ = nymID;
}

/** Verify that nym_id_ is the same as the hash of m_strSourceForNymID. Also
 * verify that *this == owner_backlink_->GetMasterCredential() (the master
 * credential.) Verify the (self-signed) signature on *this. */
bool MasterCredential::verify_internally(const Lock& lock) const
{
    // Perform common Key Credential verifications
    if (!ot_super::verify_internally(lock)) { return false; }

    // Check that the source validates this credential
    if (!verify_against_source(lock)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed verifying master credential against "
            "nym id source.")
            .Flush();

        return false;
    }

    return true;
}

bool MasterCredential::verify_against_source(const Lock& lock) const
{
    std::shared_ptr<proto::Credential> serialized;

    switch (owner_backlink_->Source().Type()) {
        case proto::SOURCETYPE_PUBKEY: {
            serialized = serialize(lock, AS_PUBLIC, WITH_SIGNATURES);
        } break;
        case proto::SOURCETYPE_BIP47: {
            serialized = serialize(lock, AS_PUBLIC, WITHOUT_SIGNATURES);
        } break;
        default: {
            return false;
        }
    }

    auto sourceSig = SourceSignature();

    if (!sourceSig) {
        otErr << __FUNCTION__ << ": Master credential not signed by its"
              << " source." << std::endl;

        return false;
    }

    return owner_backlink_->Source().Verify(*serialized, *sourceSig);
}

bool MasterCredential::New(const NymParameters& nymParameters)
{
    if (!ot_super::New(nymParameters)) { return false; }

    if (proto::SOURCEPROOFTYPE_SELF_SIGNATURE != source_proof_->type()) {
        SerializedSignature sig = std::make_shared<proto::Signature>();
        bool haveSourceSig = owner_backlink_->Sign(*this, *sig);

        if (haveSourceSig) {
            signatures_.push_back(sig);

            return true;
        }
    }

    return true;
}

serializedCredential MasterCredential::serialize(
    const Lock& lock,
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
{
    auto serializedCredential = ot_super::serialize(lock, asPrivate, asSigned);

    std::unique_ptr<proto::MasterCredentialParameters> parameters(
        new proto::MasterCredentialParameters);

    OT_ASSERT(parameters);

    parameters->set_version(1);
    *(parameters->mutable_source()) = *(owner_backlink_->Source().Serialize());

    serializedCredential->set_allocated_masterdata(parameters.release());

    serializedCredential->set_role(proto::CREDROLE_MASTERKEY);
    *(serializedCredential->mutable_masterdata()->mutable_sourceproof()) =
        *source_proof_;

    return serializedCredential;
}

bool MasterCredential::Verify(
    const proto::Credential& credential,
    const proto::CredentialRole& role,
    const Identifier& masterID,
    const proto::Signature& masterSig) const
{
    if (!proto::Validate<proto::Credential>(
            credential, VERBOSE, proto::KEYMODE_PUBLIC, role, false)) {
        otErr << __FUNCTION__ << ": Invalid credential syntax." << std::endl;

        return false;
    }

    bool sameMaster = (id_ == masterID);

    if (!sameMaster) {
        otErr << __FUNCTION__ << ": Credential does not designate this"
              << " credential as its master." << std::endl;

        return false;
    }

    proto::Credential copy;
    copy.CopyFrom(credential);
    auto& signature = *copy.add_signature();
    signature.CopyFrom(masterSig);
    signature.clear_signature();

    return Verify(proto::ProtoAsData(copy), masterSig);
}

bool MasterCredential::hasCapability(const NymCapability& capability) const
{
    switch (capability) {
        case (NymCapability::SIGN_CHILDCRED): {
            return signing_key_->hasCapability(capability);
        }
        default: {
        }
    }

    return false;
}

bool MasterCredential::Path(proto::HDPath& output) const
{
    if (false == signing_key_->HasPrivateKey()) {
        otErr << OT_METHOD << __FUNCTION__ << ": No private key." << std::endl;

        return false;
    }

    const bool found = signing_key_->GetPrivateKey().Path(output);
    output.mutable_child()->RemoveLast();

    return found;
}

std::string MasterCredential::Path() const
{
    return signing_key_->GetPrivateKey().Path();
}
}  // namespace opentxs
