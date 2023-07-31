#ifndef PLUGIN_GENERATE_H
#define PLUGIN_GENERATE_H

#include <boost/asio/awaitable.hpp>

#include <memory>

namespace plugin
{

template<class T, class Rsp, class Req>
struct Generate
{
    using service_t = std::shared_ptr<T>;
    service_t generate_service{ std::make_shared<T>() };

    boost::asio::awaitable<void> run()
    {
        while (true)
        {
            grpc::ServerContext server_context;

            cura::plugins::slots::infill::v0::generate::CallRequest request;
            grpc::ServerAsyncResponseWriter<Rsp> writer{ &server_context };
            co_await agrpc::request(
                &T::RequestCall,
                *generate_service,
                server_context,
                request,
                writer,
                boost::asio::use_awaitable);
            Rsp response;

            auto c_uuid = server_context.client_metadata().find("cura-engine-uuid");
            if (c_uuid == server_context.client_metadata().end())
            {
                spdlog::warn("cura-engine-uuid not found in client metadata");
                continue;
            }
            std::string client_metadata = std::string{ c_uuid->second.data(), c_uuid->second.size() };
            auto infill_density = 20.0;


            //                auto infill_density = std::stoi(settings[client_metadata]["infill_density"]);
            //                auto infill_pattern = settings[client_metadata]["infill_pattern"];  // split on last namespace

            grpc::Status status = grpc::Status::OK;
            try
            {
            }
            catch (const std::exception& e)
            {
                spdlog::error("Error: {}", e.what());
                status = grpc::Status(grpc::StatusCode::INTERNAL, static_cast<std::string>(e.what()));
            }

            // co_await agrpc::finish(writer, response, status, boost::asio::use_awaitable);
        }
    }
};

} // namespace plugin

#endif // PLUGIN_GENERATE_H
