#ifndef PLUGIN_SETTINGS_H
#define PLUGIN_SETTINGS_H

#include "cura/plugins/slots/broadcast/v0/broadcast.grpc.pb.h"
#include "infill/tile_type.h"

#include <semver.hpp>

#include <algorithm>
#include <cctype>
#include <ctre.hpp>
#include <locale>
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


    static std::optional<std::string> getPattern(std::string_view pattern, std::string_view plugin_name, std::string_view plugin_version)
    {
        auto semantic_version = semver::from_string(plugin_version);
        auto out = fmt::format("PLUGIN::{}@{}.{}.{}::", plugin_name, semantic_version.major, semantic_version.minor, semantic_version.patch);
        std::string pattern_key {pattern};
        if(pattern_key.find(out)!= std::string::npos)
        {
            pattern_key.erase(pattern_key.find(out), out.length());
            return pattern_key;
        }
        return std::nullopt;
    }

    static constexpr infill::TileType getTileType(std::string_view tile_type)
    {
        if (tile_type == "square")
        {
            return infill::TileType::SQUARE;
        }
        if (tile_type == "hexagon")
        {
            return infill::TileType::HEXAGON;
        }
        return infill::TileType::NONE;
    }


    static std::optional<std::string> retrieveSettings(std::string settings_key, const auto& request, const auto& metadata)
    {
        auto settings_key_ = settingKey(settings_key, metadata->plugin_name, metadata->plugin_version);
        if (request.settings().settings().contains(settings_key_))
        {
            return request.settings().settings().at(settings_key_);
        }

        return std::nullopt;
    }

    static bool validatePlugin(const cura::plugins::slots::handshake::v0::CallRequest& request, const std::shared_ptr<Metadata>& metadata)
    {
        auto plugin_name = request.plugin_name();
        auto plugin_name_expect = metadata->plugin_name;
        return plugin_name == plugin_name_expect && request.plugin_version() == metadata->plugin_version;
    }

    static std::string settingKey(std::string_view short_key, std::string_view name, std::string_view version)
    {
        std::string lower_name{ name };
        auto semantic_version = semver::from_string(version);
        std::transform(
            lower_name.begin(),
            lower_name.end(),
            lower_name.begin(),
            [](const auto& c)
            {
                return std::tolower(c);
            });
        return fmt::format("_plugin__{}__{}_{}_{}__{}", lower_name, semantic_version.major, semantic_version.minor, semantic_version.patch, short_key);
    }
};

using settings_t = std::unordered_map<std::string, Settings>;

} // namespace plugin

#endif // PLUGIN_SETTINGS_H
