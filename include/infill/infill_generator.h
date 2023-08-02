#ifndef INFILL_INFILL_GENERATOR_H
#define INFILL_INFILL_GENERATOR_H

#include "infill/point_container.h"
#include "infill/tile.h"

#include <polyclipping/clipper.hpp>
#include <range/v3/algorithm/minmax.hpp>

#include <numbers>
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
        ClipperLib::IntPoint p_max{ std::numeric_limits<ClipperLib::cInt>::min(), std::numeric_limits<ClipperLib::cInt>::min() };

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

    static std::vector<geometry::polygon_outer<>> gridToPolygon(const auto& grid)
    {
        std::vector<geometry::polygon_outer<>> shape;
        for (const auto& row : grid)
        {
            for (const auto& tile : row)
            {
                shape.push_back(tile.TileContour());
            }
        }
        return shape;
    }

    auto generate(const geometry::polygon_outer<>& outer_contour)
    {
        auto bounding_box = computeBoundingBox(outer_contour);
        auto cog = computeCoG(outer_contour);

        constexpr int64_t line_width = 200;
        constexpr int64_t tile_size = 2000;
        constexpr auto tile_type = TileType::HEXAGON;

        using tile_t = Tile<tile_type, tile_size>;
        auto width_offset = tile_type == TileType::HEXAGON ? static_cast<int64_t>(std::sqrt(3) * tile_size + line_width) : 2 * tile_size + line_width;
        auto height_offset = tile_type == TileType::HEXAGON ? 3 * tile_size / 2 + line_width : 2 * tile_size + line_width;
        auto alternating_row_offset = [width_offset, tile_type](const auto row_idx)
        {
            return tile_type == TileType::HEXAGON ? static_cast<int64_t>(row_idx % 2 * width_offset / 2) : 0;
        };

        std::vector<std::vector<tile_t>> grid;

        size_t row_count{ 0 };
        for (auto y = bounding_box.first.Y - height_offset; y < bounding_box.second.Y + height_offset; y += height_offset)
        {
            std::vector<tile_t> row;
            for (auto x = bounding_box.first.X - width_offset + alternating_row_offset(row_count); x < bounding_box.second.X + width_offset; x += width_offset)
            {
                row.push_back({ x, y });
            }
            grid.push_back(row);
            row_count++;
        }

        // Cut the grid with the outer contour using Clipper
        ClipperLib::Clipper clipper;
        clipper.AddPath(outer_contour, ClipperLib::PolyType::ptSubject, true);
        auto polys = gridToPolygon(grid);
        ClipperLib::Paths grid_poly;
        for (auto& poly : polys)
        {
            grid_poly.push_back(poly);
        }
        clipper.AddPaths(grid_poly, ClipperLib::PolyType::ptClip, true);
        // Container for resulting polygons
        ClipperLib::Paths result;

        // Compute intersection
        clipper.Execute(ClipperLib::ClipType::ctIntersection, result);

        return result;
    }
};

#endif // INFILL_INFILL_GENERATOR_H