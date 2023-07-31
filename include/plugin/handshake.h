#ifndef PLUGIN_HANDSHAKE_H
#define PLUGIN_HANDSHAKE_H

#include "cura/plugins/slots/handshake/v0/handshake.grpc.pb.h"
#include "cura/plugins/v0/slot_id.pb.h"

#include <agrpc/rpc.hpp>
#include <boost/asio/awaitable.hpp>
#include <spdlog/spdlog.h>

#include <string_view>

namespace plugin
{

struct Handshake
{
    using service_t = std::shared_ptr<cura::plugins::slots::handshake::v0::HandshakeService::AsyncService>;
    std::string_view slot_version{ "0.1.0-alpha" };
    std::string_view plugin_name{ "curaengine_plugin_infill_generate" };
    std::string_view plugin_version{ "0.1.0-alpha" };
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
            spdlog::info("Slot ID: {}, version_range: {}", static_cast<int>(request.slot_id()), request.version_range());

            cura::plugins::slots::handshake::v0::CallResponse response;
            response.set_plugin_name(static_cast<std::string>(plugin_name));
            response.set_plugin_version(static_cast<std::string>(plugin_version));
            response.set_slot_version(static_cast<std::string>(slot_version));
            response.set_plugin_version(static_cast<std::string>(plugin_version));
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
