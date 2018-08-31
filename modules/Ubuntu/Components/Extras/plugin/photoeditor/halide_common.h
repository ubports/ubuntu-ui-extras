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

#ifndef HALIDE_COMMON_H
#define HALIDE_COMMON_H

#include <QtCore/QRect>
#include <QtCore/QHash>
#include <QtGui/QImage>
#include <Halide.h>

typedef QRect (*getOutputGeometryFunction)(Halide::Internal::GeneratorBase*);

class GeneratorHelperRegistry
{
public:
    static QHash<Halide::Internal::GeneratorBase*, getOutputGeometryFunction>* getOutputGeometryRegistry() {
        static QHash<Halide::Internal::GeneratorBase*, getOutputGeometryFunction>* registry = new QHash<Halide::Internal::GeneratorBase*, getOutputGeometryFunction>;
        return registry;
    }
};

inline void set_packed(Halide::OutputImageParam image, int channels = 4) {
    image.set_stride(0, channels);
    image.set_stride(2, 1);
    image.set_extent(2, channels);
}

inline buffer_t* packedBuffer(uint8_t* pixels, int width, int height, int channels)
{
    buffer_t* buffer = new buffer_t;
    memset(buffer, 0, sizeof(buffer_t));
    buffer->host = pixels;
    buffer->elem_size = 1;
    buffer->extent[0] = width;
    buffer->extent[1] = height;
    buffer->extent[2] = channels;
    buffer->stride[0] = buffer->extent[2];
    buffer->stride[1] = buffer->extent[0] * buffer->stride[0];
    buffer->stride[2] = 1;
    return buffer;
}

inline buffer_t* halideBufferTFromQImage(QImage& image)
{
    image = image.convertToFormat(QImage::Format_RGBA8888);
    return packedBuffer(image.bits(), image.width(), image.height(), 4);
}

inline Halide::Buffer halideBufferFromQImage(QImage& image)
{
    buffer_t* buft = halideBufferTFromQImage(image);
    buft->host_dirty = true;
    Halide::Buffer buffer(Halide::type_of<uint8_t>(), buft);
    delete buft;
    return buffer;
}

inline Halide::Buffer halideBufferFromTexture(uint textureId, QSize size)
{
    buffer_t* buft = packedBuffer(NULL, size.width(), size.height(), 4);
    buft->dev = textureId;
    Halide::Buffer buffer = Halide::Buffer(Halide::type_of<uint8_t>(), buft);
    delete buft;
    return buffer;
}

inline QRect getOutputGeometry(Halide::Internal::GeneratorBase* generator)
{
    getOutputGeometryFunction getOutputGeometry = GeneratorHelperRegistry::getOutputGeometryRegistry()->value(generator, 0);
    return getOutputGeometry ? getOutputGeometry(generator) : QRect();
}



#endif // HALIDE_COMMON_H


