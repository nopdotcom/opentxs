// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACTIVITYSUMMARYITEM_HPP
#define OPENTXS_UI_ACTIVITYSUMMARYITEM_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Types.hpp"

#include <chrono>
#include <string>

#include "ListRow.hpp"

#ifdef SWIG
// clang-format off
%extend opentxs::ui::ActivitySummaryItem {
    int Timestamp() const
    {
        return std::chrono::system_clock::to_time_t($self->Timestamp());
    }
}
%ignore opentxs::ui::ActivitySummaryItem::Timestamp;
%template(OTUIActivitySummaryItem) opentxs::SharedPimpl<opentxs::ui::ActivitySummaryItem>;
%rename(UIActivitySummaryItem) opentxs::ui::ActivitySummaryItem;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class ActivitySummaryItem : virtual public ListRow
{
public:
    EXPORT virtual std::string DisplayName() const = 0;
    EXPORT virtual std::string ImageURI() const = 0;
    EXPORT virtual std::string Text() const = 0;
    EXPORT virtual std::string ThreadID() const = 0;
    EXPORT virtual std::chrono::system_clock::time_point Timestamp() const = 0;
    EXPORT virtual StorageBox Type() const = 0;

    EXPORT virtual ~ActivitySummaryItem() = default;

protected:
    ActivitySummaryItem() = default;

private:
    ActivitySummaryItem(const ActivitySummaryItem&) = delete;
    ActivitySummaryItem(ActivitySummaryItem&&) = delete;
    ActivitySummaryItem& operator=(const ActivitySummaryItem&) = delete;
    ActivitySummaryItem& operator=(ActivitySummaryItem&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif
