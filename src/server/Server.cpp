// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Server.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/server/Manager.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/api/HDSeed.hpp"
#endif
#include "opentxs/api/Identity.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/client/NymData.hpp"
#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/OTPaths.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/PushSocket.hpp"

#include "ConfigLoader.hpp"
#include "Transactor.hpp"

#include <sys/types.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include <cinttypes>
#include <cstdint>
#include <fstream>
#include <regex>
#include <string>

#define OTX_PUSH_VERSION 1
#define SERVER_PID_FILENAME "ot.pid"
#define SEED_BACKUP_FILE "seed_backup.json"
#define SERVER_CONTRACT_FILE "NEW_SERVER_CONTRACT.otc"
#define SERVER_CONFIG_LISTEN_SECTION "listen"
#define SERVER_CONFIG_BIND_KEY "bindip"
#define SERVER_CONFIG_COMMAND_KEY "command"
#define SERVER_CONFIG_NOTIFY_KEY "notification"

#define OT_METHOD "opentxs::Server::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::server
{

#ifdef _WIN32
std::int32_t OTCron::__trans_refill_amount = 500;  // The number of transaction
                                                   // numbers Cron will grab for
                                                   // itself, when it
// gets low, before each round.
std::int32_t OTCron::__cron_ms_between_process =
    10000;  // The number of milliseconds
            // (ideally) between each
            // "Cron Process" event.
std::int32_t OTCron::__cron_max_items_per_nym =
    10;  // The maximum number of cron
// items any given Nym can have
// active at the same time.
#endif

Server::Server(const opentxs::api::server::Manager& manager)
    : manager_(manager)
    , mainFile_(*this)
    , notary_(*this, manager_)
    , transactor_(*this)
    , userCommandProcessor_(*this, manager_)
    , m_strWalletFilename(String::Factory())
    , m_bReadOnly(false)
    , m_bShutdownFlag(false)
    , m_notaryID(Identifier::Factory())
    , m_strServerNymID()
    , m_nymServer(nullptr)
    , m_Cron(manager.Factory().Cron(manager))
    , notification_socket_(
          manager_.ZeroMQ().PushSocket(zmq::Socket::Direction::Connect))
{
    const auto bound = notification_socket_->Start(
        manager_.Endpoints().InternalPushNotification());

    OT_ASSERT(bound);
}

void Server::ActivateCron()
{
    otOut << "Server::ActivateCron: "
          << (m_Cron->ActivateCron() ? "(STARTED)" : "FAILED") << " \n";
}

const api::Core& Server::API() const { return manager_; }

/// Currently the test server calls this 10 times per second.
/// It also processes all the input/output at the same rate.
/// It sleeps in between. (See testserver.cpp for the call
/// and OTLog::Sleep() for the sleep code.)
///
void Server::ProcessCron()
{
    if (!m_Cron->IsActivated()) return;

    bool bAddedNumbers = false;

    // Cron requires transaction numbers in order to process.
    // So every time before I call Cron.Process(), I make sure to replenish
    // first.
    while (m_Cron->GetTransactionCount() < OTCron::GetCronRefillAmount()) {
        std::int64_t lTransNum = 0;
        bool bSuccess = transactor_.issueNextTransactionNumber(lTransNum);

        if (bSuccess) {
            m_Cron->AddTransactionNumber(lTransNum);
            bAddedNumbers = true;
        } else
            break;
    }

    if (bAddedNumbers) { m_Cron->SaveCron(); }

    m_Cron->ProcessCronItems();  // This needs to be called regularly for
                                 // trades, markets, payment plans, etc to
                                 // process.

    // NOTE:  TODO:  OTHER RE-OCCURRING SERVER FUNCTIONS CAN GO HERE AS WELL!!
    //
    // Such as sweeping server accounts after expiration dates, etc.
}

const Identifier& Server::GetServerID() const { return m_notaryID; }

const Nym& Server::GetServerNym() const { return *m_nymServer; }

bool Server::IsFlaggedForShutdown() const { return m_bShutdownFlag; }

std::pair<std::string, std::string> Server::parse_seed_backup(
    const std::string& input) const
{
    std::pair<std::string, std::string> output{};
    auto& phrase = output.first;
    auto& words = output.second;

    std::regex reg("\"passphrase\": \"(.*)\", \"words\": \"(.*)\"");
    std::cmatch match{};

    if (std::regex_search(input.c_str(), match, reg)) {
        phrase = match[1];
        words = match[2];
    }

    return output;
}

void Server::CreateMainFile(bool& mainFileExists)
{
#if OT_CRYPTO_WITH_BIP39
    const auto backup = OTDB::QueryPlainString(
        manager_.DataFolder(), SEED_BACKUP_FILE, "", "", "");
    std::string seed{};

    if (false == backup.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Seed backup found. Restoring."
              << std::endl;
        auto parsed = parse_seed_backup(backup);
        OTPassword phrase;
        OTPassword words;
        phrase.setPassword(parsed.first);
        words.setPassword(parsed.second);
        seed = manager_.Seeds().ImportSeed(words, phrase);

        if (seed.empty()) {
            otErr << OT_METHOD << __FUNCTION__ << ": Seed restoration failed."
                  << std::endl;
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Seed " << seed
                  << " restored." << std::endl;
        }
    }
#endif

#if OT_CRYPTO_SUPPORTED_KEY_HD
    NymParameters nymParameters(proto::CREDTYPE_HD);
    nymParameters.SetSeed(seed);
    nymParameters.SetNym(0);
    nymParameters.SetDefault(false);
#else
    NymParameters nymParameters(proto::CREDTYPE_LEGACY);
#endif
    m_nymServer = manager_.Wallet().Nym(nymParameters);

    if (false == bool(m_nymServer)) {
        otErr << "Error: Failed to create server nym\n";
        OT_FAIL;
    }

    if (!m_nymServer->VerifyPseudonym()) { OT_FAIL; }

    const OTIdentifier nymID = m_nymServer->ID();

    const std::string defaultTerms = "This is an example server contract.";
    const std::string& userTerms = manager_.GetUserTerms();
    std::string terms = userTerms;

    if (1 > userTerms.size()) { terms = defaultTerms; }

    const std::string defaultExternalIP = DEFAULT_EXTERNAL_IP;
    const std::string& userExternalIP = manager_.GetExternalIP();
    std::string hostname = userExternalIP;

    if (5 > hostname.size()) { hostname = defaultExternalIP; }

    const std::string defaultBindIP = DEFAULT_BIND_IP;
    const std::string& userBindIP = manager_.GetDefaultBindIP();
    std::string bindIP = userBindIP;

    if (5 > bindIP.size()) { bindIP = defaultBindIP; }

    bool notUsed = false;
    manager_.Config().Set_str(
        String::Factory(SERVER_CONFIG_LISTEN_SECTION),
        String::Factory(SERVER_CONFIG_BIND_KEY),
        String::Factory(bindIP),
        notUsed);

    const std::uint32_t defaultCommandPort = DEFAULT_COMMAND_PORT;
    const std::string& userCommandPort = manager_.GetCommandPort();
    std::uint32_t commandPort = 0;
    bool needPort = true;

    while (needPort) {
        try {
            commandPort = std::stoi(userCommandPort.c_str());
        } catch (const std::invalid_argument&) {
            commandPort = defaultCommandPort;
            needPort = false;
        } catch (const std::out_of_range&) {
            commandPort = defaultCommandPort;
            needPort = false;
        }
        commandPort =
            (MAX_TCP_PORT < commandPort) ? defaultCommandPort : commandPort;
        commandPort =
            (MIN_TCP_PORT > commandPort) ? defaultCommandPort : commandPort;
        needPort = false;
    }

    const std::string& userListenCommand = manager_.GetListenCommand();
    std::uint32_t listenCommand = 0;
    bool needListenCommand = true;

    while (needListenCommand) {
        try {
            listenCommand = std::stoi(userListenCommand.c_str());
        } catch (const std::invalid_argument&) {
            listenCommand = defaultCommandPort;
            needListenCommand = false;
        } catch (const std::out_of_range&) {
            listenCommand = defaultCommandPort;
            needListenCommand = false;
        }
        listenCommand =
            (MAX_TCP_PORT < listenCommand) ? defaultCommandPort : listenCommand;
        listenCommand =
            (MIN_TCP_PORT > listenCommand) ? defaultCommandPort : listenCommand;
        needListenCommand = false;
    }

    manager_.Config().Set_str(
        String::Factory(SERVER_CONFIG_LISTEN_SECTION),
        String::Factory(SERVER_CONFIG_COMMAND_KEY),
        String::Factory(std::to_string(listenCommand)),
        notUsed);

    const std::uint32_t defaultNotificationPort = DEFAULT_NOTIFY_PORT;

    const std::string& userListenNotification = manager_.GetListenNotify();
    std::uint32_t listenNotification = 0;
    bool needListenNotification = true;

    while (needListenNotification) {
        try {
            listenNotification = std::stoi(userListenNotification.c_str());
        } catch (const std::invalid_argument&) {
            listenNotification = defaultNotificationPort;
            needListenNotification = false;
        } catch (const std::out_of_range&) {
            listenNotification = defaultNotificationPort;
            needListenNotification = false;
        }
        listenNotification = (MAX_TCP_PORT < listenNotification)
                                 ? defaultNotificationPort
                                 : listenNotification;
        listenNotification = (MIN_TCP_PORT > listenNotification)
                                 ? defaultNotificationPort
                                 : listenNotification;
        needListenNotification = false;
    }

    manager_.Config().Set_str(
        String::Factory(SERVER_CONFIG_LISTEN_SECTION),
        String::Factory(SERVER_CONFIG_NOTIFY_KEY),
        String::Factory(std::to_string(listenNotification)),
        notUsed);

    const std::string defaultName = DEFAULT_NAME;
    const std::string& userName = manager_.GetUserName();
    std::string name = userName;

    if (1 > name.size()) { name = defaultName; }

    std::list<ServerContract::Endpoint> endpoints;
    const auto inproc = manager_.GetInproc();
    const bool useInproc = !inproc.empty();

    if (useInproc) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Creating inproc contract for instance "
              << manager_.GetInproc() << std::endl;
        ServerContract::Endpoint inproc{proto::ADDRESSTYPE_INPROC,
                                        proto::PROTOCOLVERSION_LEGACY,
                                        manager_.GetInproc(),
                                        commandPort,
                                        2};
        endpoints.push_back(inproc);
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Creating standard contract"
              << std::endl;
        ServerContract::Endpoint ipv4{proto::ADDRESSTYPE_IPV4,
                                      proto::PROTOCOLVERSION_LEGACY,
                                      hostname,
                                      commandPort,
                                      1};
        endpoints.push_back(ipv4);
        const std::string& onion = manager_.GetOnion();

        if (0 < onion.size()) {
            ServerContract::Endpoint tor{proto::ADDRESSTYPE_ONION,
                                         proto::PROTOCOLVERSION_LEGACY,
                                         onion,
                                         commandPort,
                                         1};
            endpoints.push_back(tor);
        }

        const std::string& eep = manager_.GetEEP();

        if (0 < eep.size()) {
            ServerContract::Endpoint i2p{proto::ADDRESSTYPE_EEP,
                                         proto::PROTOCOLVERSION_LEGACY,
                                         eep,
                                         commandPort,
                                         1};
            endpoints.push_back(i2p);
        }
    }

    std::shared_ptr<const ServerContract> pContract{};
    auto& wallet = manager_.Wallet();
    const auto existing = String::Factory(
        OTDB::QueryPlainString(
            manager_.DataFolder(), SERVER_CONTRACT_FILE, "", "", "")
            .data());

    if (existing->empty()) {
        pContract = wallet.Server(
            nymID->str(),
            name,
            terms,
            endpoints,
            (useInproc) ? 2 : SERVER_CONTRACT_CREATE_VERSION);
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Existing contract found. Restoring." << std::endl;
        const auto serialized =
            proto::StringToProto<proto::ServerContract>(existing);
        pContract = wallet.Server(serialized);
    }

    std::string strNotaryID;

    if (pContract) {
        std::string strHostname{};
        std::uint32_t nPort{0};
        proto::AddressType type{};

        if (!pContract->ConnectInfo(strHostname, nPort, type, type)) {
            otOut << __FUNCTION__
                  << ": Unable to retrieve connection info from "
                     "this contract. Please fix that first; see "
                     "the sample data. (Failure.)\n";
            OT_FAIL;
        }
        strNotaryID = String::Factory(pContract->ID())->Get();
    } else {
        OT_FAIL;
    }

    std::string strCachedKey;
    auto& cachedKey = manager_.Crypto().DefaultKey();

    if (cachedKey.IsGenerated()) {
        Armored ascMasterContents;

        if (cachedKey.SerializeTo(ascMasterContents)) {
            strCachedKey.assign(
                ascMasterContents.Get(), ascMasterContents.GetLength());
        } else
            OT_FAIL;
    } else {
        OT_FAIL;
    }

    OT_ASSERT(m_nymServer)

    {
        auto nymData = manager_.Wallet().mutable_Nym(nymID);

        if (false == nymData.SetScope(proto::CITEMTYPE_SERVER, name, true)) {
            OT_FAIL
        }

        if (false == nymData.SetCommonName(pContract->ID()->str())) { OT_FAIL }
    }

    m_nymServer = manager_.Wallet().Nym(nymID);

    OT_ASSERT(m_nymServer)

    const auto signedContract = proto::ProtoAsData(pContract->PublicContract());
    Armored ascContract(signedContract.get());
    auto strBookended = String::Factory();
    ascContract.WriteArmoredString(strBookended, "SERVER CONTRACT");
    OTDB::StorePlainString(
        strBookended->Get(),
        manager_.DataFolder(),
        SERVER_CONTRACT_FILE,
        "",
        "",
        "");

    otOut << "Your server contract has been saved as " << SERVER_CONTRACT_FILE
          << " in the server data directory." << std::endl;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    const std::string defaultFingerprint = manager_.Storage().DefaultSeed();
    const std::string words = manager_.Seeds().Words(defaultFingerprint);
    const std::string passphrase =
        manager_.Seeds().Passphrase(defaultFingerprint);
#else
    const std::string words;
    const std::string passphrase;
#endif

    std::string json;
    json += "{ \"passphrase\": \"";
    json += passphrase;
    json += "\", \"words\": \"";
    json += words;
    json += "\" }\n";

    OTDB::StorePlainString(
        json, manager_.DataFolder(), SEED_BACKUP_FILE, "", "", "");

    mainFileExists = mainFile_.CreateMainFile(
        strBookended->Get(), strNotaryID, "", nymID->str(), strCachedKey);

    manager_.Config().Save();
}

