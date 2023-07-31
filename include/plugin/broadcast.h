#ifndef PLUGIN_BROADCAST_H
#define PLUGIN_BROADCAST_H

#include "cura/plugins/slots/broadcast/v0/broadcast.grpc.pb.h"
#include "cura/plugins/v0/slot_id.pb.h"

#include <boost/asio/awaitable.hpp>
#include <spdlog/spdlog.h>

#include <functional>
#include <memory>
#include <unordered_map>

namespace plugin
{

// TODO: make this more generic, currently using it only for settings broadcast

struct Broadcast
{
    using service_t = std::shared_ptr<cura::plugins::slots::broadcast::v0::BroadcastService::AsyncService>;
    using settings_t = std::shared_ptr<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>;
    service_t broadcast_service{ std::make_shared<cura::plugins::slots::broadcast::v0::BroadcastService::AsyncService>() };
    settings_t settings{ std::make_shared<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>() };

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
            const google::protobuf::Empty response{};
            co_await agrpc::finish(writer, response, grpc::Status::OK, boost::asio::use_awaitable);

            auto c_uuid = server_context.client_metadata().find("cura-engine-uuid");
            if (c_uuid == server_context.client_metadata().end())
            {
                spdlog::warn("cura-engine-uuid not found in client metadata");
                continue;
            }
            const std::string client_metadata = std::string{ c_uuid->second.data(), c_uuid->second.size() };

            // We create a new settings map for this uuid
            std::unordered_map<std::string, std::string> uuid_settings;

            // We insert all the global settings from the request to the uuid_settings map
            for (const auto& [key, value] : request.global_settings().settings())
            {
                uuid_settings.emplace(key, value);
            }

            // We save the settings for this uuid in the global settings map
            settings->at(client_metadata) = uuid_settings;
        }
    }
};

} // namespace plugin

#endif // PLUGIN_BROADCAST_H
