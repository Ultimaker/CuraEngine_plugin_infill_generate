#ifndef CURAENGINE_PLUGIN_INFILL_GENERATE_INCLUDE_INFILL_GEOMETRY_H
#define CURAENGINE_PLUGIN_INFILL_GENERATE_INCLUDE_INFILL_GEOMETRY_H

#include "infill/point_container.h"
#include "polyclipping/clipper.hpp"

#include <cmath>
#include <numbers>
#include <numeric>

namespace infill::geometry
{

using BoundingBox = std::vector<ClipperLib::IntPoint>;

static BoundingBox computeBoundingBox(const auto& contour)
{
    ClipperLib::IntPoint p_min{ std::numeric_limits<ClipperLib::cInt>::max(), std::numeric_limits<ClipperLib::cInt>::max() };
    ClipperLib::IntPoint p_max{ std::numeric_limits<ClipperLib::cInt>::min(), std::numeric_limits<ClipperLib::cInt>::min() };

    for (const auto& point : contour)
    {
        p_min.X = std::min(p_min.X, point.X);
        p_min.Y = std::min(p_min.Y, point.Y);
        p_max.X = std::max(p_max.X, point.X);
        p_max.Y = std::max(p_max.Y, point.Y);
    }

    return { p_min, p_max };
}

static ClipperLib::IntPoint computeCoG(const auto& contour)
{
    ClipperLib::IntPoint cog{ 0, 0 };
    for (const auto& point : contour)
    {
        cog.X += point.X;
        cog.Y += point.Y;
    }
    cog.X /= contour.size();
    cog.Y /= contour.size();
    return cog;
}

static ClipperLib::Paths clip(const auto& polys, const bool& is_poly_closed, const std::vector<geometry::polygon_outer<>>& outer_contours)
{
    ClipperLib::Clipper clipper;
    ClipperLib::Paths outline_poly;
    for (const auto& poly : outer_contours)
    {
        outline_poly.push_back(poly);
    }
    clipper.AddPaths(outline_poly, ClipperLib::PolyType::ptClip, true);

    ClipperLib::Paths grid_poly;
    for (auto& poly : polys)
    {
        grid_poly.push_back(poly);
    }
    clipper.AddPaths(grid_poly, ClipperLib::PolyType::ptSubject, is_poly_closed);

    ClipperLib::Paths ret;
    if (! is_poly_closed)
    {
        ClipperLib::PolyTree result;
        clipper.Execute(ClipperLib::ClipType::ctIntersection, result);
        ClipperLib::OpenPathsFromPolyTree(result, ret);
    }
    else
    {
        clipper.Execute(ClipperLib::ClipType::ctIntersection, ret);
    }
    return ret;
}

} // namespace infill::geometry


#endif // CURAENGINE_PLUGIN_INFILL_GENERATE_INCLUDE_INFILL_GEOMETRY_H
