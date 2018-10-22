// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CASH_PURSE_HPP
#define OPENTXS_CASH_PURSE_HPP

#include "opentxs/Forward.hpp"

#if OT_CASH
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/LegacySymmetric.hpp"

#include <cstdint>
#include <deque>
#include <memory>

namespace opentxs
{
namespace api
{
namespace implementation
{

class Factory;

}  // namespace implementation
}  // namespace api

// A token has no Nym ID, or Account ID, or even a traceable TokenID (the
// tokenID only becomes relevant after it is spent.) But a purse can be stuffed
// full of tokens, and can be saved by accountID as filename, and can have its
// contents encrypted to the public key of a specific user.
//
// I will add an optional NymID field, so it's obvious whose public key to use
// for opening the tokens. This may seem odd, but the field is entirely optional
// because it's not necessary for the actual operation. The recipient will
// already know to use his own private key to open the purse, and then he will
// immediately open it, redeem the coins, and store the replacements again
// encrypted to his own key, until he spends them again to someone else, when he
// will also know to encrypt the purse to THEIR public key, and so on.
//
// The interface of this class is that of a simple stack. Imagine a stack of
// poker chips.

typedef std::deque<OTArmored> dequeOfTokens;

class Purse : public Contract
{
public:
    std::int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;
    /// What if you DON'T want to encrypt the purse to your Nym?? What if you
    /// just want to use a passphrase instead? That's what these functions are
    /// for. OT just generates an internal symmetric key and stores it INSIDE
    /// THE PURSE. You set the passphrase for the internal key, and thereafter
    /// your experience is one of a password-protected purse.
    /** Create internal symmetric key for password-protected purse. */
    EXPORT bool GenerateInternalKey();
    /** symmetric key for this purse.*/
    EXPORT crypto::key::LegacySymmetric& GetInternalKey()
    {
        return m_pSymmetricKey;
    }
    /** stores the passphrase for the symmetric key.*/
    EXPORT const OTCachedKey& GetInternalMaster();
    /** Retrieves the passphrase for this purse (which is cached by the master
     * key.) Prompts the user to enter his actual passphrase, if necessary to
     * unlock it. (May not need unlocking yet -- there is a timeout.)*/
    EXPORT bool GetPassphrase(
        OTPassword& theOutput,
        const char* szDisplay = nullptr);
    /** NymID may be left blank, with user left guessing. */
    EXPORT bool IsNymIDIncluded() const { return m_bIsNymIDIncluded; }
    EXPORT bool IsPasswordProtected() const { return m_bPasswordProtected; }
    /** This will return false every time, if IsNymIDIncluded() is false. */
    EXPORT bool GetNymID(Identifier& theOutput) const;
    /** FYI: OTPurse::Push makes its own copy of theToken and does NOT take
     * ownership of the one passed in. */
    EXPORT bool Push(OTNym_or_SymmetricKey theOwner, const Token& theToken);
    /** Caller IS responsible to delete.*/
    EXPORT Token* Pop(OTNym_or_SymmetricKey theOwner);
    /** Caller IS responsible to delete. Peek returns a copy of the token.*/
    EXPORT Token* Peek(OTNym_or_SymmetricKey theOwner) const;
    EXPORT std::int32_t Count() const;
    EXPORT bool IsEmpty() const;
    inline std::int64_t GetTotalValue() const { return m_lTotalValue; }
    EXPORT time64_t GetLatestValidFrom() const;
    EXPORT time64_t GetEarliestValidTo() const;
    /** Verify whether the CURRENT date is WITHIN the VALID FROM / TO dates.
     * NOTE: Keep in mind that a purse's expiration dates are based on ALL the
     * tokens within. Therefore this will never be as accurate as individually
     * examining those tokens... */
    EXPORT bool VerifyCurrentDate();
    /** Verify whether the CURRENT date is AFTER the the "VALID TO" date. */
    EXPORT bool IsExpired();
    EXPORT bool Merge(
        const Nym& theSigner,
        OTNym_or_SymmetricKey theOldNym,
        OTNym_or_SymmetricKey theNewNym,
        Purse& theNewPurse);
    EXPORT bool LoadPurse(
        const char* szNotaryID = nullptr,
        const char* szNymID = nullptr,
        const char* szInstrumentDefinitionID = nullptr);
    EXPORT bool SavePurse(
        const char* szNotaryID = nullptr,
        const char* szNymID = nullptr,
        const char* szInstrumentDefinitionID = nullptr);

