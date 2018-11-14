// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/Endpoints.hpp"
#include "opentxs/network/zeromq/Context.hpp"

#include "Endpoints.hpp"

#define ENDPOINT_VERSION_1 1

#define ACCOUNT_UPDATE_ENDPOINT "accountupdate"
#define CONNECTION_STATUS_ENDPOINT "connectionstatus"
#define CONTACT_UPDATE_ENDPOINT "contactupdate"
#define DHT_NYM_REQUEST_ENDPOINT "dht/requestnym"
#define DHT_SERVER_REQUEST_ENDPOINT "dht/requestserver"
#define DHT_UNIT_REQUEST_ENDPOINT "dht/requestunit"
#define INTERNAL_PROCESS_PUSH_NOTIFICATION_ENDPOINT "client/receivenotification"
#define INTERNAL_PUSH_NOTIFICATION_ENDPOINT "server/sendnotification"
#define ISSUER_UPDATE_ENDPOINT "issuerupdate"
#define NYM_UPDATE_ENDPOINT "nymupdate"
#define PAIR_EVENT_ENDPOINT "pairevent"
#define PEER_REPLY_UPDATE_ENDPOINT "peerreplyupdate"
#define PEER_REQUEST_UPDATE_ENDPOINT "peerrequestupdate"
#define PENDING_BAILMENT_ENDPOINT "peerrequest/pendingbailment"
#define SERVER_REPLY_RECEIVED_ENDPOINT "reply/received"
#define SERVER_REQUEST_SENT_ENDPOINT "request/sent"
#define SERVER_UPDATE_ENDPOINT "serverupdate"
#define TASK_COMPLETE_ENDPOINT "taskcomplete/"
#define THREAD_UPDATE_ENDPOINT "threadupdate/"
#define WIDGET_UPDATE_ENDPOINT "ui/widgetupdate"
#define WORKFLOW_ACCOUNT_UPDATE_ENDPOINT "ui/workflowupdate/account"

//#define OT_METHOD "opentxs::api::implementation::Endpoints::"

namespace opentxs
{
api::Endpoints* Factory::Endpoints(
    const network::zeromq::Context& zmq,
    const int instance)
{
    return new api::implementation::Endpoints(zmq, instance);
}
}  // namespace opentxs

namespace opentxs::api::implementation
{
Endpoints::Endpoints(
    const opentxs::network::zeromq::Context& zmq,
    const int instance)
    : zmq_(zmq)
    , instance_(instance)
{
}

std::string Endpoints::build_inproc_path(
    const std::string& path,
    const int version) const
{
    return zmq_.BuildEndpoint(path, instance_, version);
}

std::string Endpoints::build_inproc_path(
    const std::string& path,
    const int version,
    const std::string& suffix) const
{
    return zmq_.BuildEndpoint(path, instance_, version, suffix);
}

std::string Endpoints::AccountUpdate() const
{
    return build_inproc_path(ACCOUNT_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::ConnectionStatus() const
{
    return build_inproc_path(CONNECTION_STATUS_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::ContactUpdate() const
{
    return build_inproc_path(CONTACT_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::DhtRequestNym() const
{
    return build_inproc_path(DHT_NYM_REQUEST_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::DhtRequestServer() const
{
    return build_inproc_path(DHT_SERVER_REQUEST_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::DhtRequestUnit() const
{
    return build_inproc_path(DHT_UNIT_REQUEST_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::InternalProcessPushNotification() const
{
    return build_inproc_path(
        INTERNAL_PROCESS_PUSH_NOTIFICATION_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::InternalPushNotification() const
{
    return build_inproc_path(
        INTERNAL_PUSH_NOTIFICATION_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::IssuerUpdate() const
{
    return build_inproc_path(ISSUER_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::NymDownload() const
{
    return build_inproc_path(NYM_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::PairEvent() const
{
    return build_inproc_path(PAIR_EVENT_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::PeerReplyUpdate() const
{
    return build_inproc_path(PEER_REPLY_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::PeerRequestUpdate() const
{
    return build_inproc_path(PEER_REQUEST_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::PendingBailment() const
{
    return build_inproc_path(PENDING_BAILMENT_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::ServerReplyReceived() const
{
    return build_inproc_path(
        SERVER_REPLY_RECEIVED_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::ServerRequestSent() const
{
    return build_inproc_path(SERVER_REQUEST_SENT_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::ServerUpdate() const
{
    return build_inproc_path(SERVER_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::TaskComplete() const
{
    return build_inproc_path(TASK_COMPLETE_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::ThreadUpdate(const std::string& thread) const
{
    return build_inproc_path(
        THREAD_UPDATE_ENDPOINT, ENDPOINT_VERSION_1, thread);
}

std::string Endpoints::WidgetUpdate() const
{
    return build_inproc_path(WIDGET_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}

std::string Endpoints::WorkflowAccountUpdate() const
{
    return build_inproc_path(
        WORKFLOW_ACCOUNT_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}
}  // namespace opentxs::api::implementation
