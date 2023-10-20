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

#ifndef PLUGIN_PLUGIN_H
#define PLUGIN_PLUGIN_H

#include "plugin/broadcast.h"
#include "plugin/generate.h"
#include "plugin/handshake.h"
#include "plugin/metadata.h"

#include <agrpc/asio_grpc.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <fmt/format.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

#include <memory>
#include <optional>
#include <string_view>
#include <utility>

namespace plugin
{

template<class G>
class Plugin
{
public:
    std::optional<Broadcast> broadcast;
    std::shared_ptr<Metadata> metadata{ std::make_shared<Metadata>() };

    Plugin(std::string_view address, std::string_view port, std::shared_ptr<grpc::ServerCredentials> credentials)
    {
        builder_.AddListeningPort(fmt::format("{}:{}", address, port.data()), std::move(credentials));
    }

    void addHandshakeService(Handshake&& service)
    {
        handshake_ = std::move(service);
        builder_.RegisterService(handshake_.handshake_service.get());
    }

    void addBroadcastService(Broadcast&& service)
    {
        broadcast = std::move(service);
        builder_.RegisterService(broadcast.value().broadcast_service.get());
    }

    void addGenerateService(G&& service)
    {
        generate_ = std::move(service);
        builder_.RegisterService(generate_.value().generate_service.get());
    }

    void start()
    {
        server_ = builder_.BuildAndStart();
    }

    void run()
    {
        boost::asio::co_spawn(context_, handshake_.run(), boost::asio::detached);
        if (broadcast.has_value())
        {
            boost::asio::co_spawn(context_, broadcast.value().run(), boost::asio::detached);
        }
        if (generate_.has_value())
        {
            boost::asio::co_spawn(context_, generate_.value().run(), boost::asio::detached);
        }
        context_.run();
    }

    void stop()
    {
        context_.stop();
        server_->Shutdown();
    }

private:
    grpc::ServerBuilder builder_{};
    agrpc::GrpcContext context_{ builder_.AddCompletionQueue() };
    std::unique_ptr<grpc::Server> server_;
    Handshake handshake_;
    std::optional<G> generate_;
};


} // namespace plugin

#endif // PLUGIN_PLUGIN_H
