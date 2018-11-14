// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_ENDPOINTS_HPP
#define OPENTXS_API_ENDPOINTS_HPP

#include "opentxs/Forward.hpp"

#include <string>

#ifdef SWIG
// clang-format off
%rename(ZMQEndpoints) opentxs::api::Endpoints;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace api
{
class Endpoints
{
public:
    /** Account balance update notifications
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any account balance changes.
     *
     *  Messages bodies consist of two frames.
     *   * The first frame contains the account ID as a serialized string
     *   * The second frame contains the account balance as an Amount
     *
     *  This endpoint is active for all session types.
     */
    EXPORT virtual std::string AccountUpdate() const = 0;

    /** Connection state notifications
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any server connection state changes.
     *
     *  Connection state is a boolean value that is true if the connection
     *  has sent at least one message and has not timed out waiting for a reply.
     *
     *  Messages bodies consist of two frames.
     *   * The first frame contains the ID of the notary on the other side of
     *     the connection as a serialized string
     *   * The second frame contains the state as a bool
     *
     *  This endpoint is active for client sessions only.
     */
    EXPORT virtual std::string ConnectionStatus() const = 0;

    /** Contact update notifications
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any contact is modified or updated.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains the contact ID as a serialized string
     *
     *  This endpoint is active for client sessions only.
     */
    EXPORT virtual std::string ContactUpdate() const = 0;

    /** Search for a nym in the DHT
     *
     *  A request or dealer socket can connect to this endpoint to trigger
     *  a DHT search for a nym.
     *
     *  Request messages bodies consist of one frame.
     *
     *   * The frame should contains the nym ID being requested as a serialized
     *     string
     *
     *  Reply messages bodies consist of one frame.
     *
     *   * The frame contains a bool with a true value if the nym id provided
     *     was valid or a false value if the nym id was invalid.
     *
     *  This endpoint is active for all session types.
     */
    EXPORT virtual std::string DhtRequestNym() const = 0;

    /** Search for a notary in the DHT
     *
     *  A request or dealer socket can connect to this endpoint to trigger
     *  a DHT search for a notary.
     *
     *  Request messages bodies consist of one frame.
     *
     *   * The frame should contains the notary ID being requested as a
     * serialized string
     *
     *  Reply messages bodies consist of one frame.
     *
     *   * The frame contains a bool with a true value if the notary id provided
     *     was valid or a false value if the notary id was invalid.
     *
     *  This endpoint is active for all session types.
     */
    EXPORT virtual std::string DhtRequestServer() const = 0;

    /** Search for a unit definition in the DHT
     *
     *  A request or dealer socket can connect to this endpoint to trigger
     *  a DHT search for a unit definition.
     *
     *  Request messages bodies consist of one frame.
     *
     *   * The frame should contains the unit definition ID being requested as a
     * serialized string
     *
     *  Reply messages bodies consist of one frame.
     *
     *   * The frame contains a bool with a true value if the unit definition id
     * provided was valid or a false value if the unit definition id was
     * invalid.
     *
     *  This endpoint is active for all session types.
     */
    EXPORT virtual std::string DhtRequestUnit() const = 0;

    /** Push notification processing
     *
     *  This socket is for use by the Sync and ServerConnection classes only
     */
    EXPORT virtual std::string InternalProcessPushNotification() const = 0;

    /** Push notification initiation
     *
     *  This socket is for use by the Server and MessageProcessor classes only
     */
    EXPORT virtual std::string InternalPushNotification() const = 0;

    /** Issuer update notifications
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any issuer is modified or updated.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains the issuer nym ID as a serialized string
     *
     *  This endpoint is active for client sessions only.
     */
    EXPORT virtual std::string IssuerUpdate() const = 0;

    /** Nym update notifications
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any nym is modified or updated.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains the nym ID as a serialized string
     *
     *  This endpoint is active for all session types.
     */
    EXPORT virtual std::string NymDownload() const = 0;

    /** Node pairing event notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any peer message related to node pairing is received.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains a serialized proto::PairEvent message
     *
     *  This endpoint is active for client sessions only.
     */
    EXPORT virtual std::string PairEvent() const = 0;

    /** Peer reply event notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any peer reply is received.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains a serialized proto::PeerReply message
     *
     *  This endpoint is active for client sessions only.
     */
    EXPORT virtual std::string PeerReplyUpdate() const = 0;

    /** Peer request event notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any peer request is received.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains a serialized proto::PeerRequest message
     *
     *  This endpoint is active for client sessions only.
     */
    EXPORT virtual std::string PeerRequestUpdate() const = 0;

    /** Pending bailment notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  a pending bailment peer request has been received.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains a serialized proto::PeerRequest message
     *
     *  This endpoint is active for client sessions only.
     */
    EXPORT virtual std::string PendingBailment() const = 0;

    /** Server reply notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any server reply is received.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains of the message type as a string
     *
     *  This endpoint is active for client sessions only.
     */
    EXPORT virtual std::string ServerReplyReceived() const = 0;

    /** Server request notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any request message is sent to a notary.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains of the message type as a string
     *
     *  This endpoint is active for client sessions only.
     */
    EXPORT virtual std::string ServerRequestSent() const = 0;

    /** Server contract download notifications
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any new server contract is added to the wallet
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains the server ID as a serialized string
     *
     *  This endpoint is active for all session types.
     */
    EXPORT virtual std::string ServerUpdate() const = 0;

    /** Background task completion notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  a background task has concluded.
     *
     *  Resolution is a boolean value that is true if the task executed
     *  successfully, or false in the event of a failure.
     *
     *  Messages bodies consist of two frames.
     *   * The first frame contains the task ID as a serialized string
     *   * The second frame contains the resolution as a bool
     *
     *  This endpoint is active for client sessions only.
     */
    EXPORT virtual std::string TaskComplete() const = 0;

    /** Activity thread update notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any activity thread for a specified nym receives new activity.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains the thread ID as a serialized string
     *
     *  This endpoint is active for client sessions only.
     */
    EXPORT virtual std::string ThreadUpdate(
        const std::string& thread) const = 0;

    /** UI widget update notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any ui widget updates
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains the widget ID as a serialized string
     *
     *  This endpoint is active for client sessions only.
     */
    EXPORT virtual std::string WidgetUpdate() const = 0;

    /** Account update notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any account receives new activity.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains the account ID as a serialized string
     *
     *  This endpoint is active for client sessions only.
     */
    EXPORT virtual std::string WorkflowAccountUpdate() const = 0;

    EXPORT virtual ~Endpoints() = default;

protected:
    Endpoints() = default;

private:
    Endpoints(const Endpoints&) = delete;
    Endpoints(Endpoints&&) = delete;
    Endpoints& operator=(const Endpoints&) = delete;
    Endpoints& operator=(Endpoints&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
