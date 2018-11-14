// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "StorageParent.hpp"
#include "Scheduler.hpp"

namespace opentxs::api::implementation
{
class Core : virtual public opentxs::api::Core,
             public api::implementation::StorageParent,
             public Scheduler
{
public:
    const api::Settings& Config() const override { return config_; }
    const api::Crypto& Crypto() const override { return crypto_; }
    const std::string& DataFolder() const override { return data_folder_; }
    const api::network::Dht& DHT() const override;
    const api::Endpoints& Endpoints() const override;
    const api::Factory& Factory() const override;
    int Instance() const override { return instance_; }
#if OT_CRYPTO_WITH_BIP39
    const api::HDSeed& Seeds() const override;
#endif
    const api::storage::Storage& Storage() const override;
    const api::Wallet& Wallet() const override;
    const opentxs::network::zeromq::Context& ZeroMQ() const override
    {
        return zmq_context_;
    }

    virtual ~Core();

protected:
    const opentxs::network::zeromq::Context& zmq_context_;
    const int instance_{0};
    std::unique_ptr<api::Endpoints> endpoints_;
#if OT_CRYPTO_WITH_BIP39
    std::unique_ptr<api::HDSeed> seeds_;
#endif
    std::unique_ptr<api::Factory> factory_;
    std::unique_ptr<api::Wallet> wallet_;
    std::unique_ptr<api::network::Dht> dht_;

    void cleanup();

    Core(
        const api::Native& parent,
        Flag& running,
        const ArgList& args,
        const api::Crypto& crypto,
        const api::Settings& config,
        const opentxs::network::zeromq::Context& zmq,
        const std::string& dataFolder,
        const int instance,
        const bool dhtDefault);

private:
    void storage_gc_hook() override;

    Core() = delete;
    Core(const Core&) = delete;
    Core(Core&&) = delete;
    Core& operator=(const Core&) = delete;
    Core& operator=(Core&&) = delete;
};
}  // namespace opentxs::api::implementation
