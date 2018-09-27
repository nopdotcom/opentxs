// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CLIENT_OTAPI_EXEC_HPP
#define OPENTXS_CLIENT_OTAPI_EXEC_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/client/OT_API.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <mutex>
#include <set>
#include <string>

namespace opentxs
{
namespace api
{
namespace client
{
namespace implementation
{
class Manager;
}  // namespace implementation
}  // namespace client
}  // namespace api

class OTAPI_Exec : Lockable
{
public:
    EXPORT std::int64_t StringToLong(const std::string& strNumber) const;
    EXPORT std::string LongToString(const std::int64_t& lNumber) const;

    EXPORT std::uint64_t StringToUlong(const std::string& strNumber) const;
    EXPORT std::string UlongToString(const std::uint64_t& lNumber) const;

    EXPORT bool IsValidID(const std::string& strPurportedID) const;
    EXPORT std::string NymIDFromPaymentCode(
        const std::string& paymentCode) const;

    // SetAppBinaryFolder
    // OPTIONAL. Used in Android and Qt.
    //
    // Certain platforms use this to override the Prefix folder.
    // Basically /usr/local is the prefix folder by default, meaning
    // /usr/local/lib/opentxs will be the location of the scripts. But
    // if you override AppBinary folder to, say, "res/raw"
    // (Android does something like that) then even though the prefix remains
    // as /usr/local, the scripts folder will be res/raw
    //
    //
    EXPORT static void SetAppBinaryFolder(const std::string& Location);

    // SetHomeFolder
    // OPTIONAL. Used in Android.
    //
    // The AppDataFolder, such as /Users/au/.ot, is constructed from the home
    // folder, such as /Users/au.
    //
    // Normally the home folder is auto-detected, but certain platforms, such as
    // Android, require us to explicitly set this folder from the Java code.
    // Then
    // the AppDataFolder is constructed from it. (It's the only way it can be
    // done.)
    //
    // In Android, you would SetAppBinaryFolder to the path to
    // "/data/app/packagename/res/raw",
    // and you would SetHomeFolder to "/data/data/[app package]/files/"
    //
    EXPORT static void SetHomeFolder(const std::string& Location);
    // Then:

    /**
    INITIALIZE an OTAPI context.

    Call this once per context.

    If the configuration value doesn't exist, it will be created.

    Something like this:  bool bInit = Init();

    If this function fails, you can call it multiple times until it succeeds.


    EXPORT bool Cleanup(); // Call this before you delete OTAPI
    */
    /**
    SET WALLET (optional)

    This command will set the wallet filename in the configuration.
    Use this command to change what wallet will be loaded with the
    "LOAD WALLET" command.

    e.g. SetWallet("wallet2.xml");

    */
    EXPORT bool SetWallet(const std::string& strWalletFilename) const;

    /**
    WALLET EXISTS

    Just Checks if the m_pWallet pointer is not null.

    WalletExists();

    */
    EXPORT bool WalletExists() const;

    /**
    LOAD WALLET

    Next you'll want to load your wallet up. It needs to be in the data_folder.
    The filename is set in the configuration. Use "SET WALLET FILENAME" to
    change it.

    The Default Filename is "wallet.xml"

    LoadWallet();

    */
    EXPORT bool LoadWallet() const;

    /**
    SWITCH WALLET (experimental)

    Experimental.
    Set the new wallet with the "SET WALLET FILENAME" function.
    Then call this function to switch to the new wallet.
    */
    EXPORT bool SwitchWallet() const;

    /** Creates a section in the config file IF it didn't already exist.
        Returns true if the section exists after the call is complete (whether
       created or not.)
    */
    EXPORT bool CheckSetConfigSection(
        const std::string& strSection,
        const std::string& strComment) const;

    EXPORT std::string GetConfig_str(
        const std::string& strSection,
        const std::string& strKey) const;
    EXPORT std::int64_t GetConfig_long(
        const std::string& strSection,
        const std::string& strKey) const;
    EXPORT bool GetConfig_bool(
        const std::string& strSection,
        const std::string& strKey) const;

    EXPORT bool SetConfig_str(
        const std::string& strSection,
        const std::string& strKey,
        const std::string& strValue) const;
    EXPORT bool SetConfig_long(
        const std::string& strSection,
        const std::string& strKey,
        const std::int64_t& lValue) const;
    EXPORT bool SetConfig_bool(
        const std::string& strSection,
        const std::string& strKey,
        const bool bValue) const;

    /** TIME (in seconds)

    This will return the current time in seconds, as a string.
    Returns nullptr if failure.

    Todo: consider making this available on the server side as well,
    so the smart contracts can see what time it is.

    */
    EXPORT time64_t GetTime() const;

    /**
    These 2 functions aren't in the OT_API proper, but are only available inside
    the OTScripts on the client side. (They're not available in server-side
    scripts, and they're not available in, for example, the OTAPI Java
    programmable
    interface. You would use Java functions at that point.)
    Shown here only for reference.
    */
    // const std::string& OT_CLI_ReadLine();    // Reads from cin until
    // Newline.
    // const std::string& OT_CLI_ReadUntilEOF();    // Reads from cin until
    // EOF.

    // OTNumList is a class that encapsulates working with a comma-separated
    // list
    // of std::int64_t integers, stored in a std::set and easily serializable
    // in/out
    // of a string.
    // (It's useful.)
    //
    EXPORT std::string NumList_Add(
        const std::string& strNumList,
        const std::string& strNumbers) const;
    EXPORT std::string NumList_Remove(
        const std::string& strNumList,
        const std::string& strNumbers) const;
    EXPORT bool NumList_VerifyQuery(
        const std::string& strNumList,
        const std::string& strNumbers) const;  // returns OT_BOOL
    EXPORT bool NumList_VerifyAll(
        const std::string& strNumList,
        const std::string& strNumbers) const;  // returns OT_BOOL
    EXPORT std::int32_t NumList_Count(const std::string& strNumList) const;

    /** OT-encode a plainext string.

    This will pack, compress, and base64-encode a plain string.
    Returns the base64-encoded string, or nullptr.

    internally:
    OTString    strPlain(strPlaintext);
    Armored    ascEncoded(thePlaintext);    // ascEncoded now contains the
    OT-encoded string.
    return    ascEncoded.Get();    // We return it.
    */
    EXPORT std::string Encode(
        const std::string& strPlaintext,
        const bool& bLineBreaks) const;  // bLineBreaks
                                         // is OT_BOOL

    /** Decode an OT-encoded string (back to plainext.)

    EXPORT std::string Decode(const std::string& strEncoded);

    This will base64-decode, uncompress, and unpack an OT-encoded string.
    Returns the plainext string, or nullptr.

    internally:
    Armored    ascEncoded(strEncoded);
    OTString    strPlain(ascEncoded);    // strPlain now contains the decoded
    plainext string.
    return    strPlain.Get();    // We return it.
    */
    EXPORT std::string Decode(
        const std::string& strEncoded,
        const bool& bLineBreaks) const;  // bLineBreaks
                                         // is OT_BOOL

    /** OT-ENCRYPT a plainext string. (ASYMMETRIC)

    EXPORT std::string Encrypt(const std::string& RECIPIENT_NYM_ID,
    const std::string& strPlaintext);

    This will encode, ENCRYPT, and encode a plain string.
    Returns the base64-encoded ciphertext, or nullptr.

    internally the C++ code is:
    OTString    strPlain(strPlaintext);
    OTEnvelope    theEnvelope;
    if (theEnvelope.Seal(RECIPIENT_NYM, strPlain)) {    // Now it's encrypted
    (in binary form, inside the envelope), to the recipient's nym.
    Armored    ascCiphertext(theEnvelope);    // ascCiphertext now contains
    the base64-encoded ciphertext (as a string.)
    return ascCiphertext.Get();
    }

    */
    EXPORT std::string Encrypt(
        const std::string& RECIPIENT_NYM_ID,
        const std::string& strPlaintext) const;

    /** OT-DECRYPT an OT-encrypted string back to plainext. (ASYMMETRIC)

    EXPORT std::string Decrypt(const std::string& RECIPIENT_NYM_ID,
    const std::string& strCiphertext);

    Decrypts the base64-encoded ciphertext back into a normal string plainext.
    Returns the plainext string, or nullptr.

    internally the C++ code is:
    OTEnvelope    theEnvelope;    // Here is the envelope object. (The
    ciphertext IS the data for an OTEnvelope.)
    Armored    ascCiphertext(strCiphertext);    // The base64-encoded
    ciphertext passed in. Next we'll try to attach it to envelope object...
    if (theEnvelope.SetAsciiArmoredData(ascCiphertext)) {    // ...so that we
    can open it using the appropriate Nym, into a plain string object:
    OTString    strServerReply;    // This will contain the output when we're
    done.
    const bool    bOpened =    // Now we try to decrypt:
    theEnvelope.Open(RECIPIENT_NYM, strServerReply);
    if (bOpened)
    {
    return strServerReply.Get();
    }
    }
    */
    EXPORT std::string Decrypt(
        const std::string& RECIPIENT_NYM_ID,
        const std::string& strCiphertext) const;

    // Generates a new symmetric key, based on a passphrase,
    // and returns it (or nullptr.)
    //
    EXPORT std::string CreateSymmetricKey() const;

    EXPORT std::string SymmetricEncrypt(
        const std::string& SYMMETRIC_KEY,
        const std::string& PLAINTEXT) const;
    EXPORT std::string SymmetricDecrypt(
        const std::string& SYMMETRIC_KEY,
        const std::string& CIPHERTEXT_ENVELOPE) const;

    /** OT-Sign a CONTRACT. (First signature)

    EXPORT std::string SignContract(const std::string& SIGNER_NYM_ID,
    const std::string& THE_CONTRACT);

    Tries to instantiate the contract object, based on the string passed in.
    Releases all signatures, and then signs the contract.
    Returns the signed contract, or nullptr if failure.

    NOTE: The actual OT functionality (Use Cases) NEVER requires you to sign via
    this function. Why not? because, anytime a signature is needed on something,
    the relevant OT API call will require you to pass in the Nym, and the API
    already
    signs internally wherever it deems appropriate. Thus, this function is only
    for
    advanced uses, for OT-Scripts, server operators, etc.

    */
    EXPORT std::string SignContract(
        const std::string& SIGNER_NYM_ID,
        const std::string& THE_CONTRACT) const;

    // Instead of signing an existing contract, this is for just signing a flat
    // message.
    // Or, for example, for signing a new contract that has no signature yet.
    // Let's say you
    // have a ledger, for example, with no signatures yet. Pass "LEDGER" as the
    // CONTRACT_TYPE
    // and the resulting output will start like this: -----BEGIN OT SIGNED
    // LEDGER----- ...
    // Returns the signed output, or nullptr.
    //
    EXPORT std::string FlatSign(
        const std::string& SIGNER_NYM_ID,
        const std::string& THE_INPUT,
        const std::string& CONTRACT_TYPE) const;

    /** OT-Sign a CONTRACT. (Add a signature)

    EXPORT std::string AddSignature(const std::string& SIGNER_NYM_ID,
    const std::string& THE_CONTRACT);

    Tries to instantiate the contract object, based on the string passed in.
    Signs the contract, without releasing any signatures that are already there.
    Returns the signed contract, or nullptr if failure.

    NOTE: The actual OT functionality (Use Cases) NEVER requires you to sign via
    this function. Why not? because, anytime a signature is needed on something,
    the relevant OT API call will require you to pass in the Nym, and the API
    already
    signs internally wherever it deems appropriate. Thus, this function is only
    for
    advanced uses, for OT-Scripts, server operators, etc.

    */
    EXPORT std::string AddSignature(
        const std::string& SIGNER_NYM_ID,
        const std::string& THE_CONTRACT) const;

    /** OT-Verify the signature on a CONTRACT.

    Returns OT_BOOL -- OT_TRUE (1) or OT_FALSE (0)

    */
    EXPORT bool VerifySignature(
        const std::string& SIGNER_NYM_ID,
        const std::string& THE_CONTRACT) const;

    /** Verify and Retrieve XML Contents.
    //
    // Pass in a contract and a user ID, and this function will:
    // -- Load the contract up and verify it. (Most classes in OT
    // are derived in some way from Contract.)
    // -- Verify the user's signature on it.
    // -- Remove the PGP-style bookends (the signatures, etc)
    // and return the XML contents of the contract in string form. <==
    */
    EXPORT std::string VerifyAndRetrieveXMLContents(
        const std::string& THE_CONTRACT,
        const std::string& SIGNER_ID) const;

    /** CREATE NYM -- Create new User
    //
    // Creates a new Nym and adds it to the wallet.
    // (Including PUBLIC and PRIVATE KEYS.)
    //
    // Returns a new Nym ID (with files already created)
    // or nullptr upon failure.
    //
    // Once it exists, use registerNym() to
    // register your new Nym at any given Server. (Nearly all
    // server requests require this...)
    //
    // nKeySize must be 1024, 2048, 4096, or 8192.
    // NYM_ID_SOURCE can be empty (it will just generate a keypair
    // and use the public key as the source.) Otherwise you can pass
    // another source string in here, such as a URL, but the Nym will
    // not verify against its own source unless the credential IDs for
    // that Nym can be found posted at that same URL. Whereas if the
    // source is just a public key, then the only verification requirement
    // is that master credentials be signed by the corresponding private key.
    */
    EXPORT std::string CreateNymLegacy(
        const std::int32_t& nKeySize,
        const std::string& NYM_ID_SOURCE) const;

    /** Create a nym using HD key derivation
     *
     *  All keys associated with nyms created via this method can be recovered
     *  via the wallet seed (12/24 words).
     *
     *  \param[in] seed (optional)  Specify a custom HD seed fingerprint. If
     *                              blank or not found, the default wallet seed
     *                              will be used.
     *  \param[in] index (optional) Derivation path of the nym to be created. A
     *                              negative value will use the next index for
     *                              the specified seed.
     *  \returns nym id for the new nym on success, or an empty string
     */
    EXPORT std::string CreateNymHD(
        const proto::ContactItemType type,
        const std::string& name,
        const std::string& fingerprint = "",
        const std::int32_t index = -1) const;

    EXPORT std::string GetNym_ActiveCronItemIDs(
        const std::string& NYM_ID,
        const std::string& NOTARY_ID) const;
    EXPORT std::string GetActiveCronItem(
        const std::string& NOTARY_ID,
        std::int64_t lTransNum) const;

