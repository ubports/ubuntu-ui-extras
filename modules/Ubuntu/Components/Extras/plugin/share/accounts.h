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

#ifndef ACCOUNTSQML_H
#define ACCOUNTSQML_H

#include <QObject>
#include <Accounts/Manager>
#include <Accounts/Account>

class FacebookAccount : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int id READ id)
    Q_PROPERTY(QString name READ name)

public:
    explicit FacebookAccount(QObject *parent = 0);
    int id() const { return m_accountId; }
    QString name() const { return m_accountName; }

public Q_SLOTS:
    void update();

private:
    int m_accountId;
    QString m_accountName;
};

#endif // ACCOUNTSQML_H
