/*   -*- Mode: C++ -*-
 * meegotouchcp-connman - connectivity plugin for duicontrolpanel
 * Copyright © 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
#ifndef NETWORKLISTMODEL_H
#define NETWORKLISTMODEL_H

#include "networkitem.h"
#include "networkmanager.h"

#include <QAbstractTableModel>
#include <QtDBus>

class QDBusServiceWatcher;

class NetworkListModel : public QAbstractListModel
{
    Q_OBJECT;

    Q_PROPERTY(bool offlineMode READ offlineMode WRITE setOfflineMode NOTIFY offlineModeChanged);
    Q_PROPERTY(QString defaultTechnology READ defaultTechnology NOTIFY defaultTechnologyChanged);
    Q_PROPERTY(QString state READ state NOTIFY stateChanged);
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged);
    Q_PROPERTY(QStringList availableTechnologies READ availableTechnologies NOTIFY availableTechnologiesChanged);
    Q_PROPERTY(QStringList enabledTechnologies READ enabledTechnologies NOTIFY enabledTechnologiesChanged);
    Q_PROPERTY(QStringList connectedTechnologies READ connectedTechnologies NOTIFY connectedTechnologiesChanged);
    Q_PROPERTY(NetworkItemModel* defaultRoute READ defaultRoute WRITE setDefaultRoute NOTIFY defaultRouteChanged);


public:  
    NetworkListModel(QObject* parent=0);
    virtual ~NetworkListModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role) const;

    const QStringList availableTechnologies() const;
    const QStringList enabledTechnologies() const;
    const QStringList connectedTechnologies() const;

    NetworkItemModel* defaultRoute() const { return m_networkManager->defaultRoute(); }

    void setDefaultRoute(NetworkItemModel* item);



public slots:
    NetworkItemModel* service(QString name);

    void connectService(const QString &name, const QString &security,
			const QString &passphrase);
    void connectService(const QString &name);
    void setProperty(const int &index, QString property, const QVariant &value);
    void requestScan();
    bool offlineMode() const;
    void setOfflineMode(const bool &offlineMode);
    void enableTechnology(const QString &technology);
    void disableTechnology(const QString &technology);
    QString defaultTechnology() const;
    QString state() const;

    void networksAdded(int from, int to);
    void networksMoved(int from, int to, int to2);
    void networksRemoved(int from, int to);
    void networkChanged(int index);

signals:
    void technologiesChanged(const QStringList &availableTechnologies,
			     const QStringList &enabledTechnologies,
			     const QStringList &connectedTechnologies);
    void availableTechnologiesChanged(const QStringList);
    void enabledTechnologiesChanged(const QStringList);
    void connectedTechnologiesChanged(const QStringList);

    void offlineModeChanged(bool offlineMode);
    void defaultTechnologyChanged(const QString &defaultTechnology);
    void stateChanged(const QString &state);
    void countChanged(int newCount);
    void defaultRouteChanged(NetworkItemModel* item);
    void connectedNetworkItemsChanged();

private:
    NetworkManager* m_networkManager;
    QList<NetworkItemModel*> shadowList;

private:
    Q_DISABLE_COPY(NetworkListModel);
};

#endif //NETWORKLISTMODEL_H