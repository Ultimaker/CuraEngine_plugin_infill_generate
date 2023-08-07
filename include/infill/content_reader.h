#ifndef INFILL_CONTENT_READER_H
#define INFILL_CONTENT_READER_H

#include "infill/boost_tags.h"
#include "infill/point_container.h"

#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/io/wkt/read.hpp>
#include <range/v3/view/remove_if.hpp>

#include <filesystem>
#include <fstream>

namespace infill
{

std::tuple<std::vector<geometry::polyline<>>, std::vector<geometry::polygon_outer<>>> readContent(const std::filesystem::path& filepath)
{
    std::vector<geometry::polygon_outer<>> polygons;
    std::vector<geometry::polyline<>> linestrings;

    std::ifstream wkt_file(filepath);
    std::string line;

    while (std::getline(wkt_file, line))
    {
        if (line.empty())
        {
            continue;
        }
        if (line.starts_with("LINESTRING"))
        {
            geometry::polyline<> linestring;
            boost::geometry::read_wkt(line, linestring);
            linestrings.push_back(linestring);
        }
        if (line.starts_with("POLYGON"))
        {
            geometry::polygon_outer<> polygon;
            boost::geometry::read_wkt(line, polygon);
            polygons.push_back(polygon);
        }
    }

    return { linestrings, polygons };
}
} // namespace infill

#endif // INFILL_CONTENT_READER_H
