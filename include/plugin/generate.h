#ifndef PLUGIN_GENERATE_H
#define PLUGIN_GENERATE_H

#include "infill/infill_generator.h"
#include "plugin/broadcast.h"
#include "plugin/metadata.h"
#include "plugin/settings.h"

#include <boost/asio/awaitable.hpp>
#include <spdlog/spdlog.h>

#include <memory>

namespace plugin
{

template<class T, class Rsp, class Req>
struct Generate
{
    using service_t = std::shared_ptr<T>;
    service_t generate_service{ std::make_shared<T>() };
    Broadcast::shared_settings_t settings{ std::make_shared<Broadcast::settings_t>() };
    std::shared_ptr<Metadata> metadata{ std::make_shared<Metadata>() };

    boost::asio::awaitable<void> run()
    {
        InfillGenerator generator{};

        while (true)
        {
            grpc::ServerContext server_context;

            cura::plugins::slots::infill::v0::generate::CallRequest request;
            grpc::ServerAsyncResponseWriter<Rsp> writer{ &server_context };
            co_await agrpc::request(&T::RequestCall, *generate_service, server_context, request, writer, boost::asio::use_awaitable);
            auto pattern = Settings::getPattern(request.pattern(), metadata->plugin_name);
            spdlog::info("Received request for pattern: {}", pattern);

            Rsp response;
            auto client_metadata = getUuid(server_context);

            auto msg_outline = request.infill_areas().polygons(0).outline();
            auto outline = geometry::polygon_outer<>{};
            for (auto& point : msg_outline.path())
            {
                outline.push_back({ point.x(), point.y() });
            }

            grpc::Status status = grpc::Status::OK;
            try
            {
                auto poly_lines = generator.generate(outline);

                // convert poly_lines to protobuf response
                auto* poly_lines_msg = response.mutable_polygons();

                auto* path_msg = poly_lines_msg->add_polygons()->mutable_outline();
                for (auto& point : poly_lines)
                {
                    auto* point_msg = path_msg->add_path();
                    point_msg->set_x(point.X);
                    point_msg->set_y(point.Y);
                }
            }
            catch (const std::exception& e)
            {
                spdlog::error("Error: {}", e.what());
                status = grpc::Status(grpc::StatusCode::INTERNAL, static_cast<std::string>(e.what()));
            }

            co_await agrpc::finish(writer, response, status, boost::asio::use_awaitable);
        }
    }
};

} // namespace plugin

#endif // PLUGIN_GENERATE_H
