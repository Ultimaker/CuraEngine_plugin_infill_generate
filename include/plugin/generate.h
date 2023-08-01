#ifndef PLUGIN_GENERATE_H
#define PLUGIN_GENERATE_H

#include "infill/infill_generator.h"
#include "plugin/broadcast.h"

#include <boost/asio/awaitable.hpp>

#include <memory>

namespace plugin
{

template<class T, class Rsp, class Req>
struct Generate
{
    using service_t = std::shared_ptr<T>;
    service_t generate_service{ std::make_shared<T>() };
    Broadcast::shared_settings_t settings{ std::make_shared<Broadcast::settings_t>() };

    boost::asio::awaitable<void> run()
    {
        InfillGenerator generator{ };

        while (true)
        {
            grpc::ServerContext server_context;

            cura::plugins::slots::infill::v0::generate::CallRequest request;
            grpc::ServerAsyncResponseWriter<Rsp> writer{ &server_context };
            co_await agrpc::request(&T::RequestCall, *generate_service, server_context, request, writer, boost::asio::use_awaitable);
            Rsp response;

            auto c_uuid = server_context.client_metadata().find("cura-engine-uuid");
            if (c_uuid == server_context.client_metadata().end())
            {
                spdlog::warn("cura-engine-uuid not found in client metadata");
                continue;
            }
            std::string client_metadata = std::string{ c_uuid->second.data(), c_uuid->second.size() };
            auto infill_density = 20.0;


            grpc::Status status = grpc::Status::OK;
            try
            {
                auto poly_lines = generator.generate();

                // convert poly_lines to protobuf response
                auto* poly_lines_msg = response.mutable_poly_lines();
                for (auto& poly_line : poly_lines)
                {
                    auto* path_msg = poly_lines_msg->add_paths();
                    for (auto& point : poly_line)
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
