// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dwarningbutton.h"
#include "dpalettehelper.h"
#include "dstyleoption.h"

#include <QStyleOptionButton>
#include <QStylePainter>
#include <QLoggingCategory>

DWIDGET_BEGIN_NAMESPACE
Q_DECLARE_LOGGING_CATEGORY(logBasicWidgets)

/*!
@~english
  @class Dtk::Widget::DWarningButton
  \inmodule dtkwidget
  @brief warning button widget
  @details inherits from DPushButton widget, mainly replacing the color of text and background.
  @note Usually gives a warning sign when executing some dangerous commands but not forbid it; For example: some programs require higher permission to run; files connot be restored after delete
  @sa QPushButton
 */

/*!
@~english
  @brief constructor
  @param[in] parent is the parent object of this widget
 */
DWarningButton::DWarningButton(QWidget *parent): DPushButton(parent)
{
    qCDebug(logBasicWidgets) << "create dwarningbutton";
}

/*!
@~english
  @brief initialize the style option
  @param[in] option is the style option used to initialize this widget
 */
void DWarningButton::initStyleOption(QStyleOptionButton *option) const
{
    qCDebug(logBasicWidgets) << "init warning option";
    DPushButton::initStyleOption(option);
    DPalette pa = DPaletteHelper::instance()->palette(this);
    option->palette.setBrush(QPalette::ButtonText, pa.textWarning());
    option->features |= QStyleOptionButton::ButtonFeature(DStyleOptionButton::WarningButton);
}

void DWarningButton::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
    qCDebug(logBasicWidgets) << "paint warning button";

    QStyleOptionButton opt;
    initStyleOption(&opt);
    QStylePainter painter(this);
    painter.drawControl(QStyle::CE_PushButton, opt);
}

DWIDGET_END_NAMESPACE
