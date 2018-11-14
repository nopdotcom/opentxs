// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/network/Dht.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/api/HDSeed.hpp"
#endif
#include "opentxs/api/Wallet.hpp"

#include "Core.hpp"

//#define OT_METHOD "opentxs::api::implementation::Core::"

namespace opentxs::api::implementation
{
Core::Core(
    const api::Native& parent,
    Flag& running,
    const ArgList& args,
    const api::Crypto& crypto,
    const api::Settings& config,
    const opentxs::network::zeromq::Context& zmq,
    const std::string& dataFolder,
    const int instance,
    const bool dhtDefault)
    : StorageParent(running, args, crypto, config, dataFolder)
    , Scheduler(parent, running)
    , zmq_context_(zmq)
    , instance_(instance)
    , endpoints_(opentxs::Factory::Endpoints(zmq_context_, instance_))
#if OT_CRYPTO_WITH_BIP39
    , seeds_(opentxs::Factory::HDSeed(
          crypto_.Symmetric(),
          *storage_,
          crypto_.BIP32(),
          crypto_.BIP39(),
          crypto_.AES()))
#endif
    , factory_(opentxs::Factory::FactoryAPI(*this))
    , wallet_(nullptr)
    , dht_(opentxs::Factory::Dht(
          dhtDefault,
          *this,
          nym_publish_interval_,
          nym_refresh_interval_,
          server_publish_interval_,
          server_refresh_interval_,
          unit_publish_interval_,
          unit_refresh_interval_))
{
    OT_ASSERT(endpoints_);
    OT_ASSERT(seeds_);
    OT_ASSERT(factory_);
    OT_ASSERT(dht_)
}

const api::network::Dht& Core::DHT() const
{
    OT_ASSERT(dht_)

    return *dht_;
}

const api::Endpoints& Core::Endpoints() const
{
    OT_ASSERT(endpoints_)

    return *endpoints_;
}

void Core::cleanup()
{
    dht_.reset();
    wallet_.reset();
    factory_.reset();
#if OT_CRYPTO_WITH_BIP39
    seeds_.reset();
#endif
    endpoints_.reset();
}

const api::Factory& Core::Factory() const
{
    OT_ASSERT(factory_)

    return *factory_;
}

const api::HDSeed& Core::Seeds() const
{
    OT_ASSERT(seeds_);

    return *seeds_;
}

const api::storage::Storage& Core::Storage() const
{
    OT_ASSERT(storage_)

    return *storage_;
}

void Core::storage_gc_hook()
{
    if (storage_) { storage_->RunGC(); }
}

const api::Wallet& Core::Wallet() const
{
    OT_ASSERT(wallet_);

    return *wallet_;
}

Core::~Core() { cleanup(); }
}  // namespace opentxs::api::implementation
