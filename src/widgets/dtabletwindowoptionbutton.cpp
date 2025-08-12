// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtabletwindowoptionbutton.h"
#include "dstyleoption.h"

#include <QLoggingCategory>

DWIDGET_BEGIN_NAMESPACE

namespace {
Q_DECLARE_LOGGING_CATEGORY(logBasicWidgets)
}

/*!
  \class Dtk::Widget::DTabletWindowOptionButton
  \inmodule dtkwidget
  \brief The DTabletWindowOptionButton class is used as the unified tablet window option button.
  \brief DTabletWindowOptionButton 类是 DTK 平板窗口统一的菜单按钮控件。
  
  It's actually a special DImageButton which has the appearance of option button.
  点击按钮后，默认会显示程序主菜单，包含“关于”、“帮助”等项。
 */

/*!
  \brief DTabletWindowOptionButton::DTabletWindowOptionButton 是 DTabletWindowOptionButton 的构造
  函数，返回 DTabletWindowOptionButton 对象，普通程序一般无需使用。
  \a parent 为创建对象的父控件。
 */
DTabletWindowOptionButton::DTabletWindowOptionButton(QWidget *parent)
    : DIconButton(DStyle::SP_TitleMoreButton, parent)
{
    qCDebug(logBasicWidgets) << "Creating tablet window option button";
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    setFlat(true);
}

QSize DTabletWindowOptionButton::sizeHint() const
{
    const auto& size = iconSize();
    qCDebug(logBasicWidgets) << "Size hint requested:" << size;
    return size;
}

void DTabletWindowOptionButton::initStyleOption(DStyleOptionButton *option) const
{
    qCDebug(logBasicWidgets) << "Initializing style option";
    DIconButton::initStyleOption(option);

    option->features |= QStyleOptionButton::ButtonFeature(DStyleOptionButton::TitleBarButton);
}

DWIDGET_END_NAMESPACE
