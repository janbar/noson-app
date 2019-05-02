/*
 * Copyright (C) 2012-2014 Canonical, Ltd.
 *
 * Authors:
 *   Michal Hruby <michal.hruby@canonical.com>
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

#ifndef NOSONAPPQSORTFILTERPROXYMODELQML_H
#define NOSONAPPQSORTFILTERPROXYMODELQML_H

#include <QSortFilterProxyModel>
#include "qmlsortbehavior.h"
#include "qmlfilterbehavior.h"

namespace nosonapp
{

class Q_DECL_EXPORT QSortFilterProxyModelQML : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel* model READ sourceModel WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(SortBehavior* sort READ sortBehavior NOTIFY sortChanged)
    Q_PROPERTY(FilterBehavior* filter READ filterBehavior NOTIFY filterChanged)

public:
    explicit QSortFilterProxyModelQML(QObject *parent = 0);

    Q_INVOKABLE QVariantMap get(int row);
    QVariant data(int row, int role);
    Q_INVOKABLE int count();
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

    /* getters */
    QHash<int, QByteArray> roleNames() const;

    /* setters */
    void setFilterProperty(const QString& property);
    void setModel(QAbstractItemModel *model);

Q_SIGNALS:
    void countChanged();
    void modelChanged();
    void sortChanged();
    void filterChanged();

private:
    SortBehavior m_sortBehavior;
    SortBehavior* sortBehavior();
    void sortChangedInternal();
    FilterBehavior m_filterBehavior;
    FilterBehavior* filterBehavior();
    void filterChangedInternal();
    int roleByName(const QString& roleName) const;
};

}

#endif // NOSONAPPQSORTFILTERPROXYMODELQML_H
