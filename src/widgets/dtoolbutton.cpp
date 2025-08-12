// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtoolbutton.h"

#include <QStyleOptionButton>
#include <QStylePainter>
#include <QLoggingCategory>

DWIDGET_BEGIN_NAMESPACE
Q_DECLARE_LOGGING_CATEGORY(logUtils)

/*!
@~english
    @class Dtk::Widget::DToolButton
    @brief Inherited from the QToolButton class, used to customize tool buttons.
 */

DToolButton::DToolButton(QWidget *parent)
    : QToolButton(parent)
{
    qCDebug(logUtils) << "create dtoolbutton";

}

/*!
@~english
    @fn void DToolButton::paintEvent(QPaintEvent *event)
    @brief This method overrides the paintEvent method of QToolButton to draw a custom appearance for the tool button.
    @param event The paint event, which is not used.
 */

void DToolButton::paintEvent(QPaintEvent *event)
{
    qCDebug(logUtils) << "paint dtoolbutton";
    Q_UNUSED(event)
    QStylePainter p(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    p.drawComplexControl(QStyle::CC_ToolButton, opt);
}

/*!
@~english
    @fn void DToolButton::initStyleOption(QStyleOptionToolButton *option) const
    @brief Initializes the style options for the tool button.
    @param option A pointer to a QStyleOptionToolButton object, used to specify style options.
 */
void DToolButton::initStyleOption(QStyleOptionToolButton *option) const
{
    qCDebug(logUtils) << "init style option";
    QToolButton::initStyleOption(option);
}

/*!
@~english
    @fn QSize DToolButton::sizeHint() const
    @brief Gets the size hint for the tool button.
    @return The size hint for the tool button, of type QSize.
 */

QSize DToolButton::sizeHint() const
{
    const auto &s = QToolButton::sizeHint();
    qCDebug(logUtils) << "size hint" << s;
    return s;
}

/*!
@~english
    @fn void DToolButton::setAlignment(Qt::Alignment flag)
    @brief DToolButton:: setAlignment set the alignment of DToolButton
    @a flag alignment mode
 */
void DToolButton::setAlignment(Qt::Alignment flag)
{
    qCDebug(logUtils) << "setting tool button alignment:" << flag;
    this->setProperty("_d_dtk_toolButtonAlign", QVariant(flag));
}

/*!
@~english
    @fn Qt::Alignment DToolButton::alignment() const
    @brief DToolButton:: alignment returns the current alignment of DToolButton
    @return If setAlignment is successfully set, the current alignment is returned; otherwise, Qt:: AlignLeft is returned
 */
Qt::Alignment DToolButton::alignment() const
{
    qCDebug(logUtils) << "getting tool button alignment";
    if(this->property("_d_dtk_toolButtonAlign").isValid()) {
        const auto& alignment = static_cast<Qt::Alignment>(this->property("_d_dtk_toolButtonAlign").toInt());
        qCDebug(logUtils) << "returning custom alignment:" << alignment;
        return alignment;
    } else {
        qCDebug(logUtils) << "returning default alignment: AlignLeft";
        return Qt::AlignLeft;
    }
}

DWIDGET_END_NAMESPACE
