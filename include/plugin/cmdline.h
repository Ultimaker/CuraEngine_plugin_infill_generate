#ifndef PLUGIN_CMDLINE_H
#define PLUGIN_CMDLINE_H

#include <string>
#include <string_view>

#include <fmt/compile.h>

namespace plugin::cmdline
{

constexpr std::string_view NAME = "CuraEngine plugin infill generate";
constexpr std::string_view VERSION = "0.1.0-alpha.1";
static const auto VERSION_ID = fmt::format(FMT_COMPILE("{} {}"), NAME, VERSION);

constexpr std::string_view USAGE = R"({0}.

Usage:
  curaengine_plugin_infill_generate [--address <address>] [--port <port>]
  curaengine_plugin_infill_generate (-h | --help)
  curaengine_plugin_infill_generate --version

Options:
  -h --help                 Show this screen.
  --version                 Show version.
  -ip --address <address>   The IP address to connect the socket to [default: localhost].
  -p --port <port>          The port number to connect the socket to [default: 33800].
)";

} // namespace plugin::cmdline

#endif // PLUGIN_CMDLINE_H