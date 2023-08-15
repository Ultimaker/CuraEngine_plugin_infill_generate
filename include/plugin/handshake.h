#ifndef PLUGIN_HANDSHAKE_H
#define PLUGIN_HANDSHAKE_H

#include "cura/plugins/slots/handshake/v0/handshake.grpc.pb.h"
#include "cura/plugins/v0/slot_id.pb.h"
#include "plugin/metadata.h"
#include "plugin/settings.h"

#include <agrpc/rpc.hpp>
#include <boost/asio/awaitable.hpp>
#include <spdlog/spdlog.h>

#include <coroutine>
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
            spdlog::info("Slot ID: {}, slot version range: {}, plugin name: {}, plugin version: {}", static_cast<int>(request.slot_id()), request.version_range(), request.plugin_name(), request.plugin_version());

            const bool exists = Settings::validatePlugin(request, metadata);
            if (!exists)
            {
                grpc::Status status = grpc::Status(grpc::StatusCode::INTERNAL, "Plugin could not be validated, handshake failed!");
                co_await agrpc::finish_with_error(writer, status, boost::asio::use_awaitable);
                continue;
            }

            cura::plugins::slots::handshake::v0::CallResponse response;
            response.set_plugin_name(static_cast<std::string>(metadata->plugin_name));
            response.set_slot_version(static_cast<std::string>(metadata->slot_version));
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
