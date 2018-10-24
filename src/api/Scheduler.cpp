// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/network/Dht.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Nym.hpp"

#include <ctime>
#include <functional>
#include <limits>

#include "Scheduler.hpp"

//#define OT_METHOD "opentxs::api::implementation::Scheduler::"

namespace opentxs::api::implementation
{
Scheduler::Scheduler(const Flag& running)
    : Lockable()
    , nym_publish_interval_{std::numeric_limits<std::int64_t>::max()}
    , nym_refresh_interval_{std::numeric_limits<std::int64_t>::max()}
    , server_publish_interval_{std::numeric_limits<std::int64_t>::max()}
    , server_refresh_interval_{std::numeric_limits<std::int64_t>::max()}
    , unit_publish_interval_{std::numeric_limits<std::int64_t>::max()}
    , unit_refresh_interval_{std::numeric_limits<std::int64_t>::max()}
    , running_p_{Flag::Factory(running)}
    , running_{running_p_.get()}
    , periodic_task_list_{}
    , periodic_{nullptr}
{
}

void Scheduler::Schedule(
    const std::chrono::seconds& interval,
    const PeriodicTask& task,
    const std::chrono::seconds& last) const
{
    Lock lock(lock_);
    periodic_task_list_.push_back(
        TaskItem{last.count(), interval.count(), task});
}

void Scheduler::Start(
    const api::storage::Storage* const storage,
    const api::network::Dht* const dht)
{
    OT_ASSERT(nullptr != storage);
    OT_ASSERT(nullptr != dht);

    const auto now = std::chrono::seconds(std::time(nullptr));

    Schedule(
        std::chrono::seconds(nym_publish_interval_),
        [=]() -> void {
            NymLambda nymLambda(
                [=](const serializedCredentialIndex& nym) -> void {
                    dht->Insert(nym);
                });
            storage->MapPublicNyms(nymLambda);
        },
        now);

    Schedule(
        std::chrono::seconds(nym_refresh_interval_),
        [=]() -> void {
            NymLambda nymLambda(
                [=](const serializedCredentialIndex& nym) -> void {
                    dht->GetPublicNym(nym.nymid());
                });
            storage->MapPublicNyms(nymLambda);
        },
        (now - std::chrono::seconds(nym_refresh_interval_) / 2));

    Schedule(
        std::chrono::seconds(server_publish_interval_),
        [=]() -> void {
            ServerLambda serverLambda(
                [=](const proto::ServerContract& server) -> void {
                    dht->Insert(server);
                });
            storage->MapServers(serverLambda);
        },
        now);

    Schedule(
        std::chrono::seconds(server_refresh_interval_),
        [=]() -> void {
            ServerLambda serverLambda(
                [=](const proto::ServerContract& server) -> void {
                    dht->GetServerContract(server.id());
                });
            storage->MapServers(serverLambda);
        },
        (now - std::chrono::seconds(server_refresh_interval_) / 2));

    Schedule(
        std::chrono::seconds(unit_publish_interval_),
        [=]() -> void {
            UnitLambda unitLambda(
                [=](const proto::UnitDefinition& unit) -> void {
                    dht->Insert(unit);
                });
            storage->MapUnitDefinitions(unitLambda);
        },
        now);

    Schedule(
        std::chrono::seconds(unit_refresh_interval_),
        [=]() -> void {
            UnitLambda unitLambda(
                [=](const proto::UnitDefinition& unit) -> void {
                    dht->GetUnitDefinition(unit.id());
                });
            storage->MapUnitDefinitions(unitLambda);
        },
        (now - std::chrono::seconds(unit_refresh_interval_) / 2));

    periodic_.reset(new std::thread(&Scheduler::thread, this));
}

void Scheduler::thread()
{
    while (running_) {
        std::time_t now = std::time(nullptr);

        Lock lock(lock_);

        for (auto& [last, interval, task] : periodic_task_list_) {
            if ((now - last) > interval) {
                last = now;
                std::thread taskThread{task};
                taskThread.detach();
            }
        }

        lock.unlock();

        // Storage has its own interval checking.
        storage_gc_hook();

        Log::Sleep(std::chrono::milliseconds(100));
    }
}

Scheduler::~Scheduler()
{
    if (periodic_) { periodic_->join(); }
}
}  // namespace opentxs::api::implementation
