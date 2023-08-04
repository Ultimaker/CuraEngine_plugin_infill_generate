#ifndef INFILL_INFILL_GENERATOR_H
#define INFILL_INFILL_GENERATOR_H

#include "infill/geometry.h"
#include "infill/point_container.h"
#include "infill/tile.h"

#include <polyclipping/clipper.hpp>
#include <range/v3/algorithm/minmax.hpp>

#include <numbers>
#include <numeric>

namespace infill
{

class InfillGenerator
{
public:
    static std::tuple<std::vector<geometry::polyline<>>, std::vector<geometry::polygon_outer<>>> gridToPolygon(const auto& grid)
    {
        std::tuple<std::vector<geometry::polyline<>>, std::vector<geometry::polygon_outer<>>> shape;
        for (const auto& row : grid)
        {
            for (const auto& tile : row)
            {
                auto [lines, polys] = tile.render(false);
                std::get<0>(shape).insert(std::get<0>(shape).end(), lines.begin(), lines.end());
                std::get<1>(shape).insert(std::get<1>(shape).end(), polys.begin(), polys.end());
            }
        }
        return shape;
    }

    std::tuple<ClipperLib::Paths, ClipperLib::Paths> generate(const std::vector<geometry::polygon_outer<>>& outer_contours)
    {
        auto bounding_boxes = outer_contours
                            | ranges::views::transform(
                                  [](const auto& contour)
                                  {
                                      return geometry::computeBoundingBox(contour);
                                  })
                            | ranges::views::join | ranges::to_vector;
        auto bounding_box = geometry::computeBoundingBox(bounding_boxes);

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
        for (auto y = bounding_box.at(0).Y - height_offset; y < bounding_box.at(1).Y + height_offset; y += height_offset)
        {
            std::vector<tile_t> row;
            for (auto x = bounding_box.at(0).X - width_offset + alternating_row_offset(row_count); x < bounding_box.at(1).X + width_offset; x += width_offset)
            {
                row.push_back({ .x = x, .y = y, .filepath = "./tiles/hex/honeycomb.wkt" });
            }
            grid.push_back(row);
            row_count++;
        }

        // Cut the grid with the outer contour using Clipper
        auto [lines, polys] = gridToPolygon(grid);
        return { geometry::clip(lines, outer_contours), geometry::clip(polys, outer_contours) };
    }
};

} // namespace infill
#endif // INFILL_INFILL_GENERATOR_H