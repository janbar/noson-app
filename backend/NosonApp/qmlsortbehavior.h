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

#ifndef SORTBEHAVIOR_H
#define SORTBEHAVIOR_H

#include <QSortFilterProxyModel>

class SortBehavior : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString property READ property WRITE setProperty NOTIFY propertyChanged)
    Q_PROPERTY(Qt::SortOrder order READ order WRITE setOrder NOTIFY orderChanged)

public:
    explicit SortBehavior(QObject *parent = 0);

    QString property() const;
    void setProperty(const QString& property);
    Qt::SortOrder order() const;
    void setOrder(Qt::SortOrder order);

Q_SIGNALS:
    void propertyChanged();
    void orderChanged();

private:
    QString m_property;
    Qt::SortOrder m_order;
};

#endif // SORTBEHAVIOR_H
