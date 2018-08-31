/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Halide.h>
#include "halide_generators_common.h"

namespace {

class Transform : public HalideGeneratorExtended<Transform> {
public:
    Halide::ImageParam input{ Halide::type_of<uint8_t>(), 3, "input" };
    Halide::ImageParam transform{ Halide::type_of<float>(), 2, "transform" };
    Halide::ImageParam size{ Halide::type_of<float>(), 2, "size" };

    Halide::Func build()  {
        Halide::Var x, y, c;
        Halide::Func function;
        Halide::Func clamped;
        clamped(x, y, c) = input(Halide::clamp(x, 0, input.width()-1),
                                 Halide::clamp(y, 0, input.height()-1), c);
        function(x, y, c) = clamped(Halide::cast<int>(transform(0, 0)*x + transform(0, 1)*y + transform(0, 2)*input.width()),
                                    Halide::cast<int>(transform(1, 0)*x + transform(1, 1)*y + transform(1, 2)*input.height()),
                                    c);
        schedule_cpu_and_glsl(this, input, function, x, y, c);

        set_packed(input);
        set_packed(function.output_buffer());

        return function;
    }

    float getMatrixValue(Halide::ImageParam matrix, unsigned int row, unsigned int column) {
        return ((float*)(matrix.get().raw_buffer()->host))[row + column * matrix.get().extent(1)];
    }

    QRect getOutputGeometry() {
        int input_width = input.get().extent(0);
        int input_height = input.get().extent(1);
        int width = getMatrixValue(size, 0, 0) * input_width + getMatrixValue(size, 0, 1) * input_height;
        int height = getMatrixValue(size, 1, 0) * input_width + getMatrixValue(size, 1, 1) * input_height;

        QRect out;
        out.setWidth(width);
        out.setHeight(height);

        return out;
    }
};

Halide::RegisterGenerator<Transform> register_transform{"transform"};

}
