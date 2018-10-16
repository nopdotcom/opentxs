// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "RequestSocket.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/OT.hpp"

#include <zmq.h>
#include "Message.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::RequestSocket>;

#define POLL_MILLISECONDS 1000

#define OT_METHOD "opentxs::network::zeromq::implementation::RequestSocket::"

namespace opentxs::network::zeromq
{
OTZMQRequestSocket RequestSocket::Factory(const class Context& context)
{
    return OTZMQRequestSocket(
        new implementation::RequestSocket(context, OT::Running()));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
RequestSocket::RequestSocket(
    const zeromq::Context& context,
    const Flag& running)
    : ot_super(context, SocketType::Request, Socket::Direction::Connect)
    , CurveClient(lock_, socket_)
    , running_(running)
{
}

RequestSocket* RequestSocket::clone() const
{
    return new RequestSocket(context_, running_);
}

Socket::SendResult RequestSocket::SendRequest(opentxs::Data& input) const
{
    return SendRequest(Message::Factory(input));
}

Socket::SendResult RequestSocket::SendRequest(const std::string& input) const
{
    auto copy = input;

    return SendRequest(Message::Factory(copy));
}

Socket::SendResult RequestSocket::SendRequest(zeromq::Message& request) const
{
    OT_ASSERT(nullptr != socket_);

    Lock lock(lock_);

    SendResult output{opentxs::SendResult::ERROR, Message::Factory()};
    auto& status = output.first;
    auto& reply = output.second;

    if (false == running_) { return output; }

    if (false == send_message(lock, request)) { return output; }

    const auto ready = wait(lock);

    if (false == ready) {
        otErr << OT_METHOD << __FUNCTION__ << ": Receive timeout." << std::endl;
        status = opentxs::SendResult::TIMEOUT;

        return output;
    }

    if (receive_message(lock, reply)) {
        status = opentxs::SendResult::VALID_REPLY;
    }

    return output;
}

bool RequestSocket::SetSocksProxy(const std::string& proxy) const
{
    return set_socks_proxy(proxy);
}

bool RequestSocket::Start(const std::string& endpoint) const
{
    Lock lock(lock_);

    return start_client(lock, endpoint);
}

bool RequestSocket::wait(const Lock& lock) const
{
    OT_ASSERT(verify_lock(lock))

    const auto start = std::chrono::system_clock::now();
    zmq_pollitem_t poll[1];

    while (running_) {
        poll[0].socket = socket_;
        poll[0].events = ZMQ_POLLIN;
        const auto events = zmq_poll(poll, 1, POLL_MILLISECONDS);

        if (0 == events) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": No messages.").Flush();

            const auto now = std::chrono::system_clock::now();

            if ((now - start) > std::chrono::milliseconds(receive_timeout_)) {

                return false;
            } else {
                continue;
            }
        }

        if (0 > events) {
            const auto error = zmq_errno();
            otErr << OT_METHOD << __FUNCTION__
                  << ": Poll error: " << zmq_strerror(error) << std::endl;

            return false;
        }

        return true;
    }

    return false;
}

RequestSocket::~RequestSocket() { shutdown(); }
}  // namespace opentxs::network::zeromq::implementation