    EXPORT std::string GetNym_SourceForID(const std::string& NYM_ID) const;
    EXPORT std::string GetNym_Description(const std::string& NYM_ID) const;

    EXPORT std::string GetNym_MasterCredentialContents(
        const std::string& NYM_ID,
        const std::string& CREDENTIAL_ID) const;

    EXPORT std::string GetNym_RevokedCredContents(
        const std::string& NYM_ID,
        const std::string& CREDENTIAL_ID) const;

    EXPORT std::string GetNym_ChildCredentialContents(
        const std::string& NYM_ID,
        const std::string& MASTER_CRED_ID,
        const std::string& SUB_CRED_ID) const;

    EXPORT bool RevokeChildCredential(
        const std::string& NYM_ID,
        const std::string& MASTER_CRED_ID,
        const std::string& SUB_CRED_ID) const;

    /**   Obtain the set of contact data associated with the target nym
     *    \param[in]  nymID the indentifier of the target nym
     *    \return std::string containing serialized ContactData protobuf
     *    \note This function returns binary data, not text
     */
    EXPORT std::string GetContactData(const std::string& nymID) const;
    // Identical to the above function, except it Base64-encodes the return
    // value.
    EXPORT std::string GetContactData_Base64(const std::string& nymID) const;
    /**   Obtain human-readable summary of contact data associated with the
     *    target nym
     *    \param[in]  nymID the indentifier of the target nym
     */
    EXPORT std::string DumpContactData(const std::string& nymID) const;

    /**   Replace the target nym's contact data with a new set
     *    \param[in]  nymID the indentifier of the target nym
     *    \param[in]  data ASCII-armored serialized ContactData protobuf
     *    \return true for success, false for error
     *    \warning All existing contact credentials will be revoked, and
     *             replaced with the supplied data.
     */
    EXPORT bool SetContactData(
        const std::string& nymID,
        const std::string& data) const;
    EXPORT bool SetContactData_Base64(
        const std::string& nymID,
        const std::string& data) const;

    /**   Add a single claim to the target nym's contact credential
     *    \param[in]  nymID the indentifier of the target nym
     *    \param[in]  section section containing the claim
     *    \param[in]  claim serialized ContactItem protobuf
     *    \return true for success, false for error
     */
    EXPORT bool SetClaim(
        const std::string& nymID,
        const std::uint32_t& section,
        const std::string& claim) const;
    EXPORT bool SetClaim_Base64(
        const std::string& nymID,
        const std::uint32_t& section,
        const std::string& claim) const;

    /**   Add a single claim to the target nym's contact credential
     *    \param[in]  nymID the indentifier of the target nym
     *    \param[in]  section section containing the claim
     *    \param[in]  type claim type (proto::ContactItemType enum value)
     *    \param[in]  value claim value
     *    \param[in]  active true if the claim should have an active attribute
     *    \param[in]  primary true if the claim should have a primary attribute
     *    \param[in]  start beginning of valid time for the claim
     *    \param[in]  end end of valid time for the claim
     *    \return true for success, false for error
     */
    EXPORT bool AddClaim(
        const std::string& nymID,
        const std::uint32_t& section,
        const std::uint32_t& type,
        const std::string& value,
        const bool active = true,
        const bool primary = false,
        const std::int64_t start = 0,
        const std::int64_t end = 0) const;

    /**   Remove a single claim from the target nym's contact credential
     *    \param[in]  nymID the indentifier of the target nym
     *    \param[in]  claimID the indentifier of the target claim
     *    \return true for success, false for error
     */
    EXPORT bool DeleteClaim(
        const std::string& nymID,
        const std::string& claimID) const;

    /**  Obtain the set of claim verifications associated with the target nym
     *    \param[in]  nymID the indentifier of the target nym
     *    \return std::string containing serialized VerificationSet protobuf
     *    \note This function returns binary data, not text
     */
    EXPORT std::string GetVerificationSet(const std::string& nymID) const;
    EXPORT std::string GetVerificationSet_Base64(
        const std::string& nymID) const;

    /**   Add a single verification to the target nym's verification credential
     *    \param[out] changed set to true if the verification is added
     *    \param[in]  onNym the indentifier of the target nym
     *    \param[in]  claimantNymID the nym whose claim is being verified
     *    \param[in]  claimID the identifier of the claim being verified
     *    \param[in]  polarity type of verification: positive, neutral, negative
     *    \param[in]  start beginning of the validation interval. defaults to 0
     *    \param[in]  end end of the validation interval. defaults to 0
     *    \return std::string containing serialized VerificationSet protobuf
     *    \note This function returns binary data, not text
     */
    EXPORT std::string SetVerification(
        bool& changed,
        const std::string& onNym,
        const std::string& claimantNymID,
        const std::string& claimID,
        const ClaimPolarity polarity,
        const std::int64_t start = 0,
        const std::int64_t end = 0) const;
    EXPORT std::string SetVerification_Base64(
        bool& changed,
        const std::string& onNym,
        const std::string& claimantNymID,
        const std::string& claimID,
        const ClaimPolarity polarity,
        const std::int64_t start = 0,
        const std::int64_t end = 0) const;

    /**  Translate an claim attribute enum value to human-readable text
     *    \param[in]  type claim attribute enum value
     *    \param[in]  lang two letter code for the language to use for the
     *                     translation
     *    \return translated attribute name
     */
    EXPORT std::string ContactAttributeName(
        const proto::ContactItemAttribute type,
        std::string lang = "en") const;

    /**  Get a list of allowed section types for contact data protobufs of the
     *   specified version
     *    \param[in]  version version of the contact data protobuf to query
     *    \return list of allowed section types
     */
    EXPORT std::set<proto::ContactSectionName> ContactSectionList(
        const std::uint32_t version = 1) const;

    /**  Translate a claim section name enum value to human-readable text
     *    \param[in]  section claim section name enum value
     *    \param[in]  lang two letter code for the language to use for the
     *                     translation
     *    \return translated claim section
     */
    EXPORT std::string ContactSectionName(
        const proto::ContactSectionName section,
        std::string lang = "en") const;

    /**  Get a list of allowed claim types for sections of the specified version
     *    \param[in]  section section name
     *    \param[in]  version version of the specified section name
     *    \return list of allowed claim types
     */
    EXPORT std::set<proto::ContactItemType> ContactSectionTypeList(
        const proto::ContactSectionName section,
        const std::uint32_t version = 1) const;

    /**  Translate a claim type enum value to human-readable text
     *    \param[in]  section claim type enum value
     *    \param[in]  lang two letter code for the language to use for the
     *                     translation
     *    \return translated claim type
     */
    EXPORT std::string ContactTypeName(
        const proto::ContactItemType type,
        std::string lang = "en") const;

    /**  Find the relationship type which acts as the inverse of the given value
     *    \param[in]  relationship claim type enum value for the relationship to
     *                             be reversed
     *    \return claim type enum value for the reciprocal relationship, or
     *            proto::CITEMTYPE_ERROR
     */
    EXPORT proto::ContactItemType ReciprocalRelationship(
        const proto::ContactItemType relationship) const;

    /** Creates a contract based on the contents passed in,
    // then sets the contract key based on the NymID,
    // and signs it with that Nym.
    // This function will also ADD the contract to the wallet.
    // Returns: the new contract ID, or nullptr if failure.
    */
    EXPORT std::string CreateCurrencyContract(
        const std::string& NYM_ID,
        const std::string& shortname,
        const std::string& terms,
        const std::string& name,
        const std::string& symbol,
        const std::string& tla,
        const std::uint32_t power,
        const std::string& fraction) const;
    EXPORT std::string CreateSecurityContract(
        const std::string& NYM_ID,
        const std::string& shortname,
        const std::string& terms,
        const std::string& name,
        const std::string& symbol) const;

    EXPORT std::string CalculateContractID(
        const std::string& str_Contract) const;

    EXPORT std::string GetSignerNymID(const std::string& str_Contract) const;

    // Use these below functions to get the new contract ITSELF, using its ID
    // that was returned by the above two functions:
    //
    // EXPORT std::string GetServer_Contract(const std::string& NOTARY_ID); //
    // Return's Server's contract (based on server ID)
    // EXPORT std::string GetAssetType_Contract(const std::string&
    // INSTRUMENT_DEFINITION_ID); // Returns currency contract based on Asset
    // Type ID

    /*
    ---------------------------------

    ASSET CONTRACTS will want to make sure they put something like this at the
    top:

    <instrumentDefinition version="1.0">

    <entity shortname="Just testing"
    longname="I need user feedback to help design the contract tags the server
    must support..."
    email="F3llowTraveler (at) gmail.com"/>

    <issue company="Chuck-E-Cheese, Inc"
    email="games@chuckecheese.com"
    contractUrl="https://chuckecheese.com/games/contract/"
    type="currency"/>

    <currency name="game tokens" tla="TOKEN" symbol="tks" type="decimal"
    factor="1" decimalPower="0" fraction="mg"/>

    ---------------------------------

    SERVER CONTRACTS will want to make sure they put something like this at the
    top:

    <notaryProviderContract version="1.0">

    <entity shortname="Transactions.com"
    longname="Transactions.com, LLC"
    email="serverfarm@cloudcomputing.com"
    serverURL="https://transactions.com/vers/1/"/>

    <notaryServer hostname="localhost"
    port="7085"
    URL="https://transactions.com/vers/1/" />

    ---------------------------------


    NEITHER has to worry about THIS, which is added automatically:


    <key name="contract">
    - -----BEGIN CERTIFICATE-----
    MIICZjCCAc+gAwIBAgIJAO14L19TJgzcMA0GCSqGSIb3DQEBBQUAMFcxCzAJBgNV
    BAYTAlVTMREwDwYDVQQIEwhWaXJnaW5pYTEQMA4GA1UEBxMHRmFpcmZheDERMA8G
    A1UEChMIWm9yay5vcmcxEDAOBgNVBAMTB1Jvb3QgQ0EwHhcNMTAwOTI5MDUyMzAx
    WhcNMjAwOTI2MDUyMzAxWjBeMQswCQYDVQQGEwJVUzERMA8GA1UECBMIVmlyZ2lu
    aWExEDAOBgNVBAcTB0ZhaXJmYXgxETAPBgNVBAoTCFpvcmsub3JnMRcwFQYDVQQD
    Ew5zaGVsbC56b3JrLm9yZzCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA3vD9
    fO4ov4854L8wXrgfv2tltDz0ieVrTNSLuy1xuQyb//+MwZ0EYwu8jMMQrqbUaYG6
    y8zJu32yBKrBNPPwJ+fJE+tfgVg860dGVbwMd4KhpkKtppJXmZaGqLqvELaXa4Uw
    9N3qg/faj0NMEDIBhv/tD/B5U65vH+U0JlRJ07kCAwEAAaMzMDEwCQYDVR0TBAIw
    ADAkBgNVHREEHTAbgg5zaGVsbC56b3JrLm9yZ4IJbG9jYWxob3N0MA0GCSqGSIb3
    DQEBBQUAA4GBALLXPa/naWsiXsw0JwlSiG7aOmvMF2romUkcr6uObhN7sghd38M0
    l2kKTiptnA8txrri8RhqmQgOgiyKFCKBkxY7/XGot62cE8Y1+lqGXlhu2UHm6NjA
    pRKvng75J2HTjmmsbCHy+nexn4t44wssfPYlGPD8sGwmO24u9tRfdzJE
    - -----END CERTIFICATE-----
    </key>

    */

    /*
    EXPORT std::string Contract_AddTag(const std::string& NYM_ID, const
    std::string& THE_CONTRACT,
    const std::string& TAG_NAME, const std::string& SUBTAG_NAME,
    const std::string& SUBTAG_VALUE, const std::string& TAG_VALUE);
    key == TAG_NAME
    name == SUBTAG_NAME
    "contract" == SUBTAG_VALUE
    <the cert> == TAG_VALUE

    */

    /** --------------------------------------------------
    // ADD SERVER CONTRACT (to wallet)
    //
    // If you have a server contract that you'd like to add
    // to your wallet, call this function.
    */
    EXPORT std::string AddServerContract(const std::string& strContract) const;

    /** --------------------------------------------------
    // ADD ASSET CONTRACT (to wallet)
    //
    // If you have an asset contract that you'd like to add
    // to your wallet, call this function.
    */
    EXPORT std::string AddUnitDefinition(const std::string& strContract) const;

    /** --------------------------------------------------

    // NOTE: THE BELOW FUNCTIONS *DO NOT* SEND ANY MESSAGE TO THE SERVER,
    // BUT RATHER, THEY ONLY QUERY FOR DATA FROM LOCAL STORAGE.
    //
    // Often a server response will cause a change to the data in local storage.
    // These functions allow you to re-load that data so your GUI can reflect
    // the updates to those files.
    */
    EXPORT std::int32_t GetServerCount() const;
    EXPORT std::int32_t GetAssetTypeCount() const;
    EXPORT std::int32_t GetNymCount() const;

    EXPORT[[deprecated]] std::string GetServer_ID(
        const std::int32_t& nIndex) const;
    EXPORT std::string GetServer_Name(const std::string& NOTARY_ID) const;
    EXPORT std::string GetServer_Contract(const std::string& NOTARY_ID) const;

    std::int32_t GetCurrencyDecimalPower(
        const std::string& INSTRUMENT_DEFINITION_ID) const;
    std::string GetCurrencyTLA(
        const std::string& INSTRUMENT_DEFINITION_ID) const;
    std::string GetCurrencySymbol(
        const std::string& INSTRUMENT_DEFINITION_ID) const;

    /** FormatAmount:
    // Input: currency contract, amount. (And locale, internally.)
    // Output: 545 becomes (for example) "$5.45"
    //
    // Returns formatted string for output, for a given amount, based on
    // currency contract and locale.
    */
    EXPORT std::string FormatAmount(
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::int64_t& THE_AMOUNT) const;
    EXPORT std::string FormatAmountLocale(
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::int64_t& THE_AMOUNT,
        const std::string& THOUSANDS_SEP,
        const std::string& DECIMAL_POINT) const;

