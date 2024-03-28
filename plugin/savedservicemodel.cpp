/*
 * Copyright © 2010 Intel Corporation.
 * Copyright © 2012-2017 Jolla Ltd.
 * Contact: Slava Monich <slava.monich@jolla.com>
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0. The full text of the Apache License
 * is at http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QDebug>
#include "savedservicemodel.h"

namespace
{

bool compareServices(NetworkService *a, NetworkService *b)
{
    if (a->available() && !b->available())
        return true;

    if (b->available() && !a->available())
        return false;

    if (a->available() && b->available() && a->strength() > 0 && b->strength() > 0)
        return b->strength() < a->strength();

    return a->name() < b->name();
}

bool compareManagedServices(NetworkService *a, NetworkService *b)
{
    if (a->managed() && !b->managed())
        return true;

    if (b->managed() && !a->managed())
        return false;

    return compareServices(a, b);
}

}

SavedServiceModel::SavedServiceModel(QAbstractListModel* parent)
    : QAbstractListModel(parent)
    , m_sort(false)
    , m_groupByCategory(false)
{
    m_manager = NetworkManager::sharedInstance();
    connect(m_manager.data(), &NetworkManager::technologiesChanged,
            this, &SavedServiceModel::updateServiceList);
    connect(m_manager.data(), &NetworkManager::servicesChanged,
            this, &SavedServiceModel::updateServiceList);
}

SavedServiceModel::~SavedServiceModel()
{
}

QHash<int, QByteArray> SavedServiceModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ServiceRole] = "networkService";
    roles[ManagedRole] = "managed";
    return roles;
}

QVariant SavedServiceModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case ServiceRole:
        return QVariant::fromValue(static_cast<NetworkService *>(m_services.value(index.row())));
    case ManagedRole:
        return m_services.value(index.row())->managed();
    }

    return QVariant();
}

int SavedServiceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_services.count();
}

QString SavedServiceModel::name() const
{
    return m_techname;
}

void SavedServiceModel::setName(const QString &name)
{
    if (m_techname == name) {
        return;
    }

    m_techname = name;
    Q_EMIT nameChanged(m_techname);

    QStringList netTypes = m_manager->technologiesList();
    if (!netTypes.contains(name)) {
        qDebug() << name <<  "is not a known technology name:" << netTypes;
        return;
    }

    updateServiceList();
}

bool SavedServiceModel::sort() const
{
    return m_sort;
}

void SavedServiceModel::setSort(bool sortList)
{
    if (m_sort == sortList)
        return;

    m_sort = sortList;
    emit sortChanged();

    updateServiceList();
}


bool SavedServiceModel::groupByCategory() const
{
    return m_groupByCategory;
}

void SavedServiceModel::setGroupByCategory(bool groupByCategory)
{
    if (m_groupByCategory == groupByCategory)
        return;

    m_groupByCategory = groupByCategory;
    emit groupByCategoryChanged();

    updateServiceList();
}


NetworkService *SavedServiceModel::get(int index) const
{
    if (index < 0 || index > m_services.count())
        return 0;
    return m_services.value(index);
}

int SavedServiceModel::indexOf(const QString &dbusObjectPath) const
{
    int idx(-1);

    for (NetworkService *service : m_services) {
        idx++;
        if (service->path() == dbusObjectPath) return idx;
    }

    return -1;
}

void SavedServiceModel::updateServiceList()
{
    QVector<NetworkService *> new_services = m_manager->getSavedServices(m_techname);
    if (m_sort) {
        if (m_groupByCategory) {
            std::stable_sort(new_services.begin(), new_services.end(), compareManagedServices);
        } else {
            std::stable_sort(new_services.begin(), new_services.end(), compareServices);
        }
    }

    int num_new = new_services.count();

    for (int i = 0; i < num_new; i++) {
        int j = m_services.indexOf(new_services.value(i));
        if (j == -1) {
            // wifi service not found -> remove from list
            beginInsertRows(QModelIndex(), i, i);
            m_services.insert(i, new_services.value(i));
            endInsertRows();
        } else if (i != j) {
            // wifi service changed its position -> move it
            NetworkService* service = m_services.value(j);
            beginMoveRows(QModelIndex(), j, j, QModelIndex(), i);
            m_services.remove(j);
            m_services.insert(i, service);
            endMoveRows();
        } else {
            QModelIndex changedIndex(this->index(j, 0, QModelIndex()));
            Q_EMIT dataChanged(changedIndex, changedIndex);
        }
    }

    int num_old = m_services.count();
    if (num_old > num_new) {
        beginRemoveRows(QModelIndex(), num_new, num_old - 1);
        m_services.remove(num_new, num_old - num_new);
        endRemoveRows();
    }
}

