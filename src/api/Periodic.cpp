// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Internal.hpp"

#include "opentxs/api/Periodic.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"

#include <atomic>
#include <chrono>
#include <map>
#include <tuple>
#include <thread>

#include "Periodic.hpp"

namespace opentxs::api::implementation
{
Periodic::Periodic(Flag& running)
    : running_(running)
    , next_id_(0)
    , periodic_lock_()
    , periodic_task_list_()
    , periodic_(&Periodic::thread, this)
{
}

bool Periodic::Cancel(const int task) const
{
    Lock lock(periodic_lock_);
    const auto output = periodic_task_list_.erase(task);

    return 1 == output;
}

bool Periodic::Reschedule(const int task, const std::chrono::seconds& interval)
    const
{
    Lock lock(periodic_lock_);
    auto it = periodic_task_list_.find(task);

    if (periodic_task_list_.end() == it) { return false; }

    std::get<1>(it->second) = interval.count();

    return false;
}

int Periodic::Schedule(
    const std::chrono::seconds& interval,
    const PeriodicTask& task,
    const std::chrono::seconds& last) const
{
    const auto id = ++next_id_;
    Lock lock(periodic_lock_);
    periodic_task_list_.emplace(
        id, TaskItem{last.count(), interval.count(), task});

    return id;
}

void Periodic::Shutdown()
{
    if (periodic_.joinable()) { periodic_.join(); }
}

void Periodic::thread()
{
    while (running_) {
        std::time_t now = std::time(nullptr);
        Lock lock(periodic_lock_);

        for (auto& [key, value] : periodic_task_list_) {
            auto& [last, interval, task] = value;

            if ((now - last) > interval) {
                last = now;
                std::thread taskThread{task};
                taskThread.detach();
            }
        }

        lock.unlock();
        Log::Sleep(std::chrono::milliseconds(100));
    }
}

Periodic::~Periodic() { Shutdown(); }
}  // namespace opentxs::api::implementation