    /** FormatAmountWithoutSymbol:
    // Input: currency contract, amount. (And locale, internally.)
    // Output: 545 becomes (for example) "5.45"
    //
    // Returns formatted string for output, for a given amount, based on
    // currency contract and locale.
    */
    EXPORT std::string FormatAmountWithoutSymbol(
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::int64_t& THE_AMOUNT) const;
    EXPORT std::string FormatAmountWithoutSymbolLocale(
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::int64_t& THE_AMOUNT,
        const std::string& THOUSANDS_SEP,
        const std::string& DECIMAL_POINT) const;

    /** StringToAmount:
    // Input: currency contract, formatted string. (And locale, internally.)
    // Output: "$5.45" becomes 545 (for example.)
    //
    // Returns amount from formatted string, based on currency contract and
    // locale.
    */
    EXPORT std::int64_t StringToAmount(
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& str_input) const;
    EXPORT std::int64_t StringToAmountLocale(
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& str_input,
        const std::string& THOUSANDS_SEP,
        const std::string& DECIMAL_POINT) const;

    /** GetAssetType_ID:
     Returns Asset Type ID based on index from GetAssetTypeCount.
     */
    EXPORT[[deprecated]] std::string GetAssetType_ID(
        const std::int32_t& nIndex) const;

    /** GetAssetType_Name:
     Returns asset type name based on Instrument Definition ID.
     */
    EXPORT std::string GetAssetType_Name(
        const std::string& INSTRUMENT_DEFINITION_ID) const;

    EXPORT std::string GetAssetType_TLA(const std::string& THE_ID) const;

    /** GetAssetType_Contract:
     Returns currency contract based on Instrument Definition ID.
     */
    EXPORT std::string GetAssetType_Contract(
        const std::string& INSTRUMENT_DEFINITION_ID) const;

    /** You already have accounts in your wallet (without any server
    communications)
    // and these functions allow you to query the data members of those
    accounts.
    // Thus, "AccountWallet" denotes that you are examining copies of your
    accounts that
    // are sitting in your wallet. Originally the purpose was to eliminate
    confusion with
    // a different set of similarly-named functions.
    */
    EXPORT std::string GetAccountWallet_Name(
        const std::string& ACCOUNT_ID) const;  // returns the account name,
                                               // based on account ID.
    EXPORT std::int64_t GetAccountWallet_Balance(
        const std::string& ACCOUNT_ID) const;  // returns the account balance,
                                               // based on
                                               // account ID.
    EXPORT std::string GetAccountWallet_Type(
        const std::string& ACCOUNT_ID) const;  // returns the account type
                                               // (simple,
                                               // issuer, etc)
    EXPORT std::string GetAccountWallet_InstrumentDefinitionID(
        const std::string& ACCOUNT_ID) const;  // returns instrument definition
                                               // ID of the
                                               // account
    EXPORT std::string GetAccountWallet_NotaryID(
        const std::string& ACCOUNT_ID) const;  // returns Notary ID of the
                                               // account
    EXPORT std::string GetAccountWallet_NymID(
        const std::string& ACCOUNT_ID) const;  // returns Nym ID of the account

    /** Returns OT_BOOL. Verifies any asset account (intermediary files) against
    its own last signed receipt.
    // Obviously this will fail for any new account that hasn't done any
    transactions yet, and thus has no receipts.
    */
    EXPORT bool VerifyAccountReceipt(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCT_ID) const;

    /**----------------------------------------------------------
    // GET NYM TRANSACTION NUM COUNT
    // How many transaction numbers does the Nym have (for a given server?)
    //
    // This function returns the count of numbers available. If 0, then no
    // transactions will work until you call getTransactionNumbers()
    // to replenish your Nym's supply for that NotaryID...
    //
    // Returns a count (0 through N numbers available),
    // or -1 for error (no nym found.)
    */
    EXPORT std::int32_t GetNym_TransactionNumCount(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID) const;

    EXPORT std::string GetNym_ID(const std::int32_t& nIndex) const;  // based
                                                                     // on
    // Index (above
    // 4 functions)
    // this returns
    // the Nym's ID
    EXPORT std::string GetNym_Name(const std::string& NYM_ID) const;  // Returns
    // Nym Name
    // (based
    // on
    // NymID)
    EXPORT std::string GetNym_Stats(const std::string& NYM_ID)
        const;  // Returns Nym Statistics (based on NymID)
    EXPORT std::string GetNym_NymboxHash(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID) const;  // NymboxHash for "most recently
                                           // DOWNLOADED"
                                           // Nymbox (by NotaryID)
    EXPORT std::string GetNym_RecentHash(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID) const;  // "Most recent NymboxHash according
                                           // to the
    // SERVER's records" (Which is often sent as
    // an 'FYI' with various server replies to
    // my messages.)

    EXPORT std::string GetNym_InboxHash(
        const std::string& ACCOUNT_ID,
        const std::string& NYM_ID) const;  // InboxHash for "most recently
                                           // DOWNLOADED"
    // Inbox (by AccountID). Often contains
    // older value than
    // GetAccountWallet_InboxHash.
    EXPORT std::string GetNym_OutboxHash(
        const std::string& ACCOUNT_ID,
        const std::string& NYM_ID) const;  // OutboxHash for "most recently
                                           // DOWNLOADED"
    // Outbox (by AccountID) Often contains
    // older value than
    // GetAccountWallet_OutboxHash

    EXPORT bool IsNym_RegisteredAtServer(
        const std::string& NYM_ID,
        const std::string& NOTARY_ID) const;

    /** Each Nym has mail messages, they can come from different servers.
    // This allows you to peruse the mail for a given Nym, and erase messages.
    //


    So how would you actually USE this to browse a Nym's mail?

    -- Call GetNym_MailCount() to find out how many mail items there are.

    -- Then LOOP through them, and use GetNym_MailSenderIDByIndex and
    GetNym_MailNotaryIDByIndex to populate the list.

    -- If you want to add a subject display, you'll have to call
    GetNym_MailContentsByIndex()
    and check for a first line beginning in Subject: (there may not be one.)

    -- GetNym_MailContentsByIndex returns the body of the mail regardless.

    -- Use Nym_VerifyMailByIndex() to verify the signature on the mail,

    -- and use Nym_RemoveMailByIndex() to erase mail (when you want to.)

    */

    EXPORT std::list<std::string> GetNym_MailThreads(
        const std::string& NYM_ID,
        const bool unreadOnly) const;

    EXPORT std::list<std::string> GetNym_MailCount(
        const std::string& NYM_ID) const;
    EXPORT std::string GetNym_MailContentsByIndex(
        const std::string& NYM_ID,
        const std::string& nIndex) const;
    EXPORT std::string GetNym_MailSenderIDByIndex(
        const std::string& NYM_ID,
        const std::string& nIndex) const;
    EXPORT std::string GetNym_MailNotaryIDByIndex(
        const std::string& NYM_ID,
        const std::string& nIndex) const;
    EXPORT bool Nym_RemoveMailByIndex(
        const std::string& NYM_ID,
        const std::string& nIndex) const;
    EXPORT bool Nym_VerifyMailByIndex(
        const std::string& NYM_ID,
        const std::string& nIndex) const;

    EXPORT std::list<std::string> GetNym_OutmailCount(
        const std::string& NYM_ID) const;
    EXPORT std::string GetNym_OutmailContentsByIndex(
        const std::string& NYM_ID,
        const std::string& nIndex) const;
    EXPORT std::string GetNym_OutmailRecipientIDByIndex(
        const std::string& NYM_ID,
        const std::string& nIndex) const;
    EXPORT std::string GetNym_OutmailNotaryIDByIndex(
        const std::string& NYM_ID,
        const std::string& nIndex) const;
    EXPORT bool Nym_RemoveOutmailByIndex(
        const std::string& NYM_ID,
        const std::string& nIndex) const;
    EXPORT bool Nym_VerifyOutmailByIndex(
        const std::string& NYM_ID,
        const std::string& nIndex) const;

    EXPORT std::int32_t GetNym_OutpaymentsCount(
        const std::string& NYM_ID) const;

    EXPORT std::string GetNym_OutpaymentsContentsByIndex(
        const std::string& NYM_ID,
        const std::int32_t& nIndex) const;  // returns the message itself

    EXPORT std::string GetNym_OutpaymentsRecipientIDByIndex(
        const std::string& NYM_ID,
        const std::int32_t& nIndex) const;  // returns the NymID of the
                                            // recipient.
    EXPORT std::string GetNym_OutpaymentsNotaryIDByIndex(
        const std::string& NYM_ID,
        const std::int32_t& nIndex) const;  // returns the
    // NotaryID where the
    // message came from.

    EXPORT bool Nym_RemoveOutpaymentsByIndex(
        const std::string& NYM_ID,
        const std::int32_t& nIndex) const;  // (1 or 0.)
    EXPORT bool Nym_VerifyOutpaymentsByIndex(
        const std::string& NYM_ID,
        const std::int32_t& nIndex) const;  // true if signature verifies.
                                            // (Sender
                                            // Nym MUST
    // be in my wallet for this to work.)

    /**---------------------------------------------------------

    // *** FUNCTIONS FOR REMOVING VARIOUS CONTRACTS AND NYMS FROM THE WALLET ***

    // Can I remove this server contract from my wallet?
    //
    // You cannot remove the server contract from your wallet if there are
    accounts in there using it.
    // This function tells you whether you can remove the server contract or
    not. (Whether there are accounts...)
    // returns OT_BOOL
    */
    EXPORT bool Wallet_CanRemoveServer(const std::string& NOTARY_ID) const;

    /** Remove this server contract from my wallet!
    //
    // Try to remove the server contract from the wallet.
    // This will not work if there are any accounts in the wallet for the same
    server ID.
    // returns OT_BOOL
    */
    EXPORT bool Wallet_RemoveServer(const std::string& NOTARY_ID) const;

    /** Can I remove this asset contract from my wallet?
    //
    // You cannot remove the asset contract from your wallet if there are
    accounts in there using it.
    // This function tells you whether you can remove the asset contract or not.
    (Whether there are accounts...)
    // returns OT_BOOL
    */
    EXPORT bool Wallet_CanRemoveAssetType(
        const std::string& INSTRUMENT_DEFINITION_ID) const;

    /** Remove this asset contract from my wallet!
    //
    // Try to remove the asset contract from the wallet.
    // This will not work if there are any accounts in the wallet for the same
    instrument definition ID.
    // returns OT_BOOL
    */
    EXPORT bool Wallet_RemoveAssetType(
        const std::string& INSTRUMENT_DEFINITION_ID) const;

    /** Can I remove this Nym from my wallet?
    //
    // You cannot remove the Nym from your wallet if there are accounts in there
    using it.
    // This function tells you whether you can remove the Nym or not. (Whether
    there are accounts...)
    // returns OT_BOOL
    */
    EXPORT bool Wallet_CanRemoveNym(const std::string& NYM_ID) const;

    /** Remove this Nym from my wallet!
    //
    // Try to remove the Nym from the wallet.
    // This will not work if there are any nyms in the wallet for the same
    server ID.
    // returns OT_BOOL
    */
    EXPORT bool Wallet_RemoveNym(const std::string& NYM_ID) const;

    /** Can I remove this Account from my wallet?
    //
    // You cannot remove the Account from your wallet if there are transactions
    still open.
    // This function tells you whether you can remove the Account or not.
    (Whether there are transactions...)
    // returns OT_BOOL
    */
    EXPORT bool Wallet_CanRemoveAccount(const std::string& ACCOUNT_ID) const;

    // See deleteAssetAccount(), a server message, for deleting asset accounts.
    // (You can't just delete them out of the wallet without first deleting them
    // off of the server.)
    //

    /**
    CHANGE MASTER KEY and PASSWORD.

    Normally your passphrase is used to derive a key, which is used to unlock
    a random number (a symmetric key), which is used as the passphrase to open
    the
    master key, which is used as the passphrase to any given Nym.

    Since all the Nyms are encrypted to the master key, and since we can change
    the
    passphrase on the master key without changing the master key itself, then we
    don't
    have to do anything to update all the Nyms, since that part hasn't changed.

    But we might want a separate "Change Master Key" function that replaces that
    key
    itself, in which case we'd HAVE to load up all the Nyms and re-save them.

    UPDATE: Seems the easiest thing to do is to just change both the key and
    passphase
    at the same time here, by loading up all the private nyms, destroying the
    master key,
    and then saving all the private Nyms. (With master key never actually being
    "paused.")
    This will automatically cause it to generate a new master key during the
    saving process.
    (Make sure to save the wallet also.)
    */
    EXPORT bool Wallet_ChangePassphrase() const;  //  (true for success
                                                  // and
                                                  // false for error.)

    //! Returns the exported Nym, if success. (Else nullptr.)
    EXPORT std::string Wallet_ExportNym(const std::string& NYM_ID) const;

    //! returns NymID if success, else nullptr.
    EXPORT std::string Wallet_ImportNym(const std::string& FILE_CONTENTS) const;

    //! Attempts to find a full ID in the wallet, based on a partial of the same
    // ID.
    //! Returns nullptr on failure, otherwise returns the full ID.
    //
    EXPORT std::string Wallet_GetNymIDFromPartial(
        const std::string& PARTIAL_ID) const;
    EXPORT std::string Wallet_GetNotaryIDFromPartial(
        const std::string& PARTIAL_ID) const;
    EXPORT std::string Wallet_GetInstrumentDefinitionIDFromPartial(
        const std::string& PARTIAL_ID) const;
    EXPORT std::string Wallet_GetAccountIDFromPartial(
        const std::string& PARTIAL_ID) const;

    /** -----------------------------------
    // SET NYM NAME
    //
    // You might have 40 of your friends' public nyms in
    // your wallet. You might have labels on each of them.
    // But whenever you change a label (and thus re-sign the
    // file for that Nym when you save it), you only SIGN
    // using one of your OWN nyms, for which you have a private
    // key available for signing.
    //
    // Signer Nym?
    // When testing, there is only one nym, so you just pass it
    // twice. But in real production, a user will have a default
    // signing nym, the same way that he might have a default
    // signing key in PGP, and that must be passed in whenever
    // he changes the name on any of the other nyms in his wallet.
    // (In order to properly sign and save the change.)
    //
    // Returns OT_TRUE (1) or OT_FALSE (0)
    */
    EXPORT bool SetNym_Alias(
        const std::string& targetNymID,
        const std::string& walletNymID,
        const std::string& name) const;

    EXPORT bool Rename_Nym(
        const std::string& nymID,
        const std::string& name,
        const proto::ContactItemType type = proto::CITEMTYPE_ERROR,
        const bool primary = true) const;

