// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusinterface.h"

#include <QLoggingCategory>

namespace {
Q_DECLARE_LOGGING_CATEGORY(logMessageNotification)
}

/*
 * Implementation of interface class DBusInterface
 */

DBusInterface::DBusInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
    qCDebug(logMessageNotification) << "Creating DBus interface for service:" << service;
    QDBusConnection::sessionBus().connect(this->service(), this->path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged","sa{sv}as", this, SLOT(__propertyChanged__(QDBusMessage)));
}

DBusInterface::~DBusInterface()
{
    qCDebug(logMessageNotification) << "Destroying DBus interface";
    QDBusConnection::sessionBus().disconnect(service(), path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged",  "sa{sv}as", this, SLOT(propertyChanged(QDBusMessage)));
}

