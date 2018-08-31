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

#ifndef HALIDE_GENERATORS_COMMON_H
#define HALIDE_GENERATORS_COMMON_H

#include <Halide.h>
#include <QtCore/QRect>
#include "halide_common.h"

inline void schedule_cpu_and_glsl(Halide::Internal::GeneratorBase* generator, Halide::ImageParam& input, Halide::Func& function, Halide::Var x, Halide::Var y, Halide::Var c) {
    Halide::Target target = generator->get_target();
    if (target.has_feature(Halide::Target::OpenGL)) {
        // Schedule for GLSL
        input.set_bounds(2, 0, 3);
        function.bound(c, 0, 4);
        function.glsl(x, y, c);
    } else {
        // Schedule for CPU
        function.vectorize(c, 4).parallel(y);
    }
}

template <class T> class HalideGeneratorExtended : public Halide::Generator<T> {
public:
    HalideGeneratorExtended() {
        GeneratorHelperRegistry::getOutputGeometryRegistry()->insert(this, &HalideGeneratorExtended::staticGetOutputGeometry);
    }

    static QRect staticGetOutputGeometry(Halide::Internal::GeneratorBase* generator) {
        return (static_cast<HalideGeneratorExtended*>(generator))->getOutputGeometry();
    }

    virtual QRect getOutputGeometry() {
        QRect out;
        Q_FOREACH(Halide::Internal::Parameter parameter, this->get_filter_parameters()) {
            if (parameter.is_buffer() && QString::fromStdString(parameter.name()) == "input") {
                out.setWidth(parameter.get_buffer().extent(0));
                out.setHeight(parameter.get_buffer().extent(1));
                return out;
            }
        }
        return out;
    }
};


#endif // HALIDE_GENERATORS_COMMON_H


