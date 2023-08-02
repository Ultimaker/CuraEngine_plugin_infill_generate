#ifndef PLUGIN_SETTINGS_H
#define PLUGIN_SETTINGS_H

#include "cura/plugins/slots/broadcast/v0/broadcast.grpc.pb.h"

#include <ctre.hpp>
#include <optional>
#include <string>
#include <unordered_map>

namespace plugin
{

struct Settings
{
    int infill_extruder{ 0 };
    int64_t line_distance{ 2000 };

    explicit Settings(const cura::plugins::slots::broadcast::v0::BroadcastServiceSettingsRequest& msg)
    {
        auto global_settings = msg.global_settings().settings();
        infill_extruder = std::stoi(global_settings.at("infill_extruder_nr"));
        line_distance = std::stoll(global_settings.at("infill_line_distance"));
    }

    static constexpr std::string_view getPattern(std::string_view pattern, std::string_view name)
    {
        if (auto [_, setting_namespace, plugin_name, _, pattern_name] = ctre::match<"^(.*?)::(.*?)@(.*?)::(.*?)$">(pattern); setting_namespace == "PLUGIN" && plugin_name == name)
        {
            return pattern_name;
        }
        return pattern;
    }
};

using settings_t = std::unordered_map<std::string, Settings>;

} // namespace plugin

#endif // PLUGIN_SETTINGS_H
