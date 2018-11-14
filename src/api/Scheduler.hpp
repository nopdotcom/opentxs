// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/Native.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Lockable.hpp"

#include <cstdint>
#include <list>
#include <memory>
#include <tuple>
#include <thread>

namespace opentxs::api::implementation
{
class Scheduler : virtual public api::Periodic, public Lockable
{
public:
    bool Cancel(const int task) const override { return parent_.Cancel(task); }
    bool Reschedule(const int task, const std::chrono::seconds& interval)
        const override
    {
        return parent_.Reschedule(task, interval);
    }
    int Schedule(
        const std::chrono::seconds& interval,
        const PeriodicTask& task,
        const std::chrono::seconds& last) const override
    {
        return parent_.Schedule(interval, task, last);
    }

    virtual ~Scheduler();

protected:
    std::int64_t nym_publish_interval_{0};
    std::int64_t nym_refresh_interval_{0};
    std::int64_t server_publish_interval_{0};
    std::int64_t server_refresh_interval_{0};
    std::int64_t unit_publish_interval_{0};
    std::int64_t unit_refresh_interval_{0};
    const api::Native& parent_;
    Flag& running_;

    void Start(
        const api::storage::Storage* const storage,
        const api::network::Dht* const dht);

    Scheduler(const api::Native& parent, Flag& running);

private:
    std::thread periodic_;

    virtual void storage_gc_hook() = 0;

    Scheduler() = delete;
    Scheduler(const Scheduler&) = delete;
    Scheduler(Scheduler&&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;
    Scheduler& operator=(Scheduler&&) = delete;

    void thread();
};
}  // namespace opentxs::api::implementation
