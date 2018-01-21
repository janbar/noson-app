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

#include "qmlsortbehavior.h"

SortBehavior::SortBehavior(QObject *parent)
    : QObject(parent)
    , m_property(QString())
    , m_order(Qt::AscendingOrder)
{

}

QString
SortBehavior::property() const
{
    return m_property;
}

Qt::SortOrder
SortBehavior::order() const
{
    return m_order;
}

void
SortBehavior::setProperty(const QString& property)
{
    m_property = property;
    Q_EMIT propertyChanged();
}

void
SortBehavior::setOrder(Qt::SortOrder order)
{
    m_order = order;
    Q_EMIT orderChanged();
}

