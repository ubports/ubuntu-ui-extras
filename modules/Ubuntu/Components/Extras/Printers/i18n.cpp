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

#define NO_TR_OVERRIDE
#include "i18n.h"

#include <libintl.h>

const char *thisDomain = "";

void initTr(const char *domain, const char *localeDir)
{
    // Don't bind the domain or set textdomain as it changes the Apps domain
    // as well. Instead store the domain and use it in the lookups
//    bindtextdomain(domain, localeDir);
//    textdomain(domain);
    Q_UNUSED(localeDir);

    thisDomain = domain;
}

QString __(const char *text, const char *domain)
{
    Q_UNUSED(domain);

    // Use the stored domain
    return QString::fromUtf8(dgettext(thisDomain, text));
}