    //! Returns OT_TRUE (1) or OT_FALSE (0)
    //! The asset account's name is merely a client-side label.
    EXPORT bool SetAccountWallet_Name(
        const std::string& ACCT_ID,
        const std::string& SIGNER_NYM_ID,
        const std::string& ACCT_NEW_NAME) const;

    EXPORT bool SetAssetType_Name(
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& STR_NEW_NAME) const;

    EXPORT bool SetServer_Name(
        const std::string& NOTARY_ID,
        const std::string& STR_NEW_NAME) const;

    /** (Above) IMPORTANT: USE the above functions for setting the CLIENT-SIDE
    // display labels that you use in your UI for the
    Nyms/Servers/AssetTypes/Accounts.
    // These labels are stored SEPARATELY from their own files, in the wallet
    file.
    //
    // If you just added the contract, it will SET the label for you based on
    the contract type.
    // like if it's an asset contract, it uses the currency name field from the
    asset contract.
    // If it's a server contract it uses the entity short name. After that, it's
    // configurable for the user, and stays on client side, and persists via
    wallet.
    //
    // EVEN WHEN OT has to re-download one of those files, it will make sure to
    save
    // the display label properly in the wallet.
    //
    // THIS MEANS *you*, as a client developer:
    // 1) CAN DEPEND on these labels to have the right value.
    // 2) Can expect them to start out with good default values.
    // 3) Can configure them from there.
    // 4) SHOULD DISPLAY THEM as appropriate in your application.
    // 5) Of course, use the ID behind the scenes for making all your
    // OT calls... just use the name for display purposes.
    */

    /**

    WRITE A CHEQUE --- (Returns the cheque in string form.)

    ==> WriteCheque() internally constructs an OTCheque
    and issues it, like so:

    OTCheque theCheque( NOTARY_ID, INSTRUMENT_DEFINITION_ID );

    theCheque.IssueCheque( AMOUNT // The amount of the cheque, in string form,
    which OTAPI
    // will convert to a std::int64_t integer. Negative amounts
    // allowed, since that is how OT implements invoices.
    // (An invoice is just a cheque with a negative amount.)

    lTransactionNumber, // The API will supply this automatically, as long as
    // there are some transaction numbers in the wallet. (Call
    // getTransactionNumbers() if your wallet needs more.)

    VALID_FROM, VALID_TO, // Valid date range (in seconds since Jan 1970...)

    ACCOUNT_ID, NYM_ID, // Nym ID and Acct ID for SENDER.

    CHEQUE_MEMO, // The memo for the cheque. (Can be empty or be nullptr.)

    RECIPIENT_NYM_ID); // Recipient Nym ID is optional. (You can use an
    // empty string here, to write a blank cheque, or pass nullptr.)
    */
    EXPORT std::string WriteCheque(
        const std::string& NOTARY_ID,
        const std::int64_t& CHEQUE_AMOUNT,
        const time64_t& VALID_FROM,
        const time64_t& VALID_TO,
        const std::string& SENDER_ACCT_ID,
        const std::string& SENDER_NYM_ID,
        const std::string& CHEQUE_MEMO,
        const std::string& RECIPIENT_NYM_ID) const;

    /**
    DISCARD A CHEQUE -- returns OT_TRUE or OT_FALSE

    When you write a cheque, your wallet software must use one of your
    transaction
    numbers on the cheque. Even when you give the cheque to someone, a good
    wallet
    should still store a copy of the cheque, until it is cashed.
    This way, if you decide to "tear it up" (perhaps the recipient left the
    country!)
    then you will still have a copy of the cheque, and you can get your
    transaction
    number back before discarding it.

    Why does it matter to get the transaction number back? Because once you sign
    that number out, you will have to CONTINUE signing it out on EVERY RECEIPT
    until
    that transaction is finally closed out. You are still responsible for that
    transaction number, until then.

    So most likely, if the cheque is going nowhere, then you will want to
    RETRIEVE
    the transaction number back from the cheque so you can use it on another
    transaction
    instead. *THIS* IS THE FUNCTION YOU SHOULD USE TO DO THAT!!!!

    //Returns OT_BOOL
    */
    EXPORT bool DiscardCheque(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCT_ID,
        const std::string& THE_CHEQUE) const;

    /**

    PROPOSE PAYMENT PLAN --- Returns the payment plan in string form.

    (Called by Merchant.)

    PARAMETER NOTES:
    -- Payment Plan Delay, and Payment Plan Period, both default to 30 days (if
    you pass 0.)

    -- Payment Plan Length, and Payment Plan Max Payments, both default to 0,
    which means
    no maximum length and no maximum number of payments.

    -----------------------------------------------------------------
    FYI, the payment plan creation process (finally) is:

    1) Payment plan is written, and signed, by the recipient. (This function:
    ProposePaymentPlan)
    2) He sends it to the sender, who signs it and submits it.
    (ConfirmPaymentPlan and depositPaymentPlan)
    3) The server loads the recipient nym to verify the transaction
    number. The sender also had to burn a transaction number (to
    submit it) so now, both have verified trns#s in this way.

    ----------------------------------------------------------------------------------------

    FYI, here are all the OT library calls that are performed by this single API
    call:

    OTPaymentPlan * pPlan = new OTPaymentPlan(pAccount->GetRealNotaryID(),
    pAccount->GetInstrumentDefinitionID(),
    pAccount->GetRealAccountID(),    pAccount->GetNymID(),
    RECIPIENT_ACCT_ID, RECIPIENT_NYM_ID);

    ----------------------------------------------------------------------------------------
    From OTAgreement: (This must be called first, before the other two methods
    below can be called.)

    bool    OTAgreement::SetProposal(const OTPseudonym& MERCHANT_NYM, const
    OTString& strConsideration,
    const time64_t& VALID_FROM=0, const time64_t& VALID_TO=0);

    ----------------------------------------------------------------------------------------
    (Optional initial payment):
    bool    OTPaymentPlan::SetInitialPayment(const std::int64_t& lAmount,
    time64_t
    tTimeUntilInitialPayment=0); // default: now.
    ----------------------------------------------------------------------------------------

    These two (above and below) can be called independent of each other. You can
    have an initial payment, AND/OR a payment plan.

    ----------------------------------------------------------------------------------------
    (Optional regular payments):
    bool    OTPaymentPlan::SetPaymentPlan(const std::int64_t& lPaymentAmount,
    time64_t tTimeUntilPlanStart  =OT_TIME_MONTH_IN_SECONDS, // Default: 1st
    payment in 30 days
    time64_t tBetweenPayments     =OT_TIME_MONTH_IN_SECONDS, // Default: 30
    days.
    time64_t tPlanLength=0, std::int32_t nMaxPayments=0);
    ----------------------------------------------------------------------------------------
    */
    EXPORT std::string ProposePaymentPlan(
        const std::string& NOTARY_ID,
        const time64_t& VALID_FROM,  // Default (0 or nullptr) == current time
                                     // measured in seconds since Jan 1970.
        const time64_t& VALID_TO,    // Default (0 or nullptr) == no expiry /
                                     // cancel
                                     // anytime. Otherwise this is ADDED to
                                     // VALID_FROM (it's a length.)
        const std::string& SENDER_ACCT_ID,  // Mandatory parameters.
        const std::string& SENDER_NYM_ID,   // Both sender and recipient must
                                            // sign before submitting.
        const std::string& PLAN_CONSIDERATION,  // Like a memo.
        const std::string& RECIPIENT_ACCT_ID,   // NOT optional.
        const std::string& RECIPIENT_NYM_ID,  // Both sender and recipient must
                                              // sign before submitting.
        const std::int64_t& INITIAL_PAYMENT_AMOUNT,  // zero or nullptr == no
                                                     // initial
                                                     // payment.
        const time64_t& INITIAL_PAYMENT_DELAY,    // seconds from creation date.
                                                  // Default is zero or nullptr.
        const std::int64_t& PAYMENT_PLAN_AMOUNT,  // Zero or nullptr == no
                                                  // regular
                                                  // payments.
        const time64_t& PAYMENT_PLAN_DELAY,  // No. of seconds from creation
        // date. Default is zero or nullptr.
        // (Causing 30 days.)
        const time64_t& PAYMENT_PLAN_PERIOD,  // No. of seconds between
                                              // payments.
                                              // Default is zero or nullptr.
                                              // (Causing 30 days.)
        const time64_t& PAYMENT_PLAN_LENGTH,  // In seconds. Defaults to 0 or
                                              // nullptr (no maximum length.)
        const std::int32_t& PAYMENT_PLAN_MAX_PAYMENTS  // integer. Defaults to 0
                                                       // or
        // nullptr (no maximum payments.)
        ) const;

    // The above version has too many arguments for boost::function apparently
    // (for Chaiscript.)
    // So this is a version of it that compresses those into a fewer number of
    // arguments.
    // (Then it expands them and calls the above version.)
    // See above function for more details on parameters.
    // Basically this version has ALL the same parameters, but it stuffs two or
    // three at a time into
    // a single parameter, as a comma-separated list in string form.
    //
    EXPORT std::string EasyProposePlan(
        const std::string& NOTARY_ID,
        const std::string& DATE_RANGE,  // "from,to"  Default 'from' (0 or "")
                                        // ==
                                        // NOW, and default 'to' (0 or "") == no
                                        // expiry / cancel anytime
        const std::string& SENDER_ACCT_ID,  // Mandatory parameters.
        const std::string& SENDER_NYM_ID,   // Both sender and recipient must
                                            // sign before submitting.
        const std::string& PLAN_CONSIDERATION,  // Like a memo.
        const std::string& RECIPIENT_ACCT_ID,   // NOT optional.
        const std::string& RECIPIENT_NYM_ID,  // Both sender and recipient must
                                              // sign before submitting.
        const std::string& INITIAL_PAYMENT,  // "amount,delay"  Default 'amount'
                                             // (0 or "") == no initial payment.
        // Default 'delay' (0 or nullptr) is
        // seconds from creation date.
        const std::string& PAYMENT_PLAN,  // "amount,delay,period" 'amount' is a
                                          // recurring payment. 'delay' and
        // 'period' cause 30 days if you pass 0
        // or "".
        const std::string& PLAN_EXPIRY  // "length,number" 'length' is maximum
                                        // lifetime in seconds. 'number' is
        // maximum number of payments in seconds.
        // 0 or "" is unlimited (for both.)
        ) const;

    // Called by Customer. Pass in the plan obtained in the above call.
    //
    EXPORT std::string ConfirmPaymentPlan(
        const std::string& NOTARY_ID,
        const std::string& SENDER_NYM_ID,
        const std::string& SENDER_ACCT_ID,
        const std::string& RECIPIENT_NYM_ID,
        const std::string& PAYMENT_PLAN) const;

    // SMART CONTRACTS

    // RETURNS: the Smart Contract itself. (Or nullptr.)
    //
    EXPORT std::string Create_SmartContract(
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const time64_t& VALID_FROM,        // Default (0 or nullptr) == NOW
        const time64_t& VALID_TO,  // Default (0 or nullptr) == no expiry /
                                   // cancel anytime
        bool SPECIFY_ASSETS,  // Asset type IDs must be provided for every named
                              // account.
        bool SPECIFY_PARTIES  // Nym IDs must be provided for every party.
        ) const;

    EXPORT std::string SmartContract_SetDates(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // dates changed on it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const time64_t& VALID_FROM,        // Default (0 or nullptr) == NOW
        const time64_t& VALID_TO  // Default (0 or nullptr) == no expiry /
                                  // cancel
                                  // anytime
        ) const;

    EXPORT bool Smart_ArePartiesSpecified(
        const std::string& THE_CONTRACT) const;
    EXPORT bool Smart_AreAssetTypesSpecified(
        const std::string& THE_CONTRACT) const;

    //
    // todo: Someday add a parameter here BYLAW_LANGUAGE so that people can use
    // custom languages in their scripts. For now I have a default language, so
    // I'll just make that the default. (There's only one language right now
    // anyway.)
    //
    // returns: the updated smart contract (or nullptr)
    EXPORT std::string SmartContract_AddBylaw(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // bylaw added to it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME  // The Bylaw's NAME as referenced in the
                                       // smart contract. (And the scripts...)
        ) const;

    // returns: the updated smart contract (or nullptr)
    EXPORT std::string SmartContract_RemoveBylaw(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // bylaw removed from it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME  // The Bylaw's NAME as referenced in the
                                       // smart contract. (And the scripts...)
        ) const;

    // returns: the updated smart contract (or nullptr)
    EXPORT std::string SmartContract_AddClause(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // clause added to it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& CLAUSE_NAME,    // The Clause's name as referenced in
                                           // the smart contract. (And the
                                           // scripts...)
        const std::string& SOURCE_CODE     // The actual source code for the
                                           // clause.
        ) const;

    // returns: the updated smart contract (or nullptr)
    EXPORT std::string SmartContract_UpdateClause(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // clause updated on it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& CLAUSE_NAME,    // The Clause's name as referenced in
                                           // the smart contract. (And the
                                           // scripts...)
        const std::string& SOURCE_CODE     // The actual source code for the
                                           // clause.
        ) const;

    // returns: the updated smart contract (or nullptr)
    EXPORT std::string SmartContract_RemoveClause(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // clause removed from it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& CLAUSE_NAME     // The Clause's name as referenced in
                                           // the smart contract. (And the
                                           // scripts...)
        ) const;

    // returns: the updated smart contract (or nullptr)
    EXPORT std::string SmartContract_AddVariable(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // variable added to it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& VAR_NAME,    // The Variable's name as referenced in
                                        // the
                                        // smart contract. (And the scripts...)
        const std::string& VAR_ACCESS,  // "constant", "persistent", or
                                        // "important".
        const std::string& VAR_TYPE,    // "string", "std::int64_t", or "bool"
        const std::string& VAR_VALUE    // Contains a string. If type is
                                        // std::int64_t,
        // atol() will be used to convert value to
        // a std::int64_t. If type is bool, the strings
        // "true" or "false" are expected here in
        // order to convert to a bool.
        ) const;

    // returns: the updated smart contract (or nullptr)
    EXPORT std::string SmartContract_RemoveVariable(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // variable removed from it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& VAR_NAME  // The Variable's name as referenced in the
                                     // smart contract. (And the scripts...)
        ) const;

