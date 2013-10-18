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

#include "accounts.h"

#include <QDebug>

FacebookAccount::FacebookAccount(QObject *parent) :
    QObject(parent),
    m_accountId(-1)
{
}

/*!
 * \brief FacebookAccount::id
 * \return ID for that account
 */
int FacebookAccount::id() const
{
    if (m_accountId < 0)
        update();

    return m_accountId;
}

/*!
 * \brief FacebookAccount::name
 * \return display name for that account
 */
const QString &FacebookAccount::name() const
{
    if (m_accountName.isEmpty())
        update();

    return m_accountName;
}

/* Since we can't get a notification when the Accounts::AccountIdList changed
 * we expose this function to be called whenever the user needs to check if the
 * account still exists or if it was created.
 * This is sufficient for now since this class is only a temporary hack anyway
 * until we support all types of accounts and not just hardcode Facebook support.
 */
void FacebookAccount::update() const
{
    Accounts::Manager manager;
    Accounts::AccountIdList list = manager.accountList();
    Q_FOREACH(unsigned int accountId, list) {
        Accounts::Account* account = manager.account(accountId);
        if (account->providerName() == "facebook") {
            m_accountId = accountId;
            m_accountName = account->displayName();
            return;
        }
    }
    m_accountId = -1;
    m_accountName.clear();
}
