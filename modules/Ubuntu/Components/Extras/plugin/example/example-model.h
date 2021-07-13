/*
 * Copyright 2013 Canonical Ltd.
 *
 * This file is part of lomiri-ui-extras.
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

#ifndef __EXAMPLE_MODEL_H__
#define __EXAMPLE_MODEL_H__

// Qt
#include <QtCore/QAbstractListModel>
#include <QtCore/QList>

class QQuickItem;

class ExampleModel : public QAbstractListModel
{
    Q_OBJECT

    Q_ENUMS(Roles)

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    ExampleModel(QObject* parent=0);
    ~ExampleModel();

    enum Roles {
        Text = Qt::UserRole + 1
    };

    // reimplemented from QAbstractListModel
    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex& parent=QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role) const;

Q_SIGNALS:
    void countChanged() const;
};

#endif // __EXAMPLE_MODEL_H__