    // returns: the updated smart contract (or nullptr)
    EXPORT std::string SmartContract_AddCallback(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // callback added to it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& CALLBACK_NAME,  // The Callback's name as referenced
                                           // in the smart contract. (And the
                                           // scripts...)
        const std::string& CLAUSE_NAME     // The actual clause that will be
                                           // triggered by the callback. (Must
                                           // exist.)
        ) const;

    // returns: the updated smart contract (or nullptr)
    EXPORT std::string SmartContract_RemoveCallback(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // callback removed from it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& CALLBACK_NAME   // The Callback's name as referenced
                                           // in the smart contract. (And the
                                           // scripts...)
        ) const;

    // returns: the updated smart contract (or nullptr)
    EXPORT std::string SmartContract_AddHook(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // hook
                                           // added to it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& HOOK_NAME,   // The Hook's name as referenced in the
                                        // smart contract. (And the scripts...)
        const std::string& CLAUSE_NAME  // The actual clause that will be
                                        // triggered by the hook. (You can call
        // this multiple times, and have multiple
        // clauses trigger on the same hook.)
        ) const;

    // returns: the updated smart contract (or nullptr)
    EXPORT std::string SmartContract_RemoveHook(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // hook
                                           // removed from it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& BYLAW_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& HOOK_NAME,   // The Hook's name as referenced in the
                                        // smart contract. (And the scripts...)
        const std::string& CLAUSE_NAME  // The actual clause that will be
                                        // triggered by the hook. (You can call
        // this multiple times, and have multiple
        // clauses trigger on the same hook.)
        ) const;

    // RETURNS: Updated version of THE_CONTRACT. (Or nullptr.)
    EXPORT std::string SmartContract_AddParty(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // party added to it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& PARTY_NYM_ID,  // Required when the smart contract is
                                          // configured to require parties to be
                                          // specified. Otherwise must be empty.
        const std::string& PARTY_NAME,  // The Party's NAME as referenced in the
                                        // smart contract. (And the scripts...)
        const std::string& AGENT_NAME   // An AGENT will be added by default for
                                        // this party. Need Agent NAME.
        ) const;

    // RETURNS: Updated version of THE_CONTRACT. (Or nullptr.)
    EXPORT std::string SmartContract_RemoveParty(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // party removed from it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& PARTY_NAME  // The Party's NAME as referenced in the
                                       // smart contract. (And the scripts...)
        ) const;

    // (FYI, that is basically the only option, until I code Entities and Roles.
    // Until then, a party can ONLY be
    // a Nym, with himself as the agent representing that same party. Nym ID is
    // supplied on ConfirmParty() below.)

    // Used when creating a theoretical smart contract (that could be used over
    // and over again with different parties.)
    //
    // returns: the updated smart contract (or nullptr)
    EXPORT std::string SmartContract_AddAccount(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // account added to it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& PARTY_NAME,  // The Party's NAME as referenced in the
                                        // smart contract. (And the scripts...)
        const std::string& ACCT_NAME,   // The Account's name as referenced in
                                        // the
                                        // smart contract
        const std::string& INSTRUMENT_DEFINITION_ID  // Instrument Definition ID
                                                     // for the
                                                     // Account.
        ) const;

    // returns: the updated smart contract (or nullptr)
    EXPORT std::string SmartContract_RemoveAccount(
        const std::string& THE_CONTRACT,   // The contract, about to have the
                                           // account removed from it.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& PARTY_NAME,  // The Party's NAME as referenced in the
                                        // smart contract. (And the scripts...)
        const std::string& ACCT_NAME  // The Account's name as referenced in the
                                      // smart contract
        ) const;

    /** This function returns the count of how many trans#s a Nym needs in order
    to confirm as
    // a specific agent for a contract. (An opening number is needed for every
    party of which
    // agent is the authorizing agent, plus a closing number for every acct of
    which agent is the
    // authorized agent.)
    */
    EXPORT std::int32_t SmartContract_CountNumsNeeded(
        const std::string& THE_CONTRACT,  // The smart contract, about to be
                                          // queried by this function.
        const std::string& AGENT_NAME) const;

    /** ----------------------------------------
    // Used when taking a theoretical smart contract, and setting it up to use
    specific Nyms and accounts. This function sets the ACCT ID for the acct
    specified by party name and acct name.
    // Returns the updated smart contract (or nullptr.)
    */
    EXPORT std::string SmartContract_ConfirmAccount(
        const std::string& THE_CONTRACT,   // The smart contract, about to be
                                           // changed by this function.
        const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const std::string& PARTY_NAME,     // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& ACCT_NAME,      // Should already be on the contract.
                                           // (This way we can find it.)
        const std::string& AGENT_NAME,     // The agent name for this asset
                                           // account.
        const std::string& ACCT_ID         // AcctID for the asset account. (For
                                           // acct_name).
        ) const;

    /** ----------------------------------------
    // Called by each Party. Pass in the smart contract obtained in the above
    call.
    // Call SmartContract_ConfirmAccount() first, as much as you need to.
    // Returns the updated smart contract (or nullptr.)
    */
    EXPORT std::string SmartContract_ConfirmParty(
        const std::string& THE_CONTRACT,  // The smart contract, about to be
                                          // changed by this function.
        const std::string& PARTY_NAME,    // Should already be on the contract.
                                          // This way we can find it.
        const std::string& NYM_ID,  // Nym ID for the party, the actual owner,
        const std::string& NOTARY_ID) const;
    // ===> AS WELL AS for the default AGENT of that party.

    /* ----------------------------------------
    Various informational functions for the Smart Contracts.
    */

    EXPORT bool Smart_AreAllPartiesConfirmed(
        const std::string& THE_CONTRACT) const;  // true or false?
    EXPORT std::int32_t Smart_GetBylawCount(
        const std::string& THE_CONTRACT) const;
    EXPORT std::string Smart_GetBylawByIndex(
        const std::string& THE_CONTRACT,
        const std::int32_t& nIndex) const;  // returns the name of the bylaw.
    EXPORT std::string Bylaw_GetLanguage(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME) const;
    EXPORT std::int32_t Bylaw_GetClauseCount(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME) const;
    EXPORT std::string Clause_GetNameByIndex(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::int32_t& nIndex) const;  // returns the name of the clause.
    EXPORT std::string Clause_GetContents(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::string& CLAUSE_NAME) const;  // returns the contents of the
                                                // clause.
    EXPORT std::int32_t Bylaw_GetVariableCount(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME) const;
    EXPORT std::string Variable_GetNameByIndex(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::int32_t& nIndex) const;  // returns the name of the variable.
    EXPORT std::string Variable_GetType(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::string& VARIABLE_NAME) const;  // returns the type of the
                                                  // variable.
    EXPORT std::string Variable_GetAccess(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::string& VARIABLE_NAME) const;  // returns the access level of
                                                  // the
                                                  // variable.
    EXPORT std::string Variable_GetContents(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::string& VARIABLE_NAME) const;  // returns the contents of the
                                                  // variable.
    EXPORT std::int32_t Bylaw_GetHookCount(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME) const;
    EXPORT std::string Hook_GetNameByIndex(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::int32_t& nIndex) const;  // returns the name of the hook.
    EXPORT std::int32_t Hook_GetClauseCount(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::string& HOOK_NAME) const;  // for iterating clauses on a
                                              // hook.
    EXPORT std::string Hook_GetClauseAtIndex(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::string& HOOK_NAME,
        const std::int32_t& nIndex) const;
    EXPORT std::int32_t Bylaw_GetCallbackCount(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME) const;
    EXPORT std::string Callback_GetNameByIndex(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::int32_t& nIndex) const;  // returns the name of the callback.
    EXPORT std::string Callback_GetClause(
        const std::string& THE_CONTRACT,
        const std::string& BYLAW_NAME,
        const std::string& CALLBACK_NAME) const;  // returns name of clause
                                                  // attached to
                                                  // callback.
    EXPORT std::int32_t Smart_GetPartyCount(
        const std::string& THE_CONTRACT) const;
    EXPORT std::string Smart_GetPartyByIndex(
        const std::string& THE_CONTRACT,
        const std::int32_t& nIndex) const;  // returns the name of the party.
    EXPORT bool Smart_IsPartyConfirmed(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME) const;  // true or false?
    EXPORT std::string Party_GetID(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME) const;  // returns either NymID or Entity
                                               // ID.
    EXPORT std::int32_t Party_GetAcctCount(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME) const;
    EXPORT std::string Party_GetAcctNameByIndex(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME,
        const std::int32_t& nIndex) const;  // returns the name of the clause.
    EXPORT std::string Party_GetAcctID(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME,
        const std::string& ACCT_NAME) const;  // returns account ID for a given
                                              // acct
                                              // name.
    EXPORT std::string Party_GetAcctInstrumentDefinitionID(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME,
        const std::string& ACCT_NAME) const;  // returns instrument definition
                                              // ID
                                              // for a
                                              // given acct
                                              // name.
    EXPORT std::string Party_GetAcctAgentName(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME,
        const std::string& ACCT_NAME) const;  // returns agent name authorized
                                              // to
    // administer a given named acct. (If
    // it's set...)
    EXPORT std::int32_t Party_GetAgentCount(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME) const;
    EXPORT std::string Party_GetAgentNameByIndex(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME,
        const std::int32_t& nIndex) const;  // returns the name of the agent.
    EXPORT std::string Party_GetAgentID(
        const std::string& THE_CONTRACT,
        const std::string& PARTY_NAME,
        const std::string& AGENT_NAME) const;  // returns ID of the agent. (If
                                               // there is
                                               // one...)

    /*
    Msg_HarvestTransactionNumbers

    This function will load up the cron item (which is either a market offer, a
    payment plan,
    or a SMART CONTRACT.) UPDATE: this function operates on messages, not cron
    items.

    Then it will try to harvest all of the closing transaction numbers for
    NYM_ID that are
    available to be harvested from THE_CRON_ITEM. (There might be zero #s
    available for that
    Nym, which is still a success and will return true. False means error.)

    YOU MIGHT ASK:

    WHY WOULD I WANT to harvest ONLY the closing numbers for the Nym, and not
    the OPENING
    numbers as well? The answer is because for this Nym, the opening number
    might already
    be burned. For example, if Nym just tried to activate a smart contract, and
    the activation
    FAILED, then maybe the opening number is already gone, even though his
    closing numbers, on the
    other hand, are still valid for retrieval. (I have to double check this.)

    HOWEVER, what if the MESSAGE failed, before it even TRIED the transaction?
    In which case,
    the opening number is still good also, and should be retrieved.

    Remember, I have to keep signing for my transaction numbers until they are
    finally closed out.
    They will appear on EVERY balance agreement and transaction statement from
    here until the end
    of time, whenever I finally close out those numbers. If some of them are
    still good on a failed
    transaction, then I want to retrieve them so I can use them, and eventually
    close them out.

    ==> Whereas, what if I am the PARTY to a smart contract, but I am not the
    actual ACTIVATOR / ORIGINATOR
    (who activated the smart contract on the server). Therefore I never sent any
    transaction to the
    server, and I never burned my opening number. It's probably still a good #.
    If my wallet is not a piece
    of shit, then I should have a stored copy of any contract that I signed. If
    it turns out in the future
    that that contract wasn't activated, then I can retrieve not only my closing
    numbers, but my OPENING
    number as well! IN THAT CASE, I would call HarvestAllNumbers() instead of
    HarvestClosingNumbers().


    UPDATE: The above logic is now handled automatically in
    HarvestTransactionNumbers.
    Therefore HarvestClosingNumbers and HarvestAllNumbers have been removed.

    */

    // Returns OT_BOOL
    EXPORT bool Msg_HarvestTransactionNumbers(
        const std::string& THE_MESSAGE,
        const std::string& NYM_ID,
        const bool& bHarvestingForRetry,
        const bool& bReplyWasSuccess,
        const bool& bReplyWasFailure,
        const bool& bTransactionWasSuccess,
        const bool& bTransactionWasFailure) const;

    // Returns OT_BOOL
    // std::int32_t HarvestClosingNumbers(const std::string& NOTARY_ID,
    //    const std::string& NYM_ID,
    //    const std::string& THE_CRON_ITEM);
    //
    //
    //
    // Returns OT_BOOL
    // std::int32_t HarvestAllNumbers(const std::string& NOTARY_ID,
    //    const std::string& NYM_ID,
    //    const std::string& THE_CRON_ITEM);

    /** -----------------------------------------------------------------
    // LOAD USER PUBLIC KEY -- from local storage
    //
    // (returns as STRING)
    //
    // MEANT TO BE USED in cases where a private key is also available.
    */
    EXPORT std::string LoadUserPubkey_Encryption(
        const std::string& NYM_ID) const;  // returns nullptr, or a public key.

    EXPORT std::string LoadUserPubkey_Signing(
        const std::string& NYM_ID) const;  // returns nullptr, or a public key.

    /** -----------------------------------------------------------------
    // LOAD PUBLIC KEY -- from local storage
    //
    // (returns as STRING)
    //
    // MEANT TO BE USED in cases where a private key is NOT available.
    */
    EXPORT std::string LoadPubkey_Encryption(
        const std::string& NYM_ID) const;  // returns nullptr, or a public key.

    EXPORT std::string LoadPubkey_Signing(
        const std::string& NYM_ID) const;  // returns nullptr, or a public key.

    /** ------------------------------------------------------------------------
    //
    // Verify that NYM_ID (including its Private Key) is an
    // available and verified user in local storage.
    //
    // Loads the user's private key, verifies, then returns OT_TRUE or OT_FALSE.
    */
    EXPORT bool VerifyUserPrivateKey(
        const std::string& NYM_ID) const;  // returns OT_BOOL

/** --------------------------------------------------------------
// LOAD PURSE or Mint or ASSET CONTRACT or SERVER CONTRACT -- (from local
storage)
//
// Based on Instrument Definition ID: load a purse, a public mint, or an
asset/server
contract
// and return it as a string -- or return nullptr if it wasn't found.
*/
#if OT_CASH
    EXPORT std::string LoadPurse(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& NYM_ID) const;  // returns
                                           // nullptr,
                                           // or a
                                           // purse.

    EXPORT std::string LoadMint(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID) const;  // returns nullptr,
                                                             // or a mint
#endif                                                       // OT_CASH

