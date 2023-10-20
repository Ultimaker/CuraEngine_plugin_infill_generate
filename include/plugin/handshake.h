// BSD 4-Clause License
//
// Copyright (c) 2023, UltiMaker
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// 3. All advertising materials mentioning features or use of this software must
//   display the following acknowledgement:
//     This product includes software developed by UltiMaker.
//
// 4. Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDER "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLUGIN_HANDSHAKE_H
#define PLUGIN_HANDSHAKE_H

#include "cura/plugins/slots/handshake/v0/handshake.grpc.pb.h"
#include "cura/plugins/v0/slot_id.pb.h"
#include "plugin/metadata.h"
#include "plugin/settings.h"

#include <agrpc/rpc.hpp>
#include <boost/asio/awaitable.hpp>
#include <spdlog/spdlog.h>

#if __has_include(<coroutine>)
#include <coroutine>
#elif __has_include(<experimental/coroutine>)
#include <experimental/coroutine>
#define USE_EXPERIMENTAL_COROUTINE
#endif

#include <string_view>

namespace plugin
{

struct Handshake
{
    std::shared_ptr<Metadata> metadata{ std::make_shared<Metadata>() };
    using service_t = std::shared_ptr<cura::plugins::slots::handshake::v0::HandshakeService::AsyncService>;
    std::set<cura::plugins::v0::SlotID> broadcast_subscriptions{ cura::plugins::v0::SlotID::SETTINGS_BROADCAST };
    service_t handshake_service{ std::make_shared<cura::plugins::slots::handshake::v0::HandshakeService::AsyncService>() };

    boost::asio::awaitable<void> run()
    {
        while (true)
        {
            grpc::ServerContext server_context;

            cura::plugins::slots::handshake::v0::CallRequest request;
            grpc::ServerAsyncResponseWriter<cura::plugins::slots::handshake::v0::CallResponse> writer{ &server_context };
            co_await agrpc::request(
                &cura::plugins::slots::handshake::v0::HandshakeService::AsyncService::RequestCall,
                *handshake_service,
                server_context,
                request,
                writer,
                boost::asio::use_awaitable);

            spdlog::info("Received handshake request");
            spdlog::info("Slot ID: {}, slot version: {}, plugin name: {}, plugin version: {}", static_cast<int>(request.slot_id()), request.version(), request.plugin_name(), request.plugin_version());

            const bool exists = Settings::validatePlugin(request, metadata);
            if (!exists)
            {
                grpc::Status status = grpc::Status(grpc::StatusCode::INTERNAL, "Plugin could not be validated, handshake failed!");
                co_await agrpc::finish_with_error(writer, status, boost::asio::use_awaitable);
                continue;
            }

            cura::plugins::slots::handshake::v0::CallResponse response;
            response.set_plugin_name(static_cast<std::string>(metadata->plugin_name));
            response.set_slot_version_range(static_cast<std::string>(metadata->slot_version_range));
            response.set_plugin_version(static_cast<std::string>(metadata->plugin_version));
            for (auto slot_id : broadcast_subscriptions)
            {
                response.mutable_broadcast_subscriptions()->Add(slot_id);
            }

            co_await agrpc::finish(writer, response, grpc::Status::OK, boost::asio::use_awaitable);
        }
    }
};

} // namespace plugin

#endif // PLUGIN_HANDSHAKE_H
