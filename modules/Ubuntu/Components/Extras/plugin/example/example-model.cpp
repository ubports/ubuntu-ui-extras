/*
 * Copyright 2013 Canonical Ltd.
 *
 * This file is part of ubuntu-ui-extras.
 *
 * ubuntu-ui-extras is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * ubuntu-ui-extras is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "example-model.h"

// Qt
#include <QtCore/QDebug>

/*!
    \class Example Model
    \brief Example list model with a fixed number of items

    It's just an example of something implemented in c++ and exposed
    by a plugin.
*/
ExampleModel::ExampleModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

ExampleModel::~ExampleModel()
{
}

QHash<int, QByteArray> ExampleModel::roleNames() const
{
    static QHash<int, QByteArray> roles;
    if (roles.isEmpty()) {
        roles[Text] = "text";
    }
    return roles;
}

int ExampleModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant ExampleModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    int row = index.row();
    switch (role) {
    case Text:
        return QString("%1").arg(row);
    default:
        return QVariant();
    }
}