    EXPORT std::string LoadServerContract(const std::string& NOTARY_ID)
        const;  // returns nullptr, or a server contract.

#if OT_CASH
    //! Returns OT_TRUE if the mint is still usable.
    //! Returns OT_FALSE if expired or other error.
    //
    EXPORT bool Mint_IsStillGood(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID) const;
#endif  // OT_CASH

    /** --------------------------------------------------------------
    // IS BASKET CURRENCY ?
    //
    // Tells you whether or not a given instrument definition is actually a
    basket
    currency.
    */
    EXPORT bool IsBasketCurrency(const std::string& INSTRUMENT_DEFINITION_ID)
        const;  // returns OT_BOOL (OT_TRUE or
                // OT_FALSE aka 1 or 0.)

    /** --------------------------------------------------------------------
    // Get Basket Count (of backing instrument definitions.)
    //
    // Returns the number of instrument definitions that make up this basket.
    // (Or zero.)
    */
    EXPORT std::int32_t Basket_GetMemberCount(
        const std::string& BASKET_INSTRUMENT_DEFINITION_ID) const;

    /** --------------------------------------------------------------------
    // Get Asset Type of a basket's member currency, by index.
    //
    // (Returns a string containing Instrument Definition ID, or nullptr).
    */
    EXPORT std::string Basket_GetMemberType(
        const std::string& BASKET_INSTRUMENT_DEFINITION_ID,
        const std::int32_t& nIndex) const;

    /** ----------------------------------------------------
    // GET BASKET MINIMUM TRANSFER AMOUNT
    //
    // Returns a std::int64_t containing the minimum transfer
    // amount for the entire basket.
    //
    // FOR EXAMPLE:
    // If the basket is defined as 10 Rands == 2 Silver, 5 Gold, 8 Euro,
    // then the minimum transfer amount for the basket is 10. This function
    // would return a string containing "10", in that example.
    */
    EXPORT std::int64_t Basket_GetMinimumTransferAmount(
        const std::string& BASKET_INSTRUMENT_DEFINITION_ID) const;

    /** ----------------------------------------------------
    // GET BASKET MEMBER's MINIMUM TRANSFER AMOUNT
    //
    // Returns a std::int64_t containing the minimum transfer
    // amount for one of the member currencies in the basket.
    //
    // FOR EXAMPLE:
    // If the basket is defined as 10 Rands == 2 Silver, 5 Gold, 8 Euro,
    // then the minimum transfer amount for the member currency at index
    // 0 is 2, the minimum transfer amount for the member currency at
    // index 1 is 5, and the minimum transfer amount for the member
    // currency at index 2 is 8.
    */
    EXPORT std::int64_t Basket_GetMemberMinimumTransferAmount(
        const std::string& BASKET_INSTRUMENT_DEFINITION_ID,
        const std::int32_t& nIndex) const;

    EXPORT std::string LoadInbox(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID) const;  // Returns nullptr, or an inbox.

    EXPORT std::string LoadOutbox(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID) const;  // returns nullptr, or an outbox.

    //! These versions don't verify the ledger, they just load it up.
    //
    EXPORT std::string LoadInboxNoVerify(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID) const;  // Returns nullptr, or an inbox.

    EXPORT std::string LoadOutboxNoVerify(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID) const;  // returns nullptr, or an outbox.

    // from local storage.
    EXPORT std::string LoadPaymentInbox(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID) const;  // Returns nullptr, or a payment
                                           // inbox.

    EXPORT std::string LoadPaymentInboxNoVerify(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID) const;  // Returns nullptr, or a payment
                                           // inbox.

    //! NOTE: Sometimes the user ID is also passed in the "account ID" field,
    // depending
    //! on what kind of record box it is.
    // from local storage.
    EXPORT std::string LoadRecordBox(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID) const;  // Returns nullptr, or a
                                               // RecordBox.

    EXPORT std::string LoadRecordBoxNoVerify(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID) const;  // Returns nullptr, or a
                                               // RecordBox.

    EXPORT bool ClearRecord(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,  // NYM_ID can be passed here as well.
        const std::int32_t& nIndex,
        const bool& bClearAll  // if true, nIndex is ignored.
        ) const;

    // The expired box is only for incoming/outgoing payments that expired
    // before proecssing.
    //
    // (It doesn't apply to asset account inbox/outbox because those receipts
    // have already processed and thus cannot ever expire.)
    //
    // Whereas an incoming cheque can be left in the payments inbox and THEN
    // expire -- so this is where it will go when the user records the payment;
    // instead of putting it in the record box, we put it here. That way we
    // can tell which ones have actually expired, versus the ones that "expired"
    // AFTER they were put into the record box. (Meaning they never expired, but
    // they just seem that way because the "to" date passed sometime AFTER going
    // into the record box.) Whereas the expired box is for payments that
    // expired
    // BEFORE going into the record box. Basically, when you call RecordPayment,
    // if the instrument is expired BEFORE being recorded, it goes into the
    // expired
    // box -- whereas if it goes into the record box and THEN expires, then we
    // know
    // it wasn't expired at the time that it was recorded.)
    //
    EXPORT std::string LoadExpiredBox(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID) const;  // Returns nullptr, or an ExpiredBox

    EXPORT std::string LoadExpiredBoxNoVerify(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID) const;  // Returns nullptr, or a ExpiredBox.

    EXPORT bool ClearExpired(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::int32_t& nIndex,
        const bool& bClearAll  // if true, nIndex is
                               // ignored.
        ) const;

    //! Find out how many pending transactions (and receipts) are in this inbox.
    EXPORT std::int32_t Ledger_GetCount(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_LEDGER) const;  // Returns number
                                               // of
                                               // transactions
                                               // within.

    //! Creates a new 'response' ledger, set up with the right Notary ID, etc,
    // so you can
    //! add the 'response' transactions to it, one by one. (Pass in the original
    // ledger
    //! that you are responding to, as it uses the data from it to set up the
    // response.)
    //
    EXPORT std::string Ledger_CreateResponse(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID) const;

    //! Lookup a transaction or its ID (from within a ledger) based on index or
    //! transaction number.
    //
    EXPORT std::string Ledger_GetTransactionByIndex(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_LEDGER,
        const std::int32_t& nIndex) const;  // returns transaction by index.

    EXPORT std::string Ledger_GetTransactionByID(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_LEDGER,
        const std::int64_t& TRANSACTION_NUMBER) const;  // returns transaction
                                                        // by ID.

    EXPORT std::int64_t Ledger_GetTransactionIDByIndex(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_LEDGER,
        const std::int32_t& nIndex) const;  // returns transaction number by
                                            // index.

    EXPORT std::string Ledger_GetTransactionNums(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_LEDGER) const;

    //! Add a transaction to a ledger.
    //
    EXPORT std::string Ledger_AddTransaction(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_LEDGER,
        const std::string& THE_TRANSACTION) const;

    //! Create a 'response' transaction, that will be used to indicate my
    //! acceptance or rejection of another transaction. Usually an entire
    //! ledger full of these is sent to the server as I process the various
    //! transactions in my inbox.
    //
    EXPORT std::string Transaction_CreateResponse(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& RESPONSE_LEDGER,       // To be sent to the server...
        const std::string& ORIGINAL_TRANSACTION,  // Responding to...?
        const bool& BOOL_DO_I_ACCEPT) const;

    /**
    -------------------------------------------------------------------------
    // (Response Ledger) LEDGER FINALIZE RESPONSE
    //
    // AFTER you have set up all the transaction responses, call THIS function
    // to finalize them by adding a BALANCE AGREEMENT.
    //
    // MAKE SURE you have the latest copy of the account file, inbox file, and
    // outbox file, since we will need those in here to create the balance
    statement
    // properly.
    //
    // (Client software may wish to check those things, when downloaded, against
    // the local copies and the local signed receipts. In this way, clients can
    // protect themselves against malicious servers.)
    */
    EXPORT std::string Ledger_FinalizeResponse(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_LEDGER  // 'Response' ledger be sent to the
        ) const;                       // server...

    /**
    -------------------------------------------------------------------------
    // Ledger_GetInstrument (by index)
    //
    // Lookup a financial instrument (from within a transaction that is inside
    // a ledger) based on index or transaction number.

    sendNymInstrument does this:
    -- Puts an OTPayment (a form of contract) as an encrypted Payload on an
    OTMessage(1).
    -- Also puts instrument (same contract) as CLEAR payload on an OTMessage(2).
    -- (1) is sent to server, and (2) is added to Outpayments messages.
    -- (1) gets added to recipient's Nymbox as "in ref to" string on a
    "instrumentNotice" transaction.
    -- When recipient processes Nymbox, the "instrumentNotice" transaction
    (containing (1) in its "in ref to"
    field) is copied and added to the recipient's paymentInbox.
    -- When recipient iterates through paymentInbox transactions, they are ALL
    "instrumentNotice"s. Each
    transaction contains an OTMessage in its "in ref to" field, and that
    OTMessage object contains an
    encrypted payload of the instrument itself (an OTPayment object containing a
    cheque or payment plan
    or invoice etc.)
    -- When sender gets Outpayments contents, the original instrument (inside an
    OTPayment) is stored IN THE
    CLEAR as payload on an OTMessage.

    THEREFORE:
    TO EXTRACT INSTRUMENT FROM PAYMENTS INBOX:
    -- Iterate through the transactions in the payments inbox.
    -- (They should all be "instrumentNotice" transactions.)
    -- Each transaction contains (1) OTMessage in "in ref to" field, which in
    turn contains an encrypted
    OTPayment in the payload field, which then contains the instrument itself.
    -- *** Therefore, this function, based purely on ledger index (as we
    iterate) extracts the
    OTMessage from the Transaction "in ref to" field (for the transaction at
    that index), then decrypts
    the payload on that message and returns the decrypted cleartext.
    */
    EXPORT std::string Ledger_GetInstrument(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_LEDGER,
        const std::int32_t& nIndex) const;  // returns financial instrument by
                                            // index
                                            // of the
                                            // transaction it's in.

    EXPORT std::string Ledger_GetInstrumentByReceiptID(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_LEDGER,
        const std::int64_t& lReceiptId) const;

    EXPORT std::string Ledger_GetInstrument_lowlevel(
        const bool index_or_receipt,
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_LEDGER,
        const std::int32_t& nIndex,
        const std::int64_t& lReceiptId) const;

    // NOTE: If an instrument is already expired when this function is called,
    // it will be moved
    // to the expired box instead of the record box.
    EXPORT bool RecordPayment(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const bool& bIsInbox,  // true == payments inbox. false == payments
                               // outbox.
        const std::int32_t& nIndex,    // removes payment instrument (from
                                       // payments in
                                       // or out box) and moves to record box.
        const bool& bSaveCopy) const;  // If false, a copy will NOT be saved in
                                       // the
                                       // record box.

    //! Get Transaction Type (internally uses GetTransactionTypeString().)
    //
    EXPORT std::string Transaction_GetType(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_TRANSACTION) const;

    //! Transactions do not have request numbers. However, if you have a
    // replyNotice
    //! in your Nymbox, which is an OTTransaction object, it will CONTAIN a
    // server
    //! reply to some previous message. This function will only work on a
    // replyNotice,
    //! and it returns the actual request number of the server reply inside that
    // notice.
    //! Used for calling HaveAlreadySeenReply() in order to see if we've already
    //! processed the reply for that message.
    //
    EXPORT std::int64_t ReplyNotice_GetRequestNum(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& THE_TRANSACTION) const;

    /** --------------------------------------------------------------------
    // Retrieve Voucher from Transaction
    //
    // If you withdrew into a voucher instead of cash, this function allows
    // you to retrieve the actual voucher cheque from the reply transaction.
    // (A voucher is a cheque drawn on an std::int32_t server account instead
    // of a user's asset account, so the voucher cannot ever bounce due to
    // insufficient funds. We are accustomed to this functionality already
    // in our daily lives, via "money orders" and "cashier's cheques".)
    //
    // How would you use this in full?
    //
    // First, call withdrawVoucher() in order to send the request
    // to the server. (You may optionally call FlushMessageBuffer()
    // before doing this.)
    //
    // Then, call PopMessageBuffer() to retrieve any server reply.
    //
    // If there is a message from the server in reply, then call
    // Message_GetCommand to verify that it's a reply to the message
    // that you sent, and call Message_GetSuccess to verify whether
    // the message was a success.
    //
    // If it was a success, next call Message_GetLedger to retrieve
    // the actual "reply ledger" from the server.
    //
    // Penultimately, call Ledger_GetTransactionByID() and then,
    // finally, call Transaction_GetVoucher() (below) in order to
    // retrieve the voucher cheque itself from the transaction.
    */
    EXPORT std::string Transaction_GetVoucher(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_TRANSACTION) const;

    /** --------------------------------------------------
    //
    // Get TransactionSuccess
    //
    // OT_TRUE  (1) == acknowledgment
    // OT_FALSE (0) == rejection
    // OT_ERROR(-1) == error_state (such as dropped message.)
    //
    // Returns OT_BOOL.
    */
    EXPORT std::int32_t Transaction_GetSuccess(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_TRANSACTION) const;

    EXPORT std::int32_t Transaction_IsCanceled(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_TRANSACTION) const;

    /** Gets the balance agreement success (from a transaction.)
    // returns OT_BOOL.
    // OT_TRUE  (1) == acknowledgment
    // OT_FALSE (0) == rejection
    // OT_ERROR(-1) == error_state (such as dropped message.)
    */
    EXPORT std::int32_t Transaction_GetBalanceAgreementSuccess(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_TRANSACTION) const;

    /** --------------------------------------------------
    //
    // Get Transaction Date Signed (internally uses
    OTTransaction::GetDateSigned().)
    */
    EXPORT time64_t Transaction_GetDateSigned(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_TRANSACTION) const;

    EXPORT std::int64_t Transaction_GetAmount(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_TRANSACTION) const;

    /** --------------------------------------------------
    //
    // PENDING TRANSFER (various functions)
    //
    // When someone has sent you a PENDING TRANSFER (a form of transaction
    // that will be sitting in your inbox waiting for you to accept/reject it)
    // then, as you are reading the inbox, you can use these functions in
    // order to get data from each pending transfer. That way your user can
    // then decide whether to accept or reject it (see the ledger functions.)
    */
    EXPORT std::string Pending_GetNote(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_TRANSACTION) const;

    EXPORT std::string Transaction_GetSenderNymID(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_TRANSACTION) const;

