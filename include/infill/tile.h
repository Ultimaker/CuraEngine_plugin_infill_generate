#ifndef INFILL_TILE_H
#define INFILL_TILE_H

#include "infill/point_container.h"

#include <cmath>
#include <numbers>

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
    inline static constexpr int64_t magnitude{ M };
    inline static constexpr TileType tile_type{ T };

    constexpr auto TileContour(const auto x, const auto y) const noexcept
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
                { x + static_cast<coord_t>(magnitude * std::cos(0)), y + static_cast<coord_t>(magnitude * std::sin(0)) },
                { x + static_cast<coord_t>(magnitude * std::cos(std::numbers::pi / 3)), y + static_cast<coord_t>(magnitude * std::sin(std::numbers::pi / 3)) },
                { x + static_cast<coord_t>(magnitude * std::cos(2 * std::numbers::pi / 3)), y + static_cast<coord_t>(magnitude * std::sin(2 * std::numbers::pi / 3)) },
                { x + static_cast<coord_t>(magnitude * std::cos(3 * std::numbers::pi / 3)), y + static_cast<coord_t>(magnitude * std::sin(3 * std::numbers::pi / 3)) },
                { x + static_cast<coord_t>(magnitude * std::cos(4 * std::numbers::pi / 3)), y + static_cast<coord_t>(magnitude * std::sin(4 * std::numbers::pi / 3)) },
                { x + static_cast<coord_t>(magnitude * std::cos(5 * std::numbers::pi / 3)), y + static_cast<coord_t>(magnitude * std::sin(5 * std::numbers::pi / 3)) }
            };
        default:
            return geometry::polygon_outer{ { x + static_cast<coord_t>(magnitude / -2), y + static_cast<coord_t>(magnitude / -2) },
                                            { x, y + static_cast<coord_t>(magnitude / 2) },
                                            { x + static_cast<coord_t>(magnitude / 2), y + static_cast<coord_t>(magnitude / -2) } };
        }
    }
};

#endif // INFILL_TILE_H
