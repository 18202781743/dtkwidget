// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dthemehelper.h"

#include <QLoggingCategory>

namespace {
Q_DECLARE_LOGGING_CATEGORY(logStyleTheme)
}

DThemeHelper::DThemeHelper(QObject *parent) :
    QObject(parent)
{
    qCDebug(logStyleTheme) << "Creating theme helper";
}

QString DThemeHelper::themeToQss(QString theme)
{
    qCDebug(logStyleTheme) << "Converting theme to QSS:" << theme;
    return theme;
}

QString DThemeHelper::trimThemeContents(QString qss)
{
    qCDebug(logStyleTheme) << "Trimming theme contents, length:" << qss.length();
    return qss;
}