    EXPORT std::string Transaction_GetSenderAcctID(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_TRANSACTION) const;

    EXPORT std::string Transaction_GetRecipientNymID(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_TRANSACTION) const;

    EXPORT std::string Transaction_GetRecipientAcctID(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_TRANSACTION) const;

    /** The pending notice in the inbox has a transaction number that
    // was issued to the server (so it could put the notice in your inbox.)
    // But it's IN REFERENCE TO a transfer that was initiated by another
    // user. THIS function will retrieve THAT transaction number, because
    // this function queries a pending transaction to see what transaction
    // it is "in reference to."
    */
    EXPORT std::int64_t Transaction_GetDisplayReferenceToNum(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_TRANSACTION) const;

#if OT_CASH
    /**
    // PURSES (containing cash tokens.)


    UNDERSTAND: If you write someone a cheque, it can be in the amount of $27.63
    But you cannot do this with a cash token, since it can only represent
    a standard denomination. (1, 5, 10, 25, 100, 500, 1000, 2000, 5000,
    10000, etc.)

    Therefore if you wanted to give CASH to someone in the amount of $27.63,
    you would have to give them a PURSE, not a single token. And the purse
    would contain a 2000 cent token, a 500 cent token, two 100-cent tokens,
    two 25-cent tokens, a 10-cent token, and three 1-cent tokens.

    The purse would thus contain 2763 cents worth of digital cash tokens, all
    neatly wrapped up in the same purse, all encrypted to the same owner or key.

    (The fact that 2763 displays as $27.63 should be purely the result of a
    mask applied by the currency contract.)

    (above a bit.)
    EXPORT std::string LoadPurse(const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& NYM_ID); // returns nullptr, or a purse.
    */

    /** Warning! This will overwrite whatever purse is there.
    // The proper way to use this function is, LOAD the purse,
    // then Merge whatever other purse you want into it, then
    // SAVE it again. (Which is all handled for you automatically
    // if you use Wallet_ImportPurse.) Tokens may have to
    // be decrypted from one owner and re-encrypted to the new owner.
    // Do you want to have to deal with that? If you still insist on
    // using SavePurse directly, just remember to load first,
    // then pop/push any tokens you need to, keeping in mind that
    // at this low level you are responsible to make sure the purse
    // and token have the same owner, then save again to overwrite
    // whatever purse was there before.
    // (Presumably if any tokens were removed, it's because they are
    // no longer any good, or because they were given to someone else
    // and then a copy was recorded in your payment outbox, or whatever.)
    */
    EXPORT bool SavePurse(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& NYM_ID,
        const std::string& THE_PURSE) const;

    //
    EXPORT std::string CreatePurse(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& OWNER_ID,
        const std::string& SIGNER_ID) const;  // returns nullptr, or a purse.

    // Creates a password-protected purse, instead of nym-protected.
    //
    EXPORT std::string CreatePurse_Passphrase(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& SIGNER_ID) const;

    //! Get Purse Total Value (internally uses GetTotalValue().)
    //
    //! Returns the purported sum of all the tokens within.
    //
    EXPORT std::int64_t Purse_GetTotalValue(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& THE_PURSE) const;

    //! returns a count of the number of cash tokens inside this purse.
    //
    EXPORT std::int32_t Purse_Count(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& THE_PURSE) const;

    //! Some purses are encrypted to a specific Nym.
    //! Whereas other purses are encrypted to a passphrase.
    //! This function returns bool and lets you know, either way.
    //
    EXPORT bool Purse_HasPassword(
        const std::string& NOTARY_ID,
        const std::string& THE_PURSE) const;

    //! Returns the TOKEN on top of the stock (LEAVING it on top of the stack,
    //! but giving you a string copy of it.)
    //
    // returns empty string if failure.
    //
    EXPORT std::string Purse_Peek(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& OWNER_ID,
        const std::string& THE_PURSE) const;

    //! Removes the token from the top of the stock and DESTROYS IT,
    //! and RETURNS THE UPDATED PURSE (with the token now missing from it.)
    //! WARNING: Do not call this function unless you have PEEK()d FIRST!!
    //! Otherwise you will lose the token and get left "holding the bag".
    // returns nullptr if failure.
    EXPORT std::string Purse_Pop(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& OWNER_OR_SIGNER_ID,
        const std::string& THE_PURSE) const;

    //! Pushes a token onto the stack (of the purse.)
    //! Returns the updated purse (now including the token.)
    // Returns nullptr if failure.
    EXPORT std::string Purse_Push(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& SIGNER_ID,  // The purse, in order to be changed,
                                       // must
                                       // be re-signed, which requires a private
                                       // Nym. Even if the purse is
                                       // password-protected, then there's no
        // owner, but you still need to pass a Nym
        // in here to sign it (doesn't really
        // matter which one, but must have private
        // key for signing.)
        const std::string& OWNER_ID,  // If the purse is password-protected,
                                      // then
                                      // there's no owner, and this owner
        // parameter should be nullptr. However, if
        // the purse DOES have a Nym owner, then
        // you MUST pass the owner's Nym ID here,
        // in order for this action to be
        // successful. Furthermore, the public key
        // for that Nym must be available, in order
        // to encrypt the token being pushed into
        // the purse. (Private key NOT necessary
        // for owner, in this case.)
        const std::string& THE_PURSE,
        const std::string& THE_TOKEN) const;

    /**
    Makes an exact copy of a purse, except with no tokens inside. (Value of
    zero.)
    // Useful when you need to create a temporary purse for moving tokens
    around, and
    // you don't want some new symmetric/master key being generated for that
    purse as
    // though it were really some new "other purse."
    // For example, if you have a password-protected purse, you might want to
    pop all of
    // the tokens off of it, and as you iterate, re-assign half of them to new
    ownership,
    // push those onto a new purse owned by that new owner. But you only want to
    do this
    // with half of the tokens... the other half of the tokens, you just want to
    push onto
    // a temp purse (for the loop) that's owned by the original owner, so you
    can then save
    // it back over the original in storage (since it contains "all the tokens
    that WEREN'T
    // deposited" or "all the tokens that WEREN'T exported" etc.
    //
    // The point? If the "original owner" is a password-protected purse with a
    symmetric
    // key, then you can't just generate some new "temp purse" without also
    generating a
    // whole new KEY, which you DO NOT want to do. You also don't want to have
    to deal with
    // re-assigning ownership back and forth between the two purses -- you just
    want to
    // shove some tokens into one temporarily so you can finish your loop.
    // You could take the original purse and make a copy of it, and then just
    pop all the
    // tokens off of it one-by-one, but that is very cumbersome and expensive.
    But that'd
    // be the only way to get a copy of the original purse with the SAME
    symmetric key,
    // except empty, so you can use it as a temp purse.
    // Therefore, to make that easier and solve that whole dilemma, I present:
    Purse_Empty.
    // Purse_Empty takes a purse and returns an empty version of it (except if
    there's
    // a symmetric/master key inside, those are preserved, so you can use it as
    a temp purse.)
    //
    // This function is effectively the same thing as calling Pop until the
    purse is empty.
    // Returns: the empty purse, or nullptr if failure.
    */
    EXPORT std::string Purse_Empty(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& SIGNER_ID,
        const std::string& THE_PURSE) const;

    // Returns OT_BOOL
    //! Should handle duplicates. Should load, merge, and save.
    EXPORT bool Wallet_ImportPurse(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& NYM_ID,    // you pass in the purse you're trying to
                                      // import
        const std::string& THE_PURSE  // It should either have your Nym ID on
                                      // it, or the key should be inside so you
                                      // can import.
        ) const;

    /** Tokens are stored in an encrypted state for security reasons.
    // This function is used for exporting those tokens to another Nym,
    // such as a Dummy nym, or another user's Nym.
    */
    EXPORT std::string Token_ChangeOwner(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& THE_TOKEN,
        const std::string& SIGNER_NYM_ID,
        const std::string& OLD_OWNER,  // Pass a NymID here as a string, or a
        // purse. (IF symmetrically encrypted, the
        // relevant key is in the purse.)
        const std::string& NEW_OWNER  // Pass a NymID here as a string, or a
                                      // purse. (IF symmetrically encrypted, the
                                      // relevant key is in the purse.)
        ) const;

    /** Returns an encrypted form of the actual blinded token ID.
    // (There's no need to decrypt the ID until redeeming the token, when
    // you re-encrypt it to the server's public key, or until spending it,
    // when you re-encrypt it to the recipient's public key, or exporting
    // it, when you create a dummy recipient and attach it to the purse.)
    */
    EXPORT std::string Token_GetID(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& THE_TOKEN) const;

    //! The actual cash value of the token. Returns a std::int64_t.
    //
    EXPORT std::int64_t Token_GetDenomination(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& THE_TOKEN) const;

    EXPORT std::int32_t Token_GetSeries(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& THE_TOKEN) const;

    //! the date is seconds since Jan 1970.
    //
    EXPORT time64_t Token_GetValidFrom(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& THE_TOKEN) const;

    // the date is seconds since Jan 1970.
    //
    EXPORT time64_t Token_GetValidTo(
        const std::string& NOTARY_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& THE_TOKEN) const;

    EXPORT std::string Token_GetInstrumentDefinitionID(
        const std::string& THE_TOKEN) const;
    EXPORT std::string Token_GetNotaryID(const std::string& THE_TOKEN) const;
#endif  // OT_CASH

    /**
    //
    // THESE FUNCTIONS were added for the PAYMENTS screen. (They are fairly
    new.)
    //
    // Basically there was a need to have DIFFERENT instruments, but to be able
    to
    // treat them as though they are a single type.
    //
    // In keeping with that, the below functions will work with disparate types.
    // You can pass [ CHEQUES / VOUCHERS / INVOICES ] and PAYMENT PLANS, and
    // SMART CONTRACTS, and PURSEs into these functions, and they should be able
    // to handle any of those types.
    //
    */
    EXPORT std::int64_t Instrmnt_GetAmount(
        const std::string& THE_INSTRUMENT) const;
    EXPORT std::int64_t Instrmnt_GetTransNum(
        const std::string& THE_INSTRUMENT) const;
    EXPORT time64_t
    Instrmnt_GetValidFrom(const std::string& THE_INSTRUMENT) const;
    EXPORT time64_t
    Instrmnt_GetValidTo(const std::string& THE_INSTRUMENT) const;
    EXPORT std::string Instrmnt_GetMemo(
        const std::string& THE_INSTRUMENT) const;
    EXPORT std::string Instrmnt_GetType(
        const std::string& THE_INSTRUMENT) const;
    EXPORT std::string Instrmnt_GetNotaryID(
        const std::string& THE_INSTRUMENT) const;
    EXPORT std::string Instrmnt_GetInstrumentDefinitionID(
        const std::string& THE_INSTRUMENT) const;
    EXPORT std::string Instrmnt_GetSenderNymID(
        const std::string& THE_INSTRUMENT) const;
    EXPORT std::string Instrmnt_GetSenderAcctID(
        const std::string& THE_INSTRUMENT) const;
    EXPORT std::string Instrmnt_GetRemitterNymID(
        const std::string& THE_INSTRUMENT) const;
    EXPORT std::string Instrmnt_GetRemitterAcctID(
        const std::string& THE_INSTRUMENT) const;
    EXPORT std::string Instrmnt_GetRecipientNymID(
        const std::string& THE_INSTRUMENT) const;
    EXPORT std::string Instrmnt_GetRecipientAcctID(
        const std::string& THE_INSTRUMENT) const;

    // *** MESSAGES BEING SENT TO THE SERVER -- BELOW!!! ***

    /** IF THE_MESSAGE is of command type usageCreditsResponse, and IF it was a
    SUCCESS,
    // then this function returns the usage credits BALANCE (it's a
    std::int64_t, but
    // passed as a string). If you adjusted the balance using the usageCredits
    // command (THE_MESSAGE being the server's reply to that) then you will see
    // the balance AFTER the adjustment. (The current "Usage Credits" balance.)
    */
    EXPORT std::int64_t Message_GetUsageCredits(
        const std::string& THE_MESSAGE) const;

    EXPORT std::int32_t completePeerReply(
        const std::string& nymID,
        const std::string& replyOrRequestID) const;

    EXPORT std::int32_t completePeerRequest(
        const std::string& nymID,
        const std::string& requestID) const;

    EXPORT std::list<std::string> getSentRequests(
        const std::string& nymID) const;

    EXPORT std::list<std::string> getIncomingRequests(
        const std::string& nymID) const;

    EXPORT std::list<std::string> getFinishedRequests(
        const std::string& nymID) const;

    EXPORT std::list<std::string> getProcessedRequests(
        const std::string& nymID) const;

    EXPORT std::list<std::string> getSentReplies(
        const std::string& nymID) const;

    EXPORT std::list<std::string> getIncomingReplies(
        const std::string& nymID) const;

    EXPORT std::list<std::string> getFinishedReplies(
        const std::string& nymID) const;

    EXPORT std::list<std::string> getProcessedReplies(
        const std::string& nymID) const;

    EXPORT std::string getRequest(
        const std::string& nymID,
        const std::string& requestID,
        const StorageBox box) const;

    EXPORT std::string getRequest_Base64(
        const std::string& nymID,
        const std::string& requestID) const;

    EXPORT std::string getReply(
        const std::string& nymID,
        const std::string& replyID,
        const StorageBox box) const;

    EXPORT std::string getReply_Base64(
        const std::string& nymID,
        const std::string& replyID) const;

    /** ----------------------------------------------------
    // GENERATE BASKET CREATION REQUEST
    //
    // (returns the basket in string form.)
    //
    // Call AddBasketCreationItem multiple times to add
    // the various currencies to the basket, and then call
    // issueBasket to send the request to the server.
    */
    EXPORT std::string GenerateBasketCreation(
        const std::string& nymID,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::uint64_t weight) const;

    /** ----------------------------------------------------
    // ADD BASKET CREATION ITEM
    //
    // (returns the updated basket in string form.)
    //
    // Call GenerateBasketCreation first (above), then
    // call this function multiple times to add the various
    // currencies to the basket, and then call issueBasket
    // to send the request to the server.
    */
    EXPORT std::string AddBasketCreationItem(
        const std::string& basketTemplate,
        const std::string& currencyID,
        const std::uint64_t& weight) const;

