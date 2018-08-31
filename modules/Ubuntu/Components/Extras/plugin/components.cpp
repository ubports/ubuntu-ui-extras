/*
 * Copyright (C) 2012-2013, 2016 Canonical, Ltd.
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

#include "photoeditor/photo-metadata.h"
#include "photoeditor/photo-image-provider.h"
#include "photoeditor/file-utils.h"

#include "tabsbar/drag-helper.h"

void Components::registerTypes(const char *uri)
{
    // Example component
    qmlRegisterType<ExampleModel>(uri, 0, 2, "ExampleModel");

    // PhotoEditor component
    qmlRegisterType<PhotoEditor::PhotoMetadata>(uri, 0, 2, "PhotoMetadata");
    qmlRegisterSingletonType<PhotoEditor::FileUtils>(uri, 0, 2, "FileUtils",
                                        exportFileUtilsSingleton);

    // TabsBar component
    qmlRegisterType<DragHelper>(uri, 0, 3, "DragHelper");
}

void Components::initializeEngine(QQmlEngine *engine, const char *uri)
{
    QQmlExtensionPlugin::initializeEngine(engine, uri);

    PhotoImageProvider* provider = new PhotoImageProvider();
    engine->addImageProvider(PhotoImageProvider::PROVIDER_ID,
                             provider);
}

QObject* Components::exportFileUtilsSingleton(QQmlEngine *engine,
                                              QJSEngine *scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);

    return new PhotoEditor::FileUtils();
}