void Server::Init(bool readOnly)
{
    m_bReadOnly = readOnly;

    if (!ConfigLoader::load(
            manager_.Crypto(), manager_.Config(), WalletFilename())) {
        otErr << "Unable to Load Config File!";
        OT_FAIL;
    }

    auto dataPath = String::Factory(manager_.DataFolder().c_str());

    // PID -- Make sure we're not running two copies of OT on the same data
    // simultaneously here.
    // If we want to WRITE to the data location, then we can't be in
    // read-only mode.
    if (!readOnly) {
        // 1. READ A FILE STORING THE PID. (It will already exist, if OT is
        // already running.)
        //
        // We open it for reading first, to see if it already exists. If it
        // does, we read the number. 0 is fine, since we overwrite with 0 on
        // shutdown. But any OTHER number means OT is still running. Or it
        // means it was killed while running and didn't shut down properly,
        // and that you need to delete the pid file by hand before running
        // OT again. (This is all for the purpose of preventing two copies
        // of OT running at the same time and corrupting the data folder.)
        //
        auto strPIDPath = String::Factory();
        OTPaths::AppendFile(
            strPIDPath, dataPath, String::Factory(SERVER_PID_FILENAME));

        std::ifstream pid_infile(strPIDPath->Get());

        // 2. (IF FILE EXISTS WITH ANY PID INSIDE, THEN DIE.)
        if (pid_infile.is_open()) {
            std::uint32_t old_pid = 0;
            pid_infile >> old_pid;
            pid_infile.close();

            // There was a real PID in there.
            if (old_pid != 0) {
                std::uint64_t lPID = old_pid;
                otErr << "\n\n\nIS OPEN-TRANSACTIONS ALREADY RUNNING?\n\n"
                         "I found a PID ("
                      << lPID
                      << ") in the data lock file, located "
                         "at: "
                      << strPIDPath->Get()
                      << "\n\n"
                         "If the OT process with PID "
                      << lPID
                      << " is truly not running anymore, "
                         "then just ERASE THAT FILE and then RESTART.\n";
                exit(-1);
            }
            // Otherwise, though the file existed, the PID within was 0.
            // (Meaning the previous instance of OT already set it to 0 as
            // it was shutting down.)
        }
        // Next let's record our PID to the same file, so other copies of OT
        // can't trample on US.

        // 3. GET THE CURRENT (ACTUAL) PROCESS ID.
        std::uint64_t the_pid = 0;

#ifdef _WIN32
        the_pid = GetCurrentProcessId();
#else
        the_pid = getpid();
#endif

        // 4. OPEN THE FILE IN WRITE MODE, AND SAVE THE PID TO IT.
        std::ofstream pid_outfile(strPIDPath->Get());

        if (pid_outfile.is_open()) {
            pid_outfile << the_pid;
            pid_outfile.close();
        } else {
            otErr << "Failed trying to open data locking file (to "
                     "store PID "
                  << the_pid << "): " << strPIDPath->Get() << "\n";
        }
    }
    OTDB::InitDefaultStorage(OTDB_DEFAULT_STORAGE, OTDB_DEFAULT_PACKER);

    // Load up the transaction number and other Server data members.
    bool mainFileExists =
        WalletFilename().Exists()
            ? OTDB::Exists(
                  manager_.DataFolder(), ".", WalletFilename().Get(), "", "")
            : false;

    if (false == mainFileExists) {
        if (readOnly) {
            otErr << "Error: Main file non-existent (" << WalletFilename().Get()
                  << "). "
                     "Plus, unable to create, since read-only flag is set.\n";
            OT_FAIL;
        } else {
            CreateMainFile(mainFileExists);
        }
    }

    if (mainFileExists) {
        if (false == mainFile_.LoadMainFile(readOnly)) {
            otErr << "Error in Loading Main File, re-creating.\n";
            OTDB::EraseValueByKey(
                manager_.DataFolder(), ".", WalletFilename().Get(), "", "");
            CreateMainFile(mainFileExists);

            OT_ASSERT(mainFileExists);

            if (!mainFile_.LoadMainFile(readOnly)) { OT_FAIL; }
        }
    }

    auto password = manager_.Crypto().Encode().Nonce(16);
    auto notUsed = String::Factory();
    bool ignored;
    manager_.Config().CheckSet_str(
        String::Factory("permissions"),
        String::Factory("admin_password"),
        password,
        notUsed,
        ignored);
    manager_.Config().Save();

    // With the Server's private key loaded, and the latest transaction number
    // loaded, and all the various other data (contracts, etc) the server is now
    // ready for operation!
}

