// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::api::implementation
{
class Endpoints final : public opentxs::api::Endpoints
{
public:
    std::string AccountUpdate() const override;
    std::string ConnectionStatus() const override;
    std::string ContactUpdate() const override;
    std::string DhtRequestNym() const override;
    std::string DhtRequestServer() const override;
    std::string DhtRequestUnit() const override;
    std::string InternalProcessPushNotification() const override;
    std::string InternalPushNotification() const override;
    std::string IssuerUpdate() const override;
    std::string NymDownload() const override;
    std::string PairEvent() const override;
    std::string PeerReplyUpdate() const override;
    std::string PeerRequestUpdate() const override;
    std::string PendingBailment() const override;
    std::string ServerReplyReceived() const override;
    std::string ServerRequestSent() const override;
    std::string ServerUpdate() const override;
    std::string TaskComplete() const override;
    std::string ThreadUpdate(const std::string& thread) const override;
    std::string WidgetUpdate() const override;
    std::string WorkflowAccountUpdate() const override;

    ~Endpoints() = default;

private:
    friend opentxs::Factory;

    const opentxs::network::zeromq::Context& zmq_;
    const int instance_;

    std::string build_inproc_path(const std::string& path, const int version)
        const;
    std::string build_inproc_path(
        const std::string& path,
        const int version,
        const std::string& suffix) const;

    Endpoints(const opentxs::network::zeromq::Context& zmq, const int instance);
    Endpoints() = delete;
    Endpoints(const Endpoints&) = delete;
    Endpoints(Endpoints&&) = delete;
    Endpoints& operator=(const Endpoints&) = delete;
    Endpoints& operator=(Endpoints&&) = delete;
};
}  // namespace opentxs::api::implementation
