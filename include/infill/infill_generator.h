#ifndef INFILL_INFILL_GENERATOR_H
#define INFILL_INFILL_GENERATOR_H

#include "infill/point_container.h"
#include "infill/tile.h"

#include <polyclipping/clipper.hpp>
#include <range/v3/algorithm/minmax.hpp>

#include <numeric>

class InfillGenerator
{
public:
    size_t tile_points{ 5 };
    double magnitude{ 0.5 };

    using BoundingBox = std::pair<ClipperLib::IntPoint, ClipperLib::IntPoint>;

    static BoundingBox computeBoundingBox(const geometry::polygon_outer<>& outer_contour)
    {
        ClipperLib::IntPoint p_min{ std::numeric_limits<ClipperLib::cInt>::max(), std::numeric_limits<ClipperLib::cInt>::max() };
        ClipperLib::IntPoint p_max{ std::numeric_limits<ClipperLib::cInt>::min(), std::numeric_limits<ClipperLib::cInt>::max() };

        for (const auto& point : outer_contour)
        {
            p_min.X = std::min(p_min.X, point.X);
            p_min.Y = std::min(p_min.Y, point.Y);
            p_max.X = std::max(p_max.X, point.X);
            p_max.Y = std::max(p_max.Y, point.Y);
        }

        return { p_min, p_max };
    }

    static ClipperLib::IntPoint computeCoG(const geometry::polygon_outer<>& outer_contour)
    {
        ClipperLib::IntPoint cog{ 0, 0 };
        for (const auto& point : outer_contour)
        {
            cog.X += point.X;
            cog.Y += point.Y;
        }
        cog.X /= outer_contour.size();
        cog.Y /= outer_contour.size();
        return cog;
    }

    auto generate(const geometry::polygon_outer<>& outer_contour)
    {
        auto bounding_box = computeBoundingBox(outer_contour);
        auto cog = computeCoG(outer_contour);
        Tile<TileType::HEXAGON, 3000> centerTile{};
        auto shape = centerTile.TileContour(cog.X, cog.Y);

        return shape;
    }
};

#endif // INFILL_INFILL_GENERATOR_H