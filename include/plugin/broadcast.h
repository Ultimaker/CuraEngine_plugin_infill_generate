#ifndef PLUGIN_BROADCAST_H
#define PLUGIN_BROADCAST_H

#include "cura/plugins/slots/broadcast/v0/broadcast.grpc.pb.h"
#include "cura/plugins/v0/slot_id.pb.h"
#include "plugin/metadata.h"
#include "plugin/settings.h"

#include <agrpc/asio_grpc.hpp>
#include <boost/asio/awaitable.hpp>
#include <spdlog/spdlog.h>

#include <coroutine>
#include <functional>
#include <memory>
#include <unordered_map>

namespace plugin
{

// TODO: make this more generic, currently using it only for settings broadcast

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
            const google::protobuf::Empty response{};
            co_await agrpc::finish(writer, response, grpc::Status::OK, boost::asio::use_awaitable);
            settings->insert_or_assign(getUuid(server_context), Settings{ request });
        }
    }
};

} // namespace plugin

#endif // PLUGIN_BROADCAST_H
