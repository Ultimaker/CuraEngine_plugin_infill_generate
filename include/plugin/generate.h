#ifndef PLUGIN_GENERATE_H
#define PLUGIN_GENERATE_H

#include "infill/infill_generator.h"
#include "plugin/broadcast.h"
#include "plugin/metadata.h"
#include "plugin/settings.h"

#include <boost/asio/awaitable.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>
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
    std::filesystem::path tiles_path;
    infill::InfillGenerator generator{ .tiles_path = tiles_path };

    boost::asio::awaitable<void> run()
    {
        while (true)
        {
            grpc::ServerContext server_context;

            cura::plugins::slots::infill::v0::generate::CallRequest request;
            grpc::ServerAsyncResponseWriter<Rsp> writer{ &server_context };
            co_await agrpc::request(&T::RequestCall, *generate_service, server_context, request, writer, boost::asio::use_awaitable);
            const auto pattern = Settings::getPattern(request.pattern(), metadata->plugin_name);
            const auto tile_type = Settings::getTileType(request.settings().settings().at(Settings::settingKey("tile_shape", metadata->plugin_name, metadata->plugin_version)));
            const int64_t tile_size = std::stoll(request.settings().settings().at(Settings::settingKey("tile_size", metadata->plugin_name, metadata->plugin_version))) * 1000;
            const bool absolute_tiles = request.settings().settings().at(Settings::settingKey("absolute_tiles", metadata->plugin_name, metadata->plugin_version)) == "True";

            Rsp response;
            auto client_metadata = getUuid(server_context);

            auto outlines = std::vector<infill::geometry::polygon_outer<>>{};
            for (const auto& msg_outline : request.infill_areas().polygons())
            {
                auto outline = infill::geometry::polygon_outer<>{};
                for (const auto& point : msg_outline.outline().path())
                {
                    outline.push_back({ point.x(), point.y() });
                }
                outlines.push_back(outline);
                for (const auto& hole : msg_outline.holes())
                {
                    auto hole_outline = infill::geometry::polygon_outer<>{};
                    for (const auto& point : hole.path())
                    {
                        hole_outline.push_back({ point.x(), point.y() });
                    }
                    outlines.push_back(hole_outline);
                }
            }

            grpc::Status status = grpc::Status::OK;
            try
            {
                auto [lines, polys] = generator.generate(outlines, pattern, tile_size, absolute_tiles, tile_type);

                // convert poly_lines to protobuf response
                auto* poly_lines_msg = response.mutable_poly_lines();
                for (auto& poly_line : lines)
                {
                    auto* path_msg = poly_lines_msg->add_paths();
                    for (auto& point : poly_line)
                    {
                        auto* point_msg = path_msg->add_path();
                        point_msg->set_x(point.X);
                        point_msg->set_y(point.Y);
                    }
                }

                auto* polygons_msg = response.mutable_polygons();

                for (const auto& pp : polys)
                {
                    auto* path_msg = polygons_msg->add_polygons()->mutable_outline();
                    for (const auto& point : pp)
                    {
                        auto* point_msg = path_msg->add_path();
                        point_msg->set_x(point.X);
                        point_msg->set_y(point.Y);
                    }
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
