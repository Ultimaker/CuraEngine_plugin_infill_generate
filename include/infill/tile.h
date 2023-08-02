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

    int64_t x{ 0 };
    int64_t y{ 0 };

    constexpr auto TileContour() const noexcept
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
};

#endif // INFILL_TILE_H
