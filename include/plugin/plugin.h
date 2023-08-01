#ifndef PLUGIN_PLUGIN_H
#define PLUGIN_PLUGIN_H

#include "plugin/broadcast.h"
#include "plugin/generate.h"
#include "plugin/handshake.h"

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
