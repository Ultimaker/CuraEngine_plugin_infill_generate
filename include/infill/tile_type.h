// Copyright (c) 2023 UltiMaker
// curaengine_plugin_generate_infill is released under the terms of the AGPLv3 or higher

#ifndef INFILL_TILE_TYPE_H
#define INFILL_TILE_TYPE_H

namespace infill
{
// Enum describing the 3 supported tile types triangle, square and hexagon.
enum class TileType
{
    SQUARE,
    HEXAGON,
    NONE
};
} // namespace infill

#endif // INFILL_TILE_TYPE_H
