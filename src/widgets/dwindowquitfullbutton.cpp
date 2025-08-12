// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dstyleoption.h"
#include "dwindowquitfullbutton.h"
#include <QLoggingCategory>

#include <DStyledIconEngine>

DWIDGET_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(logBasicWidgets)

DWindowQuitFullButton::DWindowQuitFullButton(QWidget * parent)
    : DIconButton(parent)
{
    qCDebug(logBasicWidgets) << "Construct exit fullscreen button"
                             << reinterpret_cast<const void *>(this);
    //DStyle::SP_TitleQuitFullButton
    auto iconEngine = new DStyledIconEngine(DDrawUtils::drawTitleQuitFullButton, QStringLiteral("TitleQuitFullButton"));
    setIcon(QIcon(iconEngine));
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    setFlat(true);
}

QSize DWindowQuitFullButton::sizeHint() const
{
    qCDebug(logBasicWidgets) << "Size hint";
    return iconSize();
}

void DWindowQuitFullButton::initStyleOption(DStyleOptionButton *option) const
{
    DIconButton::initStyleOption(option);

    qCDebug(logBasicWidgets) << "Init style option";
    option->features |= QStyleOptionButton::ButtonFeature(DStyleOptionButton::TitleBarButton);
}

DWIDGET_END_NAMESPACE
