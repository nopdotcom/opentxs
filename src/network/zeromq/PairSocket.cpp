// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "PairSocket.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <zmq.h>

template class opentxs::Pimpl<opentxs::network::zeromq::PairSocket>;

// #define PAIR_ENDPOINT_PATH "pair"
// #define PAIR_ENDPOINT_INSTANCE -1
// #define PAIR_ENDPOINT_VERSION 1

#define OT_METHOD "opentxs::network::zeromq::implementation::PairSocket::"

namespace opentxs::network::zeromq
{
OTZMQPairSocket PairSocket::Factory(
    const class Context& context,
    const ListenCallback& callback)
{
    return OTZMQPairSocket(new implementation::PairSocket(context, callback));
}

OTZMQPairSocket PairSocket::Factory(
    const ListenCallback& callback,
    const PairSocket& peer)
{
    return OTZMQPairSocket(new implementation::PairSocket(callback, peer));
}

OTZMQPairSocket PairSocket::Factory(
    const class Context& context,
    const ListenCallback& callback,
    const std::string& endpoint)
{
    return OTZMQPairSocket(
        new implementation::PairSocket(context, callback, endpoint));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
PairSocket::PairSocket(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback,
    const std::string& endpoint,
    const bool listener,
    const bool startThread)
    : ot_super(
          context,
          SocketType::Pair,
          (listener) ? Socket::Direction::Bind : Socket::Direction::Connect)
    , Bidirectional(context, lock_, socket_, startThread)
    , callback_(callback)
    , endpoint_(endpoint)
    , bind_(listener)
{
    OT_ASSERT(false == endpoint_.empty())

    bool init{false};
    Lock lock(lock_);

    if (bind_) {
        init = Socket::bind(lock, endpoint_);
        otInfo << OT_METHOD << __FUNCTION__ << ": Bound to " << endpoint_
               << std::endl;
    } else {
        init = start_client(lock, endpoint_);
        otInfo << OT_METHOD << __FUNCTION__ << ": Connected to " << endpoint_
               << std::endl;
    }

    OT_ASSERT(init)
}

PairSocket::PairSocket(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback,
    const bool startThread)
    : PairSocket(
          context,
          callback,
          context.BuildEndpoint(
              PAIR_ENDPOINT_PATH,
              PAIR_ENDPOINT_INSTANCE,
              PAIR_ENDPOINT_VERSION,
              Identifier::Random()->str()),
          true,
          startThread)
{
}

PairSocket::PairSocket(
    const zeromq::ListenCallback& callback,
    const zeromq::PairSocket& peer,
    const bool startThread)
    : PairSocket(peer.Context(), callback, peer.Endpoint(), false, startThread)
{
}

PairSocket::PairSocket(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback,
    const std::string& endpoint)
    : PairSocket(context, callback, endpoint, false, true)
{
}

PairSocket* PairSocket::clone() const
{
    return new PairSocket(context_, callback_, endpoint_, bind_, false);
}

const std::string& PairSocket::Endpoint() const { return endpoint_; }

bool PairSocket::have_callback() const { return true; }

void PairSocket::process_incoming(const Lock& lock, Message& message)
{
    OT_ASSERT(verify_lock(lock))

    callback_.Process(message);
}

bool PairSocket::Send(const std::string& data) const
{
    return Send(Message::Factory(data));
}

bool PairSocket::Send(const opentxs::Data& data) const
{
    return Send(Message::Factory(data));
}

bool PairSocket::Send(zeromq::Message& data) const
{
    return queue_message(data);
}

bool PairSocket::Start(const std::string&) const { return false; }

PairSocket::~PairSocket() {}
}  // namespace opentxs::network::zeromq::implementation
