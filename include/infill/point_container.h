// Copyright (c) 2023 UltiMaker
// CuraEngine is released under the terms of the AGPLv3 or higher

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
template<concepts::point P, bool IsClosed, direction Direction, template<class, class = std::allocator<P>> class Container>
    requires concepts::point<typename Container<P>::value_type>
struct point_container : public Container<P>
{
    inline static constexpr bool is_closed = IsClosed;
    inline static constexpr direction winding = Direction;
};

template<concepts::point P = Point, template<class, class = std::allocator<P>> class Container = std::vector>
struct polyline : public point_container<P, false, direction::NA, Container>
{
    constexpr polyline() noexcept = default;
    constexpr explicit polyline(std::initializer_list<P> points) noexcept
        : point_container<P, false, direction::NA, Container>(points)
    {
    }
};

template<concepts::point P, direction Direction, template<class, class = std::allocator<P>> class Container>
struct polygon : public point_container<P, true, Direction, Container>
{
    constexpr polygon() noexcept = default;
    constexpr polygon(std::initializer_list<P> points) noexcept
        : point_container<P, true, Direction, Container>(points)
    {
    }
};

template<concepts::point P = Point, template<class, class = std::allocator<P>> class Container = std::vector>
polygon(std::initializer_list<P>) -> polygon<P, direction::NA, Container>;

template<concepts::point P = Point, template<class, class = std::allocator<P>> class Container = std::vector>
struct polygon_outer : public point_container<P, true, direction::CW, Container>
{
    constexpr polygon_outer() noexcept = default;
    constexpr explicit polygon_outer(std::initializer_list<P> points) noexcept
        : point_container<P, true, direction::CW, Container>(points)
    {
    }
};

template<concepts::point P = Point, template<class, class = std::allocator<P>> class Container = std::vector>
struct polygon_inner : public point_container<P, true, direction::CCW, Container>
{
    constexpr polygon_inner() noexcept = default;
    constexpr explicit polygon_inner(std::initializer_list<P> points) noexcept
        : point_container<P, true, direction::CCW, Container>(points)
    {
    }
};

template<concepts::point P = Point, template<class, class = std::allocator<P>> class Container = std::vector>
    requires concepts::point<typename Container<P>::value_type>
struct polygons : public Container<polygon<P, direction::NA, Container>*>
{
    constexpr polygons() noexcept = default;
    constexpr explicit polygons(std::initializer_list<polygon<P, direction::NA, Container>*> polygons) noexcept
        : Container<polygon<P, direction::NA, Container>*>(polygons)
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