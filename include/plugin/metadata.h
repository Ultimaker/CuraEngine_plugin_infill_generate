#ifndef PLUGIN_METADATA_H
#define PLUGIN_METADATA_H

#include "plugin/cmdline.h"

#include <agrpc/asio_grpc.hpp>
#include <spdlog/spdlog.h>

#include <string_view>

namespace plugin
{

struct Metadata
{
    std::string_view slot_version_range{ ">=0.1.0" };
    std::string_view plugin_name{ cmdline::NAME };
    std::string_view plugin_version{ cmdline::VERSION };
};

std::string getUuid(grpc::ServerContext& server_context)
{
    auto c_uuid = server_context.client_metadata().find("cura-engine-uuid");
    if (c_uuid == server_context.client_metadata().end())
    {
        spdlog::error("cura-engine-uuid not found in client metadata");
        throw std::runtime_error("cura-engine-uuid not found in client metadata");
    }
    return { c_uuid->second.data(), c_uuid->second.size() };
}


} // namespace plugin

#endif // PLUGIN_METADATA_H
