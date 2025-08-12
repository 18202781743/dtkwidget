// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsuggestbutton.h"
#include "dstyleoption.h"

#include <QStylePainter>
#include <QStyleOptionButton>
#include <DGuiApplicationHelper>
#include <QLoggingCategory>

DWIDGET_BEGIN_NAMESPACE

namespace {
Q_DECLARE_LOGGING_CATEGORY(logBasicWidgets)
}

/*!
@~english
  @class Dtk::Widget::DSuggestButton
  @brief This widget provides the default suggest button style in DTK.
 */

/*!
@~english
  @brief Constructor of DSuggestButton
  @param[in] parent
 */
DSuggestButton::DSuggestButton(QWidget *parent)
    : QPushButton(parent)
{
    qCDebug(logBasicWidgets) << "Creating suggest button";
}

/*!
@~english
  @brief Overloaded constructor of DSuggestButton that accepts the default displayed text.
  @param[in] text The default displayed text
  @param[in] parent
 */
DSuggestButton::DSuggestButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    qCDebug(logBasicWidgets) << "Creating suggest button with text:" << text;
}


void DSuggestButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    qCDebug(logBasicWidgets) << "Painting suggest button";
    QStylePainter p(this);
    DStyleOptionButton option;
    initStyleOption(&option);
    option.init(this);
    option.features |= QStyleOptionButton::ButtonFeature(DStyleOptionButton::SuggestButton);

    QColor startColor = palette().color(QPalette::Highlight);
    QColor endColor = DGuiApplicationHelper::adjustColor(startColor, 0, 0, +10, 0, 0, 0, 0);
    qCDebug(logBasicWidgets) << "Using colors - start:" << startColor << "end:" << endColor;

    option.palette.setBrush(QPalette::Light, QBrush(endColor));
    option.palette.setBrush(QPalette::Dark, QBrush(startColor));
    option.palette.setBrush(QPalette::ButtonText, option.palette.highlightedText());
    p.drawControl(QStyle::CE_PushButton, option);
}

DWIDGET_END_NAMESPACE
