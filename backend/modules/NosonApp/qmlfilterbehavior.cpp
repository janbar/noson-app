/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *   Christian Dywan <christian.dywan@canonical.com>
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

#include "qmlfilterbehavior.h"

FilterBehavior::FilterBehavior(QObject *parent)
    : QObject(parent)
    , m_property(QString())
    , m_pattern(QRegExp())
{

}

QString
FilterBehavior::property() const
{
    return m_property;
}

void
FilterBehavior::setProperty(const QString& property)
{
    m_property = property;
    Q_EMIT propertyChanged();
}

QRegExp
FilterBehavior::pattern() const
{
    return m_pattern;
}

void
FilterBehavior::setPattern(QRegExp pattern)
{
    m_pattern = pattern;
    Q_EMIT patternChanged();
}

