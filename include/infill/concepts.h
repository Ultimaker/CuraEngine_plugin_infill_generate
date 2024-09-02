// Copyright (c) 2023 UltiMaker
// curaengine_plugin_generate_infill is released under the terms of the AGPLv3 or higher

#ifndef UTILS_CONCEPTS_GEOMETRY_H
#define UTILS_CONCEPTS_GEOMETRY_H

#include <range/v3/range/concepts.hpp>

#include <concepts>
#include <string>
#include <type_traits>

namespace infill
{
enum class direction
{
    NA,
    CW,
    CCW
};
} // namespace infill

#endif // UTILS_CONCEPTS_GEOMETRY_H
