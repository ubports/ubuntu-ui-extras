/*
 * Copyright (C) 2014, 2017 Canonical, Ltd.
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
 *
 * Authored by: Ken VanDine <ken.vandine@canonical.com>
 *              Andrew Hayzen <andrew.hayzen@canonical.com>
 */

#ifndef I18N_H
#define I18N_H

#include <QtCore/QString>

void initTr(const char *domain, const char *localeDir);
QString __(const char *text, const char *domain = 0);

#endif // I18N_H