bool Server::LoadServerNym(const Identifier& nymID)
{
    auto nym = manager_.Wallet().Nym(nymID);

    if (false == bool(nym)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Server nym does not exist."
              << std::endl;

        return false;
    }

    m_nymServer = nym;

    OT_ASSERT(m_nymServer);

    return true;
}

// msg, the request msg from payer, which is attached WHOLE to the Nymbox
// receipt. contains payment already. or pass pPayment instead: we will create
// our own msg here (with payment inside) to be attached to the receipt.
// szCommand for passing payDividend (as the message command instead of
// sendNymInstrument, the default.)
bool Server::SendInstrumentToNym(
    const Identifier& NOTARY_ID,
    const Identifier& SENDER_NYM_ID,
    const Identifier& RECIPIENT_NYM_ID,
    const OTPayment& pPayment,
    const char* szCommand)
{
    OT_ASSERT(pPayment.IsValid());

    // If a payment was passed in (for us to use it to construct pMsg, which is
    // nullptr in the case where payment isn't nullptr)
    // Then we grab it in string form, so we can pass it on...
    auto strPayment = String::Factory();
    const bool bGotPaymentContents = pPayment.GetPaymentContents(strPayment);

    if (!bGotPaymentContents) {
        otErr << __FUNCTION__ << ": Error GetPaymentContents Failed";
    }

    const bool bDropped = DropMessageToNymbox(
        NOTARY_ID,
        SENDER_NYM_ID,
        RECIPIENT_NYM_ID,
        transactionType::instrumentNotice,
        nullptr,
        strPayment,
        szCommand);

    return bDropped;
}

