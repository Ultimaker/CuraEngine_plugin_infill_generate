
#include "cura/plugins/slots/infill/v0/generate.grpc.pb.h"
#include "cura/plugins/slots/infill/v0/generate.pb.h"
#include "plugin/cmdline.h" // Custom command line argument definitions
#include "plugin/handshake.h" // Handshake interface
#include "plugin/plugin.h" // Plugin interface

#include <boost/asio/signal_set.hpp>
#include <docopt/docopt.h> // Library for parsing command line arguments
#include <fmt/format.h> // Formatting library
#include <grpcpp/server.h>
#include <spdlog/spdlog.h> // Logging library

#include <map>

using namespace cura::plugins::slots::infill::v0;

int main(int argc, const char** argv)
{
    spdlog::set_level(spdlog::level::debug);
    constexpr bool show_help = true;
    const std::map<std::string, docopt::value> args
        = docopt::docopt(fmt::format(plugin::cmdline::USAGE, "curaengine_plugin_infill_generate"), { argv + 1, argv + argc }, show_help, plugin::cmdline::VERSION_ID);

    using generate_t = plugin::infill_generate::Generate<cura::plugins::slots::infill::v0::generate::InfillGenerateService::AsyncService,
                                        cura::plugins::slots::infill::v0::generate::CallResponse,
                                        cura::plugins::slots::infill::v0::generate::CallRequest>;

    plugin::Plugin<generate_t> plugin{ args.at("--address").asString(), args.at("--port").asString(), grpc::InsecureServerCredentials() };
    plugin.addHandshakeService(plugin::Handshake{ .metadata = plugin.metadata });

    auto broadcast_settings = std::make_shared<plugin::Broadcast::settings_t>();
    plugin.addBroadcastService(plugin::Broadcast{ .settings = broadcast_settings });
    plugin.addGenerateService(generate_t{ .settings = broadcast_settings, .metadata = plugin.metadata, .tiles_path = args.at("--tiles_path").asString() });
    plugin.start();
    plugin.run();
    plugin.stop();
}