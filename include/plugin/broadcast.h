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

#ifndef PLUGIN_BROADCAST_H
#define PLUGIN_BROADCAST_H

#include "cura/plugins/slots/broadcast/v0/broadcast.grpc.pb.h"
#include "cura/plugins/v0/slot_id.pb.h"
#include "plugin/metadata.h"
#include "plugin/settings.h"

#include <agrpc/asio_grpc.hpp>
#include <boost/asio/awaitable.hpp>
#include <spdlog/spdlog.h>

#if __has_include(<coroutine>)
#include <coroutine>
#elif __has_include(<experimental/coroutine>)
#include <experimental/coroutine>
#define USE_EXPERIMENTAL_COROUTINE
#endif
#include <functional>
#include <memory>
#include <unordered_map>

namespace plugin
{

struct Broadcast
{
    using service_t = std::shared_ptr<cura::plugins::slots::broadcast::v0::BroadcastService::AsyncService>;
    using settings_t = std::unordered_map<std::string, Settings>;
    using shared_settings_t = std::shared_ptr<settings_t>;
    service_t broadcast_service{ std::make_shared<cura::plugins::slots::broadcast::v0::BroadcastService::AsyncService>() };
    shared_settings_t settings{ std::make_shared<settings_t>() };

    boost::asio::awaitable<void> run()
    {
        while (true)
        {
            grpc::ServerContext server_context;
            cura::plugins::slots::broadcast::v0::BroadcastServiceSettingsRequest request;
            grpc::ServerAsyncResponseWriter<google::protobuf::Empty> writer{ &server_context };
            co_await agrpc::request(
                &cura::plugins::slots::broadcast::v0::BroadcastService::AsyncService::RequestBroadcastSettings,
                *broadcast_service,
                server_context,
                request,
                writer,
                boost::asio::use_awaitable);
            spdlog::info("Received broadcast settings request");

            grpc::Status status = grpc::Status::OK;
            try
            {
                settings->insert_or_assign(getUuid(server_context), Settings{ request });
            }
            catch (const std::exception& e)
            {
                spdlog::error("Failed to parse broadcast settings request: {}", e.what());
                status = grpc::Status(grpc::StatusCode::INTERNAL, e.what());
            }
            if (! status.ok())
            {
                co_await agrpc::finish_with_error(writer, status, boost::asio::use_awaitable);
                continue;
            }

            const google::protobuf::Empty response{};
            co_await agrpc::finish(writer, response, grpc::Status::OK, boost::asio::use_awaitable);
        }
    }
};

} // namespace plugin

#endif // PLUGIN_BROADCAST_H