bool Server::SendInstrumentToNym(
    const Identifier& NOTARY_ID,
    const Identifier& SENDER_NYM_ID,
    const Identifier& RECIPIENT_NYM_ID,
    const Message& pMsg)
{
    return DropMessageToNymbox(
        NOTARY_ID,
        SENDER_NYM_ID,
        RECIPIENT_NYM_ID,
        transactionType::instrumentNotice,
        pMsg);
}

bool Server::DropMessageToNymbox(
    const Identifier& notaryID,
    const Identifier& senderNymID,
    const Identifier& recipientNymID,
    transactionType transactionType,
    const Message& msg)
{
    return DropMessageToNymbox(
        notaryID, senderNymID, recipientNymID, transactionType, &msg);
}

// Can't be static (transactor_.issueNextTransactionNumber is called...)
//
// About pMsg...
// (Normally) when you send a cheque to someone, you encrypt it inside an
// envelope, and that
// envelope is attached to a OTMessage (sendNymInstrument) and sent to the
// server. The server
// takes your entire OTMessage and attaches it to an instrumentNotice
// (OTTransaction) which is
// added to the recipient's Nymbox.
// In that case, just pass the pointer to the incoming message here as pMsg, and
// the OT Server
// will attach it as described.
// But let's say you are paying a dividend. The server can't just attach your
// dividend request in
// that case. Normally the recipient's cheque is already in the request. But
// with a dividend, there
// could be a thousand recipients, and their individual vouchers are only
// generated and sent AFTER
// the server receives the "pay dividend" request.
// Therefore in that case, nullptr would be passed for pMsg, meaning that inside
// this function we have
// to generate our own OTMessage "from the server" instead of "from the sender".
// After all, the server's
// private key is the only signer we have in here. And the recipient will be
// expecting to have to
// open a message, so we must create one to give him. So if pMsg is nullptr,
// then
// this function will
// create a message "from the server", containing the instrument, and drop it
// into the recipient's nymbox
// as though it were some incoming message from a normal user.
// This message, in the case of payDividend, should be an "payDividendResponse"
// message, "from" the server
// and "to" the recipient. The payment instrument must be attached to that new
// message, and therefore it
// must be passed into this function.
// Of course, if pMsg was not nullptr, that means the message (and instrument
// inside of it) already exist,
// so no instrument would need to be passed. But if pMsg IS nullptr, that means
// the
// msg must be generated,
// and thus the instrument MUST be passed in, so that that can be done.
// Therefore the instrument will sometimes be passed in, and sometimes not.
// Therefore the instrument must
// be passed as a pointer.
//
// Conclusion: if pMsg is passed in, then pass a null instrument. (Since the
// instrument is already on pMsg.)
//                (And since the instrument defaults to nullptr, this makes pMsg
// the final argument in the call.)
//         but if pMsg is nullptr, then you must pass the payment instrument as
// the
// next argument. (So pMsg can be created with it.)
// Note: you cannot pass BOTH, or the instrument will simply be ignored, since
// it's already assumed to be in pMsg.
// You might ask: what about the original request then, doesn't the recipient
// get a copy of that? Well, maybe we
// pass it in here and attach it to the new message. Or maybe we just set it as
// the voucher memo.
//
bool Server::DropMessageToNymbox(
    const Identifier& NOTARY_ID,
    const Identifier& SENDER_NYM_ID,
    const Identifier& RECIPIENT_NYM_ID,
    transactionType theType,
    const Message* pMsg,
    const String& pstrMessage,
    const char* szCommand)  // If you pass something here, it will
{                           // replace pMsg->m_strCommand below.
    OT_ASSERT_MSG(
        !((nullptr == pMsg) && (pstrMessage.empty())),
        "pMsg and pstrMessage -- these can't BOTH be nullptr.\n");
    // ^^^ Must provde one or the other.
    OT_ASSERT_MSG(
        !((nullptr != pMsg) && (!pstrMessage.empty())),
        "pMsg and pstrMessage -- these can't BOTH be not-nullptr.\n");
    // ^^^ Can't provide both.
    std::int64_t lTransNum{0};
    const bool bGotNextTransNum =
        transactor_.issueNextTransactionNumber(lTransNum);

    if (!bGotNextTransNum) {
        otErr << __FUNCTION__
              << ": Error: failed trying to get next transaction number.\n";
        return false;
    }
    switch (theType) {
        case transactionType::message:
            break;
        case transactionType::instrumentNotice:
            break;
        default:
            otErr
                << __FUNCTION__
                << ": Unexpected transactionType passed here (expected message "
                   "or instrumentNotice.)\n";
            return false;
    }
    // If pMsg was not already passed in here, then
    // create pMsg using pstrMessage.
    //
    std::unique_ptr<Message> theMsgAngel;
    const Message* message{nullptr};

    if (nullptr == pMsg) {
        theMsgAngel.reset(manager_.Factory().Message().release());

        if (nullptr != szCommand)
            theMsgAngel->m_strCommand = String::Factory(szCommand);
        else {
            switch (theType) {
                case transactionType::message:
                    theMsgAngel->m_strCommand =
                        String::Factory("sendNymMessage");
                    break;
                case transactionType::instrumentNotice:
                    theMsgAngel->m_strCommand =
                        String::Factory("sendNymInstrument");
                    break;
                default:
                    break;  // should never happen.
            }
        }
        theMsgAngel->m_strNotaryID = String::Factory(m_notaryID);
        theMsgAngel->m_bSuccess = true;
        SENDER_NYM_ID.GetString(theMsgAngel->m_strNymID);
        RECIPIENT_NYM_ID.GetString(
            theMsgAngel->m_strNymID2);  // set the recipient ID
                                        // in theMsgAngel to match our
                                        // recipient ID.
        // Load up the recipient's public key (so we can encrypt the envelope
        // to him that will contain the payment instrument.)
        //
        ConstNym nymRecipient = manager_.Wallet().Nym(RECIPIENT_NYM_ID);

        const crypto::key::Asymmetric& thePubkey =
            nymRecipient->GetPublicEncrKey();
        // Wrap the message up into an envelope and attach it to theMsgAngel.
        //
        OTEnvelope theEnvelope;

        theMsgAngel->m_ascPayload.Release();

        if ((!pstrMessage.empty()) &&
            theEnvelope.Seal(thePubkey, pstrMessage) &&  // Seal pstrMessage
                                                         // into theEnvelope,
            // using nymRecipient's
            // public key.
            theEnvelope.GetCiphertext(theMsgAngel->m_ascPayload))  // Grab the
                                                                   // sealed
                                                                   // version as
        // base64-encoded string, into
        // theMsgAngel->m_ascPayload.
        {
            theMsgAngel->SignContract(*m_nymServer);
            theMsgAngel->SaveContract();
        } else {
            otErr << __FUNCTION__
                  << ": Failed trying to seal envelope containing theMsgAngel "
                     "(or while grabbing the base64-encoded result.)\n";
            return false;
        }

        // By this point, pMsg is all set up, signed and saved. Its payload
        // contains
        // the envelope (as base64) containing the encrypted message.

        message = theMsgAngel.get();
    } else {
        message = pMsg;
    }
    //  else // pMsg was passed in, so it's not nullptr. No need to create it
    // ourselves like above. (pstrMessage should be nullptr anyway in this
    // case.)
    //  {
    //       // Apparently no need to do anything in here at all.
    //  }
    // Grab a string copy of message.
    //
    const auto strInMessage = String::Factory(*message);
    auto theLedger{manager_.Factory().Ledger(
        RECIPIENT_NYM_ID, RECIPIENT_NYM_ID, NOTARY_ID)};  // The
                                                          // recipient's
                                                          // Nymbox.
    // Drop in the Nymbox
    if ((theLedger->LoadNymbox() &&  // I think this loads the box receipts too,
                                     // since I didn't call "LoadNymboxNoVerify"
         //          theLedger.VerifyAccount(m_nymServer)    &&    // This loads
         // all the Box Receipts, which is unnecessary.
         theLedger->VerifyContractID() &&  // Instead, we'll verify the IDs and
                                           // Signature only.
         theLedger->VerifySignature(*m_nymServer))) {
        // Create the instrumentNotice to put in the Nymbox.
        auto pTransaction{manager_.Factory().Transaction(
            *theLedger, theType, originType::not_applicable, lTransNum)};

        if (false != bool(pTransaction))  // The above has an OT_ASSERT within,
                                          // but I just like to check my
                                          // pointers.
        {
            // NOTE: todo: SHOULD this be "in reference to" itself? The reason,
            // I assume we are doing this
            // is because there is a reference STRING so "therefore" there must
            // be a reference # as well. Eh?
            // Anyway, it must be understood by those involved that a message is
            // stored inside. (Which has no transaction #.)

            pTransaction->SetReferenceToNum(lTransNum);  // <====== Recipient
                                                         // RECEIVES entire
                                                         // incoming message as
                                                         // string here, which
                                                         // includes the sender
                                                         // user ID,
            pTransaction->SetReferenceString(
                strInMessage);  // and has an OTEnvelope in the payload. Message
            // is signed by sender, and envelope is encrypted
            // to recipient.

            pTransaction->SignContract(*m_nymServer);
            pTransaction->SaveContract();
            std::shared_ptr<OTTransaction> transaction{pTransaction.release()};
            theLedger->AddTransaction(transaction);  // Add the message
                                                     // transaction to the
                                                     // nymbox. (It will
                                                     // cleanup.)

            theLedger->ReleaseSignatures();
            theLedger->SignContract(*m_nymServer);
            theLedger->SaveContract();
            theLedger->SaveNymbox(Identifier::Factory());  // We don't grab the
                                                           // Nymbox hash here,
                                                           // since
            // nothing important changed (just a message
            // was sent.)

            // Any inbox/nymbox/outbox ledger will only itself contain
            // abbreviated versions of the receipts, including their hashes.
            //
            // The rest is stored separately, in the box receipt, which is
            // created
            // whenever a receipt is added to a box, and deleted after a receipt
            // is removed from a box.
            //
            transaction->SaveBoxReceipt(*theLedger);
            notification_socket_->Push(
                nymbox_push(RECIPIENT_NYM_ID, *transaction));

            return true;
        } else  // should never happen
        {
            const auto strRecipientNymID = String::Factory(RECIPIENT_NYM_ID);
            otErr
                << __FUNCTION__
                << ": Failed while trying to generate transaction in order to "
                   "add a message to Nymbox: "
                << strRecipientNymID->Get() << "\n";
        }
    } else {
        const auto strRecipientNymID = String::Factory(RECIPIENT_NYM_ID);
        otErr << __FUNCTION__
              << ": Failed while trying to load or verify Nymbox: "
              << strRecipientNymID->Get() << "\n";
    }

    return false;
}

