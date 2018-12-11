// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/RequestSocket.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/Widget.hpp"

namespace opentxs::ui::implementation
{
template <typename T>
T extract_custom(const CustomData& custom, const std::size_t index = 0)
{
    OT_ASSERT((index + 1) <= custom.size())

    std::unique_ptr<const T> output{static_cast<const T*>(custom.at(index))};

    OT_ASSERT(output)

    return *output;
}

class Widget : virtual public opentxs::ui::Widget
{
public:
    class MessageFunctor
    {
    public:
        virtual void operator()(Widget* object, const network::zeromq::Message&)
            const = 0;

        virtual ~MessageFunctor() = default;

    protected:
        MessageFunctor() = default;

    private:
        MessageFunctor(const MessageFunctor&) = delete;
        MessageFunctor(MessageFunctor&&) = delete;
        MessageFunctor& operator=(const MessageFunctor&) = delete;
        MessageFunctor& operator=(MessageFunctor&&) = delete;
    };

    template <typename T>
    class MessageProcessor : virtual public MessageFunctor
    {
    public:
        typedef void (T::*Function)(const network::zeromq::Message&);

        void operator()(Widget* object, const network::zeromq::Message& message)
            const override
        {
            auto real = dynamic_cast<T*>(object);

            OT_ASSERT(nullptr != real)

            (real->*callback_)(message);
        }

        MessageProcessor(Function callback)
            : callback_(callback)
        {
        }
        MessageProcessor(MessageProcessor&&) = default;
        MessageProcessor& operator=(MessageProcessor&&) = default;

    private:
        Function callback_;

        MessageProcessor() = delete;
        MessageProcessor(const MessageProcessor&) = delete;
        MessageProcessor& operator=(const MessageProcessor&) = delete;
    };

    OTIdentifier WidgetID() const override;

    virtual ~Widget() = default;

protected:
    using ListenerDefinition = std::pair<std::string, MessageFunctor*>;
    using ListenerDefinitions = std::vector<ListenerDefinition>;

    const api::client::Manager& api_;
    const network::zeromq::PublishSocket& publisher_;
    const OTIdentifier widget_id_;

    void setup_listeners(const ListenerDefinitions& definitions);
    void UpdateNotify() const;

    Widget(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const Identifier& id);
    Widget(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher);

private:
    std::vector<OTZMQListenCallback> callbacks_;
    std::vector<OTZMQSubscribeSocket> listeners_;

    Widget() = delete;
    Widget(const Widget&) = delete;
    Widget(Widget&&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget& operator=(Widget&&) = delete;
};  // namespace opentxs::ui::implementation
}  // namespace opentxs::ui::implementation