    /** ----------------------------------------------------
    // GENERATE BASKET EXCHANGE REQUEST
    //
    // (Returns the new basket exchange request in string form.)
    //
    // Call this function first. Then call AddBasketExchangeItem
    // multiple times, and then finally call exchangeBasket to
    // send the request to the server.
    */
    EXPORT std::string GenerateBasketExchange(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& BASKET_INSTRUMENT_DEFINITION_ID,
        const std::string& BASKET_ASSET_ACCT_ID,
        const std::int32_t& TRANSFER_MULTIPLE) const;

    //! 1    2    3
    //! 5=2,3,4 OR 10=4,6,8 OR 15=6,9,12 Etc. (The MULTIPLE.)

    /** ----------------------------------------------------
    // ADD BASKET EXCHANGE ITEM
    //
    // Returns the updated basket exchange request in string form.
    // (Or nullptr.)
    //
    // Call the above function first. Then call this one multiple
    // times, and then finally call exchangeBasket to send
    // the request to the server.
    */
    EXPORT std::string AddBasketExchangeItem(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& THE_BASKET,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& ASSET_ACCT_ID) const;

    // from local storage.
    EXPORT std::string LoadNymbox(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID) const;  // Returns
                                           // nullptr,
                                           // or a
                                           // Nymbox.

    EXPORT std::string LoadNymboxNoVerify(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID) const;  // Returns nullptr, or a Nymbox.

    /** Some server replies (to your messages) are so important that a notice is
    dropped
    // into your Nymbox with a copy of the server's reply. It's called a
    replyNotice.
    // Since the server is usually replying to a message, I've added this
    function for
    // quickly looking up the message reply, if it's there, based on the
    requestNumber.
    // This is the only example in the entire OT API where a Transaction is
    looked-up from
    // a ledger, based on a REQUEST NUMBER. (Normally transactions use
    transaction numbers,
    // and messages use request numbers. But in this case, it's a transaction
    that carries
    // a copy of a message.)
    */
    EXPORT std::string Nymbox_GetReplyNotice(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::int64_t& REQUEST_NUMBER) const;  // returns replyNotice
                                                    // transaction
                                                    // by
                                                    // requestNumber.

    /** If the client-side has ALREADY seen the server's reply to a specific
    // request number, he stores that number in a list which can be queried
    // using this function. A copy of that list is sent with nearly every
    request
    // message sent to the server. This way the server can see which replies you
    // have already received. The server will mirror every number it sees (it
    sends
    // its own list in all its replies.) Whenever you see a number mirrored in
    the
    // server's reply, that means the server now knows you got its original
    reply
    // (the one referenced by the number) and the server removed any replyNotice
    // of that from your Nymbox (so you don't have to download it.) Basically
    that
    // means you can go ahead and remove it from your list, and once you do, the
    server
    // will remove its matching copy as well.
    // When you are downloading your box receipts, you can skip any receipts
    where
    // you have ALREADY seen the reply. So you can use this function to see if
    you already
    // saw it, and if you did, then you can skip downloading that box receipt.
    // Warning: this function isn't "perfect", in the sense that it cannot tell
    you definitively
    // whether you have actually seen a server reply, but it CAN tell you if you
    have seen
    // one until it finishes the above-described protocol (it will work in that
    way, which is
    // how it was intended.) But after that, it will no longer know if you got
    the reply since
    // it has removed it from its list.
    // returns OT_BOOL.
    */
    EXPORT bool HaveAlreadySeenReply(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::int64_t& REQUEST_NUMBER) const;  // returns OT_BOOL

    EXPORT bool DoesBoxReceiptExist(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,      // Unused here for now, but still
                                        // convention.
        const std::string& ACCOUNT_ID,  // If for Nymbox (vs inbox/outbox) then
                                        // pass NYM_ID in this field also.
        const std::int32_t& nBoxType,   // 0/nymbox, 1/inbox, 2/outbox
        const std::int64_t& TRANSACTION_NUMBER) const;

    // Outgoing:

    EXPORT std::string GetSentMessage(
        const std::int64_t& REQUEST_NUMBER,
        const std::string& NOTARY_ID,
        const std::string& NYM_ID) const;

    EXPORT bool RemoveSentMessage(
        const std::int64_t& REQUEST_NUMBER,
        const std::string& NOTARY_ID,
        const std::string& NYM_ID) const;

    // Note: Might remove this from API. Basically, the sent messages queue must
    // store
    // messages (by request number) until we know for SURE whether we have a
    // success, a failure,
    // or a lost/rejected message. That is, until we DOWNLOAD the Nymbox, and
    // thus know for SURE
    // that a response to a given message is there...or not. Why do we care? For
    // making this
    // choice:
    //
    // Messages that DO have a reply are therefore already "in the system" and
    // will be handled
    // normally--they can be ignored and flushed from the "sent messages" queue.
    // Whereas messages
    // that do NOT have a reply in the Nymbox (yet are still in the "sent
    // messages" queue) can be
    // assumed safely to have been rejected at "message level" (before any
    // transaction could
    // have processed) and the reply must have been dropped on the network, OR
    // the server never
    // even received the message in the first place. EITHER WAY the trans #s can
    // be harvested
    // accordingly and then removed from the sent buffer. In a perfect world
    // (read: iteration 2)
    // these sent messages will be serialized somehow along with the Nym, and
    // not just stored in
    // RAM like this version does.

    /** -----------------------------------------------------------
    // FlushSentMessages
    //
    // Make sure to call this directly after a successful getNymboxResponse.
    // (And ONLY at that time.)
    //
    // This empties the buffer of sent messages.
    // (Harvesting any transaction numbers that are still there.)
    //
    // NOTE: You normally ONLY call this immediately after receiving
    // a successful getNymboxResponse. It's only then that you can see which
    // messages a server actually received or not -- which transactions
    // it processed (success or fail) vs which transactions did NOT
    // process (and thus did NOT leave any success/fail receipt in the
    // nymbox.)
    //
    // I COULD have just flushed myself IN the getNymboxResponse code (where
    // the reply is processed.) But then the developer using the OT API
    // would never have the opportunity to see whether a message was
    // replied to, and harvest it for himself (say, just before attempting
    // a re-try, which I plan to do in the high-level Java API, which is
    // why I'm coding it this way.)
    //
    // This way, he can do that if he wishes, THEN call this function,
    // and harvesting will still occur properly, and he will also thus have
    // his chance to check for his own replies to harvest before then.
    // This all depends on the developer using the API being smart enough
    // to call this function after a successful getNymboxResponse!
    */
    EXPORT void FlushSentMessages(
        const bool& bHarvestingForRetry,  // bHarvestingForRetry is actually
                                          // OT_BOOL
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& THE_NYMBOX) const;

    /* For emergency/testing use only. This call forces you to trust the server.
    You should never need to call this for any normal use, and hopefully
    you should never need to actually call it at all, ever. But if your Nym
    ever gets out of sync with the server, this will resync it based on
    the server's version of the data.

    Note: actually returns OT_BOOL.
    */
    // Make sure you download your Nymbox (getNymbox) before calling this,
    // so when it loads the Nymbox it will have the latest version of it.
    //
    // Also, call registerNym() and pass the server reply message in
    // here, so that it can read theMessageNym (to sync the transaction
    // numbers.)
    //
    EXPORT bool ResyncNymWithServer(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& THE_MESSAGE) const;

    /** -----------------------------------------------------------
    // GET MESSAGE COMMAND TYPE
    //
    // This way you can discover what kind of command it was.
    // All server replies are pre-pended with the @ sign. For example, if
    // you send a "getAccountData" message, the server reply is
    "getAccountDataResponse",
    // and if you send "getMint" the reply is "getMintResponse", and so on.
    */
    EXPORT std::string Message_GetCommand(const std::string& THE_MESSAGE) const;

    /** -----------------------------------------------------------
    // GET MESSAGE SUCCESS (True or False)
    //
    // Returns OT_TRUE (1) for Success and OT_FALSE (0) for Failure.
    // Returns -1 for Error condition.
    */
    EXPORT std::int32_t Message_GetSuccess(
        const std::string& THE_MESSAGE) const;

    /** -----------------------------------------------------------
    // GET MESSAGE PAYLOAD
    //
    // This way you can retrieve the payload from any message.
    // Useful, for example, for getting the encoded StringMap object
    // from the queryInstrumentDefinitions and
    queryInstrumentDefinitionsResponse messages, which both
    // use the m_ascPayload field to transport it.
    */
    EXPORT std::string Message_GetPayload(const std::string& THE_MESSAGE) const;

    /** -----------------------------------------------------------
    // GET MESSAGE "DEPTH" (USED FOR MARKET-SPECIFIC MESSAGES.)
    //
    // Returns the count of relevant items, so you know whether to bother
    reading the payload.
    // Returns -1 if error.
    //
    // The "depth" variable stores the count of items being returned.
    // For example, if I call getMarketList, and 10 markets are returned,
    // then depth will be set to 10. OR, if I call getNymMarketOffers, and
    // the Nym has 16 offers on the various markets, then the depth will be 16.
    //
    // This value is important when processing server replies to market
    inquiries.
    // If the depth is 0, then you are done. End. BUT! if it contains a number,
    such as 10,
    // then that means you will want to next READ those 10 markets (or offers,
    or trades, etc)
    // out of the server reply's payload.
    //
    // Whereas if success is TRUE, but depth is 0, that means while the message
    processed
    // successfully, the list was simply empty (and thus it would be a waste of
    time trying
    // to unpack the non-existent, empty list of data items from the payload of
    your successful
    // reply.)
    */
    EXPORT std::int32_t Message_GetDepth(const std::string& THE_MESSAGE) const;

    /** -----------------------------------------------------------
    // GET MESSAGE TRANSACTION SUCCESS (True or False)
    //
    // Returns OT_TRUE (1) for Success and OT_FALSE (0) for Failure.
    // Returns OT_ERROR for error. (-1)
    */
    EXPORT std::int32_t Message_GetTransactionSuccess(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_MESSAGE) const;

    EXPORT std::int32_t Message_IsTransactionCanceled(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_MESSAGE) const;

    /** -----------------------------------------------------------
    // GET BALANCE AGREEMENT SUCCESS (From a MESSAGE.)
    //
    // Returns OT_TRUE (1) for Success and OT_FALSE (0) for Failure.
    // Returns OT_ERROR for error. (-1)
    */
    EXPORT std::int32_t Message_GetBalanceAgreementSuccess(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& THE_MESSAGE) const;

    /** -----------------------------------------------------------
    // GET MESSAGE LEDGER
    //
    // If you just received a server response to a transaction, and
    // you want to actually iterate through the response ledger for
    // that transaction, this function will retrieve it for you.
    */
    EXPORT std::string Message_GetLedger(const std::string& THE_MESSAGE) const;

    /** -----------------------------------------------------------
    // GET NEW ASSET TYPE ID
    //
    // If you just issued a new instrument definition, you'll want to read the
    // server reply and get the new instrument definition ID out of it.
    // Otherwise how will you ever open accounts in that new type?
    */
    EXPORT std::string Message_GetNewInstrumentDefinitionID(
        const std::string& THE_MESSAGE) const;

    /** -----------------------------------------------------------
    // GET NEW ISSUER ACCOUNT ID
    //
    // If you just issued a new instrument definition, you'll want to read the
    // server reply and get the new issuer acct ID out of it.
    // Otherwise how will you ever issue anything with it?
    */
    EXPORT std::string Message_GetNewIssuerAcctID(
        const std::string& THE_MESSAGE) const;

    /** -----------------------------------------------------------
    // GET NEW ACCOUNT ID
    //
    // If you just opened a new asset account, you'll want to read the
    // server reply and get the new acct ID out of it.
    // Otherwise how will you know which account you just created?
    // This function allows you to get the new account ID out of the
    // server reply message.
    */
    EXPORT std::string Message_GetNewAcctID(
        const std::string& THE_MESSAGE) const;

    /** -----------------------------------------------------------
    // GET NYMBOX HASH
    //
    // Some messages include a copy of the Nymbox Hash. This helps the
    // server to quickly ascertain whether some messages will fail, and
    // also allows the client to query the server for this information
    // for syncronicity purposes.
    */
    EXPORT std::string Message_GetNymboxHash(
        const std::string& THE_MESSAGE) const;

    /** Get wallet BIP32 seed as hex string
     */
    EXPORT std::string Wallet_GetSeed() const;

    /** Get the BIP39 passphrase associcated with the wallet seed
     */
    EXPORT std::string Wallet_GetPassphrase() const;

    /** Get wallet BIP39 seed as a word list
     */
    EXPORT std::string Wallet_GetWords() const;

    /** Import a BIP39 seed into the wallet.
     *
     *  The imported seed will be set to the default seed if a default does not
     *  already exist.
     */
    EXPORT std::string Wallet_ImportSeed(
        const std::string& words,
        const std::string& passphrase) const;

    EXPORT void SetZMQKeepAlive(const std::uint64_t seconds) const;

    EXPORT bool CheckConnection(const std::string& server) const;

    EXPORT std::string AddChildEd25519Credential(
        const Identifier& nymID,
        const Identifier& masterID) const;

    EXPORT std::string AddChildSecp256k1Credential(
        const Identifier& nymID,
        const Identifier& masterID) const;

    EXPORT std::string AddChildRSACredential(
        const Identifier& nymID,
        const Identifier& masterID,
        const std::uint32_t keysize) const;

    EXPORT ~OTAPI_Exec() = default;

private:
    friend class api::client::implementation::Manager;

    const api::Core& api_;
    const api::client::Activity& activity_;
    const api::client::Contacts& contacts_;
    const api::network::ZMQ& zeromq_;
    const api::Identity& identity_;
    const OT_API& ot_api_;
    ContextLockCallback lock_callback_;

    OTAPI_Exec(
        const api::Core& api,
        const api::client::Activity& activity,
        const api::client::Contacts& contacts,
        const api::network::ZMQ& zeromq,
        const api::Identity& identity,
        const OT_API& otapi,
        const ContextLockCallback& lockCallback);
    OTAPI_Exec() = delete;
    OTAPI_Exec(const OTAPI_Exec&) = delete;
    OTAPI_Exec(OTAPI_Exec&&) = delete;
    OTAPI_Exec operator=(const OTAPI_Exec&) = delete;
    OTAPI_Exec operator=(OTAPI_Exec&&) = delete;
};
}  // namespace opentxs
#endif
