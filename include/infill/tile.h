#ifndef INFILL_TILE_H
#define INFILL_TILE_H

#include "infill/content_reader.h"
#include "infill/geometry.h"
#include "infill/point_container.h"

#include <fmt/ranges.h>
#include <range/v3/all.hpp>
#include <spdlog/spdlog.h>

#include <cmath>
#include <filesystem>
#include <numbers>
#include <vector>

namespace infill
{
// Enum describing the 3 supported tile types triangle, square and hexagon.
enum class TileType
{
    TRIANGLE,
    SQUARE,
    HEXAGON
};

template<TileType T, int64_t M = 1000>
class Tile
{
public:
    using value_type = std::tuple<std::vector<geometry::polyline<>>, std::vector<geometry::polygon_outer<>>>;
    int64_t x{ 0 };
    int64_t y{ 0 };
    std::filesystem::path filepath{};

    inline static constexpr int64_t magnitude{ M };
    inline static constexpr TileType tile_type{ T };

    value_type render(const bool contour) const noexcept
    {
        auto content = readContent(filepath);
        content = fitContent(content);
        return content;
    }

private:
    constexpr auto tileContour() const noexcept
        requires std::is_same_v<decltype(x), decltype(y)>
    {
        using coord_t = decltype(x);
        switch (tile_type)
        {
        case TileType::TRIANGLE:
            return geometry::polygon_outer{ { x + static_cast<coord_t>(magnitude / -2), y + static_cast<coord_t>(magnitude / -2) },
                                            { x, y + static_cast<coord_t>(magnitude / 2) },
                                            { x + static_cast<coord_t>(magnitude / 2), y + static_cast<coord_t>(magnitude / -2) } };
        case TileType::SQUARE:
            return geometry::polygon_outer{ { x + static_cast<coord_t>(magnitude / -2), y + static_cast<coord_t>(magnitude / -2) },
                                            { x + static_cast<coord_t>(magnitude / -2), y + static_cast<coord_t>(magnitude / 2) },
                                            { x + static_cast<coord_t>(magnitude / 2), y + static_cast<coord_t>(magnitude / 2) },
                                            { x + static_cast<coord_t>(magnitude / 2), y + static_cast<coord_t>(magnitude / -2) } };
        case TileType::HEXAGON:
            return geometry::polygon_outer{
                { x, y + static_cast<coord_t>(magnitude * 1.0) }, // top
                { x + static_cast<coord_t>(magnitude * sqrt(3) / 2), y + static_cast<coord_t>(magnitude * 0.5) }, // top-right
                { x + static_cast<coord_t>(magnitude * sqrt(3) / 2), y - static_cast<coord_t>(magnitude * 0.5) }, // bottom-right
                { x, y - static_cast<coord_t>(magnitude * 1.0) }, // bottom
                { x - static_cast<coord_t>(magnitude * sqrt(3) / 2), y - static_cast<coord_t>(magnitude * 0.5) }, // bottom-left
                { x - static_cast<coord_t>(magnitude * sqrt(3) / 2), y + static_cast<coord_t>(magnitude * 0.5) }, // top-left
            };
        default:
            return geometry::polygon_outer{ { x + static_cast<coord_t>(magnitude / -2), y + static_cast<coord_t>(magnitude / -2) },
                                            { x, y + static_cast<coord_t>(magnitude / 2) },
                                            { x + static_cast<coord_t>(magnitude / 2), y + static_cast<coord_t>(magnitude / -2) } };
        }
    }

    constexpr value_type fitContent(value_type& content) const noexcept
    {
        auto bb_lines = std::get<0>(content)
                      | ranges::views::transform(
                            [](const auto& contour)
                            {
                                return geometry::computeBoundingBox(contour);
                            })
                      | ranges::views::join;
        auto bb_polys = std::get<1>(content)
                      | ranges::views::transform(
                            [](const auto& contour)
                            {
                                return geometry::computeBoundingBox(contour);
                            })
                      | ranges::views::join;
        auto bounding_boxes = ranges::views::concat(bb_lines, bb_polys) | ranges::to_vector;
        auto bb = geometry::computeBoundingBox(bounding_boxes);

        // Center and scale the content in the tile.
        auto center = geometry::computeCoG(bb);
        auto scale_factor = 2 * magnitude / std::max(bb.at(1).X - bb.at(0).X, bb.at(1).Y - bb.at(0).Y);
        for (auto& line : std::get<0>(content))
        {
            for (auto& point : line)
            {
                point.X = x + static_cast<int64_t>(scale_factor * (point.X - center.X));
                point.Y = y + static_cast<int64_t>(scale_factor * (point.Y - center.Y));
            }
        }
        for (auto& poly : std::get<1>(content))
        {
            for (auto& point : poly)
            {
                point.X = x + static_cast<int64_t>(scale_factor * (point.X - center.X));
                point.Y = y + static_cast<int64_t>(scale_factor * (point.Y - center.Y));
            }
        }
        // remove the first polygon, which is the bounding box of the content.
        std::get<1>(content).erase(std::get<1>(content).begin());
        return content;
    }
};
} // namespace infill
#endif // INFILL_TILE_H