    bool LoadContract() override;

    inline const Identifier& GetNotaryID() const { return m_NotaryID; }
    inline const Identifier& GetInstrumentDefinitionID() const
    {
        return m_InstrumentDefinitionID;
    }
    EXPORT void InitPurse();
    void Release() override;
    EXPORT void Release_Purse();
    EXPORT void ReleaseTokens();

    EXPORT virtual ~Purse();

protected:
    void UpdateContents() override;  // Before transmission or serialization,
                                     // this
                                     // is where the Purse saves its contents

    dequeOfTokens m_dequeTokens;

    // TODO: Add a boolean value, so that the NymID is either for a real user,
    // or is for a temp Nym which must be ATTACHED to the purse, if that boolean
    // is set to true.

    OTIdentifier m_NymID;                   // Optional
    OTIdentifier m_NotaryID;                // Mandatory
    OTIdentifier m_InstrumentDefinitionID;  // Mandatory
    std::int64_t m_lTotalValue{0};  // Push increments this by denomination, and
                                    // Pop decrements it by denomination.
    bool m_bPasswordProtected{false};  // this purse might be encrypted to a
                                       // passphrase, instead of a Nym.
    // If that's the case, BTW, then there will be a Symmetric Key and a Master
    // Key. The symmetric key is used to store the actual key for
    // encrypting/decrypting the tokens in this purse. Whereas the master key is
    // used for retrieving the passphrase to use for unlocking the symmetric
    // key. The passphrase in question is actually a random number stored inside
    // the master key, inside its own internal symmetric key. In order to unlock
    // it, OTCachedKey may occasionally ask the user to enter a passphrase,
    // which is used to derived a key to unlock it. This key may then be cached
    // in memory by OTCachedKey until a timeout, and later be zapped by a thread
    // for that purpose.
    bool m_bIsNymIDIncluded{false};  // It's possible to use a purse WITHOUT
                                     // attaching the relevant NymID. (The
                                     // holder of the purse just has to "know"
                                     // what the correct NymID is, or it won't
                                     // work.) This bool tells us whether the ID
                                     // is attached, or not.
    OTLegacySymmetricKey m_pSymmetricKey{
        crypto::key::LegacySymmetric::Blank()};  // If this purse
                                                 // contains its own
    // symmetric key (instead of using an
    // owner Nym)...
    // ...then it will have a master key as well, for unlocking that symmetric
    // key, and managing timeouts, etc.
    std::shared_ptr<OTCachedKey> m_pCachedKey;
    time64_t m_tLatestValidFrom{0};  // The tokens in the purse may become valid
                                     // on different dates. This stores the
                                     // latest one.
    time64_t m_tEarliestValidTo{0};  // The tokens in the purse may have
                                     // different expirations. This stores the
                                     // earliest one.
    void RecalculateExpirationDates(OTNym_or_SymmetricKey& theOwner);

private:
    friend api::implementation::Factory;

    typedef Contract ot_super;

    /** just for copy another purse's Server and Instrument Definition Id */
    Purse(const api::Core& core, const Purse& thePurse);
    /** similar thing */
    Purse(
        const api::Core& core,
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID);
    /** Don't use this unless you really don't know the instrument definition
     * (Like if you're about to read it out of a string.) */
    Purse(const api::Core& core, const Identifier& NOTARY_ID);
    /** Normally you really really want to set the instrument definition. */
    Purse(
        const api::Core& core,
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& NYM_ID);  // NymID optional
    Purse(const api::Core& core);

    Purse() = delete;
};
}  // namespace opentxs
#endif  // OT_CASH
#endif
