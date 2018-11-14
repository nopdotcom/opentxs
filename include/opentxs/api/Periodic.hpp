// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_PERIODIC_HPP
#define OPENTXS_API_PERIODIC_HPP

#include "opentxs/Forward.hpp"

namespace opentxs
{
namespace api
{
class Periodic
{
public:
    EXPORT virtual bool Cancel(const int task) const = 0;
    EXPORT virtual bool Reschedule(
        const int task,
        const std::chrono::seconds& interval) const = 0;
    /** Adds a task to the periodic task list with the specified interval. By
     * default, schedules for immediate execution.
     *
     * \returns: task identifier which may be used to manage the task
     */
    EXPORT virtual int Schedule(
        const std::chrono::seconds& interval,
        const opentxs::PeriodicTask& task,
        const std::chrono::seconds& last = std::chrono::seconds(0)) const = 0;

protected:
    Periodic() = default;

private:
    Periodic(const Periodic&) = delete;
    Periodic(Periodic&&) = delete;
    Periodic& operator=(const Periodic&) = delete;
    Periodic& operator=(Periodic&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
