#ifndef PLUGIN_SETTINGS_H
#define PLUGIN_SETTINGS_H

#include "cura/plugins/slots/broadcast/v0/broadcast.grpc.pb.h"
#include "plugin/cmdline.h"

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
    std::string pattern{ "grid" };

    explicit Settings(const cura::plugins::slots::broadcast::v0::BroadcastServiceSettingsRequest& msg)
    {
        auto global_settings = msg.global_settings().settings();
        infill_extruder = std::stoi(global_settings.at("infill_extruder_nr"));
        line_distance = std::stoll(global_settings.at("infill_line_distance"));
        auto extruder_settings = msg.extruder_settings().at(infill_extruder).settings();
        if (extruder_settings.contains("infill_pattern"))
        {
            if (auto extruder_pattern = getPattern(extruder_settings.at("infill_pattern")); extruder_pattern.has_value())
            {
                pattern = extruder_pattern.value();
            }
            else
            {
                spdlog::error("Unknown pattern: {}", extruder_settings.at("infill_pattern"));
            }
        }
    }

    static constexpr std::optional<std::string_view> getPattern(std::string_view pattern)
    {
        if (auto [_, plugin_name, pattern_name] = ctre::match<".*?::(.*?)::(.*)">(pattern); plugin_name == cmdline::NAME)
        {
            return pattern_name;
        }
        return std::nullopt;
    }
};

using settings_t = std::unordered_map<std::string, Settings>;

} // namespace plugin

#endif // PLUGIN_SETTINGS_H
