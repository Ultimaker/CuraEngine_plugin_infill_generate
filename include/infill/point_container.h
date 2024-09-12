// Copyright (c) 2023 UltiMaker
// curaengine_plugin_generate_infill is released under the terms of the AGPLv3 or higher

#ifndef UTILS_GEOMETRY_POINT_CONTAINER_H
#define UTILS_GEOMETRY_POINT_CONTAINER_H

#include "infill/concepts.h"

#include <polyclipping/clipper.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/transform.hpp>

#include <initializer_list>
#include <memory>
#include <vector>

namespace infill::geometry
{

using Point = ClipperLib::IntPoint;

/*! The base clase of all point based container types
 *
 * @tparam P
 * @tparam IsClosed
 * @tparam Direction
 * @tparam Container
 */
template<typename P, bool IsClosed, direction Direction>
struct point_container : public std::vector<P>
{
    inline static constexpr bool is_closed = IsClosed;
    inline static constexpr direction winding = Direction;

    constexpr point_container() noexcept = default;
    constexpr explicit point_container(std::initializer_list<P> points) noexcept
        : std::vector<P>(points)
    {
    }
};

template<typename P = Point>
struct polyline : public point_container<P, false, direction::NA>
{
    constexpr polyline() noexcept = default;
    constexpr explicit polyline(std::initializer_list<P> points) noexcept
        : point_container<P, false, direction::NA>(points)
    {
    }
};

template<typename P, direction Direction>
struct polygon : public point_container<P, true, Direction>
{
    constexpr polygon() noexcept = default;
    constexpr polygon(std::initializer_list<P> points) noexcept
        : point_container<P, true, Direction>(points)
    {
    }
};

template<typename P = Point>
polygon(std::initializer_list<P>) -> polygon<P, direction::NA>;

template<typename P = Point>
struct polygon_outer : public point_container<P, true, direction::CW>
{
    constexpr polygon_outer() noexcept = default;
    constexpr explicit polygon_outer(std::initializer_list<P> points) noexcept
        : point_container<P, true, direction::CW>(points)
    {
    }
};

template<typename P = Point>
polygon_outer(std::initializer_list<P>) -> polygon_outer<P>;

template<typename P = Point>
struct polygon_inner : public point_container<P, true, direction::CCW>
{
    constexpr polygon_inner() noexcept = default;
    constexpr explicit polygon_inner(std::initializer_list<P> points) noexcept
        : point_container<P, true, direction::CCW>(points)
    {
    }
};

template<typename P = Point>
polygon_inner(std::initializer_list<P>) -> polygon_inner<P>;

template<typename P = Point>
struct polygons : public std::vector<polygon<P, direction::NA>*>
{
    constexpr polygons() noexcept = default;
    constexpr explicit polygons(std::initializer_list<polygon<P, direction::NA>*> polygons) noexcept
        : std::vector<polygon<P, direction::NA>*>(polygons)
    {
    }

    constexpr auto outer() noexcept
    {
        return polygon_outer{ this->front() };
    }

    constexpr auto inners() noexcept
    {
        return ranges::views::drop(this->base(), 1)
             | ranges::views::transform(
                   [](auto& p)
                   {
                       return polygon_inner{ p };
                   });
    }
};

template<typename P = Point>
polygons(polygon_outer<P>, std::initializer_list<polygon_inner<P>>) -> polygons<P>;

} // namespace infill::geometry

static inline infill::geometry::Point operator-(const infill::geometry::Point& p0)
{
    return infill::geometry::Point{ -p0.X, -p0.Y };
}
static inline infill::geometry::Point operator+(const infill::geometry::Point& p0, const infill::geometry::Point& p1)
{
    return infill::geometry::Point{ p0.X + p1.X, p0.Y + p1.Y };
}
static inline infill::geometry::Point operator-(const infill::geometry::Point& p0, const infill::geometry::Point& p1)
{
    return infill::geometry::Point{ p0.X - p1.X, p0.Y - p1.Y };
}

#endif // UTILS_GEOMETRY_POINT_CONTAINER_H
