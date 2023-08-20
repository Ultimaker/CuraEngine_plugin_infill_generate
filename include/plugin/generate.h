#ifndef PLUGIN_GENERATE_H
#define PLUGIN_GENERATE_H

#include "infill/infill_generator.h"
#include "plugin/broadcast.h"
#include "plugin/metadata.h"
#include "plugin/settings.h"

#include <boost/asio/awaitable.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <coroutine>
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
            grpc::Status status = grpc::Status::OK;
            cura::plugins::slots::infill::v0::generate::CallRequest request;
            grpc::ServerAsyncResponseWriter<Rsp> writer{ &server_context };
            co_await agrpc::request(&T::RequestCall, *generate_service, server_context, request, writer, boost::asio::use_awaitable);
            const auto pattern_setting = Settings::getPattern(request.pattern(), metadata->plugin_name);
            const auto tile_type_setting = Settings::retrieveSettings("tile_shape", request, metadata);
            const auto tile_size_setting = Settings::retrieveSettings("tile_size", request, metadata);
            const auto absolute_tiles_setting = Settings::retrieveSettings("absolute_tiles", request, metadata);

            if (! pattern_setting.has_value() || ! tile_type_setting.has_value() || ! tile_size_setting.has_value() || ! absolute_tiles_setting.has_value())
            {
                spdlog::error(
                    "pattern: {}, tile_shape: {}, tile size: {}, absolute tiles: {}",
                    pattern_setting.has_value(),
                    tile_type_setting.has_value(),
                    tile_size_setting.has_value(),
                    absolute_tiles_setting.has_value());
                status = grpc::Status(
                    grpc::StatusCode::INTERNAL,
                    fmt::format(
                        "Plugin could not retrieve settings! pattern: {}, tile_shape: {}, tile size: {}, absolute tiles: {}",
                        pattern_setting.has_value(),
                        tile_type_setting.has_value(),
                        tile_size_setting.has_value(),
                        absolute_tiles_setting.has_value()));
            }
            if (! status.ok())
            {
                co_await agrpc::finish_with_error(writer, status, boost::asio::use_awaitable);
                continue;
            }

            const auto tile_type = Settings::getTileType(tile_type_setting.value());
            const int64_t tile_size = std::stoll(tile_size_setting.value()) * 1000;
            const bool absolute_tiles = absolute_tiles_setting.value() == "True" || absolute_tiles_setting.value() == "true";

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

            Rsp response;

            ClipperLib::Paths lines;
            ClipperLib::Paths polys;
            try
            {
                auto [lines_, polys_] = generator.generate(outlines, pattern_setting.value(), tile_size, absolute_tiles, tile_type);
                lines = std::move(lines_);
                polys = std::move(polys_);
            }
            catch (const std::exception& e)
            {
                spdlog::error("Error: {}", e.what());
                status = grpc::Status(grpc::StatusCode::INTERNAL, static_cast<std::string>(e.what()));
            }
            if (! status.ok())
            {
                co_await agrpc::finish_with_error(writer, status, boost::asio::use_awaitable);
                continue;
            }

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

            co_await agrpc::finish(writer, response, status, boost::asio::use_awaitable);
        }
    }
};

} // namespace plugin

#endif // PLUGIN_GENERATE_H