bool Server::GetConnectInfo(
    proto::AddressType& type,
    std::string& strHostname,
    std::uint32_t& nPort) const
{
    auto contract = manager_.Wallet().Server(m_notaryID);

    OT_ASSERT(contract);

    std::string contractHostname{};
    std::uint32_t contractPort{};

    const auto haveEndpoints =
        contract->ConnectInfo(contractHostname, contractPort, type, type);

    OT_ASSERT(haveEndpoints)

    bool notUsed = false;
    std::int64_t port = 0;

    const bool haveIP = manager_.Config().CheckSet_str(
        String::Factory(SERVER_CONFIG_LISTEN_SECTION),
        String::Factory("bindip"),
        String::Factory(DEFAULT_BIND_IP),
        strHostname,
        notUsed);

    const bool havePort = manager_.Config().CheckSet_long(
        String::Factory(SERVER_CONFIG_LISTEN_SECTION),
        String::Factory(SERVER_CONFIG_COMMAND_KEY),
        DEFAULT_COMMAND_PORT,
        port,
        notUsed);

    port = (MAX_TCP_PORT < port) ? DEFAULT_COMMAND_PORT : port;
    port = (MIN_TCP_PORT > port) ? DEFAULT_COMMAND_PORT : port;

    nPort = port;

    manager_.Config().Save();

    return (haveIP && havePort);
}

