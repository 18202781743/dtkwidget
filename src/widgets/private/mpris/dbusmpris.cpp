// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusmpris.h"

#include <QLoggingCategory>

namespace {
Q_DECLARE_LOGGING_CATEGORY(logMessageNotification)
}

/*
 * Implementation of interface class DBusMPRIS
 */

DBusMPRIS::DBusMPRIS(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
    qCDebug(logMessageNotification) << "Creating DBus MPRIS interface for service:" << service;
    QDBusConnection::sessionBus().connect(this->service(), this->path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged","sa{sv}as", this, SLOT(__propertyChanged__(QDBusMessage)));
}

DBusMPRIS::~DBusMPRIS()
{
    qCDebug(logMessageNotification) << "Destroying DBus MPRIS interface";
    QDBusConnection::sessionBus().disconnect(service(), path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged",  "sa{sv}as", this, SLOT(propertyChanged(QDBusMessage)));
}

