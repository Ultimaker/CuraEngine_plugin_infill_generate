#ifndef CURAENGINE_PLUGIN_INFILL_GENERATE_INCLUDE_INFILL_INFILL_GENERATOR_H
#define CURAENGINE_PLUGIN_INFILL_GENERATE_INCLUDE_INFILL_INFILL_GENERATOR_H

#endif // CURAENGINE_PLUGIN_INFILL_GENERATE_INCLUDE_INFILL_INFILL_GENERATOR_H

#include "infill/point_container.h"

class InfillGenerator
{
public:
    InfillGenerator() = default;
    ~InfillGenerator() = default;

    std::vector<geometry::polyline<>> generate()
    {
        std::vector<geometry::polyline<>> poly_lines;
        for (int x = 0; x < 200000; x += 10000)
        {
            geometry::polyline<> poly_line;
            poly_line.push_back({ x, 0 });
            poly_line.push_back({ x, 200000 });
            poly_lines.push_back(poly_line);
        }

        return poly_lines;
    }
};