OTZMQMessage Server::nymbox_push(
    const Identifier& nymID,
    const OTTransaction& item) const
{
    auto output = zmq::Message::Factory();
    output->AddFrame(nymID.str());
    proto::OTXPush push;
    push.set_version(OTX_PUSH_VERSION);
    push.set_type(proto::OTXPUSH_NYMBOX);
    push.set_item(String::Factory(item)->Get());
    output->AddFrame(proto::ProtoAsString(push));

    return output;
}

std::unique_ptr<OTPassword> Server::TransportKey(Data& pubkey) const
{
    auto contract = manager_.Wallet().Server(m_notaryID);

    OT_ASSERT(contract);

    return contract->TransportKey(pubkey);
}

Server::~Server()
{
    // PID -- Set it to 0 in the lock file so the next time we run OT, it knows
    // there isn't
    // another copy already running (otherwise we might wind up with two copies
    // trying to write
    // to the same data folder simultaneously, which could corrupt the data...)
    auto strDataPath = String::Factory(manager_.DataFolder().c_str());

    if (!m_bReadOnly) {
        auto strPIDPath = String::Factory();
        OTPaths::AppendFile(
            strPIDPath, strDataPath, String::Factory(SERVER_PID_FILENAME));
        std::ofstream pid_outfile(strPIDPath->Get());

        if (pid_outfile.is_open()) {
            std::uint32_t the_pid = 0;
            pid_outfile << the_pid;
            pid_outfile.close();
        } else {
            otErr << "Failed trying to open data locking file (to wipe "
                     "PID back to 0): "
                  << strPIDPath->Get() << "\n";
        }
    }
}
}  // namespace opentxs::server
