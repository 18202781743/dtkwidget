// SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dfloatingbutton.h"
#include "dstyleoption.h"
#include "dobject_p.h"
#include "dstyle.h"

#include <private/qabstractbutton_p.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logBasicWidgets, "dtk.widgets.basic")

DWIDGET_BEGIN_NAMESPACE

/*!
@~english
  @brief DFloatingButton::DFloatingButton 用于定制化的button，根据传入的图标参数具体调整
  @a parent
 */
DFloatingButton::DFloatingButton(QWidget *parent)
    : DIconButton(parent)
{
    qCDebug(logBasicWidgets) << "Creating DFloatingButton with parent";
    setBackgroundRole(QPalette::Highlight);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

DFloatingButton::DFloatingButton(QStyle::StandardPixmap iconType, QWidget *parent)
    : DIconButton(iconType, parent)
{
    qCDebug(logBasicWidgets) << "Creating DFloatingButton with QStyle icon type:" << iconType;
    setBackgroundRole(QPalette::Highlight);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

DFloatingButton::DFloatingButton(DStyle::StandardPixmap iconType, QWidget *parent)
    : DIconButton(iconType, parent)
{
    qCDebug(logBasicWidgets) << "Creating DFloatingButton with DStyle icon type:" << iconType;
    setBackgroundRole(QPalette::Highlight);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

DFloatingButton::DFloatingButton(const QString &text, QWidget *parent)
    : DFloatingButton(parent)
{
    qCDebug(logBasicWidgets) << "Creating DFloatingButton with text:" << text;
    setText(text);
}

DFloatingButton::DFloatingButton(const QIcon &icon, const QString &text, QWidget *parent)
    : DFloatingButton(parent)
{
    qCDebug(logBasicWidgets) << "Creating DFloatingButton with icon and text:" << text;
    setIcon(icon);
    setText(text);
}

DFloatingButton::DFloatingButton(const DDciIcon &icon, const QString &text, QWidget *parent)
    : DFloatingButton(text, parent)
{
    qCDebug(logBasicWidgets) << "Creating DFloatingButton with DDciIcon and text:" << text;
    setIcon(icon);
}

DStyleOptionButton DFloatingButton::baseStyleOption() const
{
    qCDebug(logBasicWidgets) << "Getting base style option for floating button";
    DStyleOptionButton opt;
    opt.features = QStyleOptionButton::ButtonFeature(DStyleOptionButton::FloatingButton);
    qCDebug(logBasicWidgets) << "Base style option created with FloatingButton feature";
    return opt;
}

void DFloatingButton::initStyleOption(DStyleOptionButton *option) const
{
    qCDebug(logBasicWidgets) << "Initializing style option for floating button";
    DIconButton::initStyleOption(option);
    qCDebug(logBasicWidgets) << "Style option initialized via DIconButton";
}

DWIDGET_END_NAMESPACE
