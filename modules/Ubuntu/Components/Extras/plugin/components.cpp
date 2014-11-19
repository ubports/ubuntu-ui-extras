/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtQuick>

#include "components.h"
#include "example/example-model.h"

#include "share/accounts.h"
#include "share/imageresizer.h"
#include "share/notifyqml.h"

#include "photoeditor/photo.h"
#include "photoeditor/photo-image-provider.h"
#include "photoeditor/file-utils.h"


void Components::registerTypes(const char *uri)
{
    // Example component
    qmlRegisterType<ExampleModel>(uri, 0, 1, "ExampleModel");

    // Share component
    qmlRegisterType<FacebookAccount>(uri, 0, 1, "FacebookAccount");
    qmlRegisterType<Notify>(uri, 0, 1, "Notify");
    qmlRegisterType<ImageResizer>(uri, 0, 1, "ImageResizer");

    // PhotoEditor component
    qmlRegisterType<Photo>(uri, 0, 1, "Photo");
    qmlRegisterSingletonType<FileUtils>(uri, 0, 1, "FileUtils",
                                        exportFileUtilsSingleton);
}

void Components::initializeEngine(QQmlEngine *engine, const char *uri)
{
    QQmlExtensionPlugin::initializeEngine(engine, uri);

    PhotoImageProvider* provider = new PhotoImageProvider();
    provider->setLogging(true);
    engine->addImageProvider(PhotoImageProvider::PROVIDER_ID,
                             provider);
}

QObject* Components::exportFileUtilsSingleton(QQmlEngine *engine,
                                              QJSEngine *scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);

    return new FileUtils();
}
