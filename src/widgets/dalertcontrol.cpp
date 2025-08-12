// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DStyle"
#include "dalertcontrol.h"
#include "private/dalertcontrol_p.h"

#include <QTimer>
#include <QLayout>
#include <QEvent>
#include <QLoggingCategory>

DWIDGET_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(logUtils)

DAlertControlPrivate::DAlertControlPrivate(DAlertControl *q)
 : DObjectPrivate(q)
{
    qCDebug(logUtils) << "Creating DAlertControlPrivate";
    alertColor = q->defaultAlertColor();
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, q, &DAlertControl::hideAlertMessage);
}

void DAlertControlPrivate::updateTooltipPos()
{
    qCDebug(logUtils) << "Updating tooltip position";
    if (!target || !target->parentWidget() || !frame || !frame->parentWidget()) {
        qCWarning(logUtils) << "target or frame is nullptr";
        return;
    }
    QWidget *p = target->parentWidget();

    int w = DStyle::pixelMetric(p->style(), DStyle::PM_FloatingWidgetShadowMargins) / 2;
    QPoint point = QPoint(target->x() - w, target->y() + target->height() - w);
    frame->move(p->mapTo(qobject_cast<QWidget *>(frame->parentWidget()), point));
    int tipWidth= frame->parentWidget()->width() - 20;
    tooltip->setMaximumWidth(tipWidth);
    frame->setMinimumHeight(tooltip->heightForWidth(tipWidth) + frame->layout()->spacing() *2);
    frame->adjustSize();

    int tw = target->width();
    int fw = frame->width();
    int leftPending = tw - fw;
    QPoint objPoint;
    switch (alignment) {
    case Qt::AlignLeft:
        qCDebug(logUtils) << "Aligning tooltip to left";
        objPoint = p->mapTo(qobject_cast<QWidget *>(frame->parentWidget()), point);
        if (objPoint.x() + frame->width() > (frame->parentWidget()->width())) {
            int shift = (objPoint.x() + frame->width()) - frame->parentWidget()->width();
            objPoint.setX(objPoint.x() - shift);
        }
        break;
    case Qt::AlignRight:
        qCDebug(logUtils) << "Aligning tooltip to right";
        point += QPoint(leftPending, 0);
        objPoint = p->mapTo(qobject_cast<QWidget *>(frame->parentWidget()), point);
        if (objPoint.x() < 0) {
            objPoint.setX(0);
        }
        break;
    case Qt::AlignHCenter:
    case Qt::AlignCenter:
        qCDebug(logUtils) << "Aligning tooltip to center";
        point += QPoint(leftPending/2, 0);
        objPoint = p->mapTo(qobject_cast<QWidget *>(frame->parentWidget()), point);
        break;
    default:
        qCWarning(logUtils) << "Unknown alignment:" << static_cast<int>(alignment);
        return;
    }

    frame->move(objPoint);
}

DAlertControl::DAlertControl(QWidget *target, QObject *parent)
    : QObject(parent)
    , DObject(*new DAlertControlPrivate(this))
{
    qCDebug(logUtils) << "Creating DAlertControl with target:" << target;
    D_D(DAlertControl);
    d->target = target;
}

DAlertControl::~DAlertControl()
{
    qCDebug(logUtils) << "Destroying DAlertControl";
    hideAlertMessage();
}

/*!
@~english
  \brief DAlertControl::setAlertSet whether to turn on the warning modeOpen the warning mode, Target will display the warning color
  \a isAlert Whether to turn on a warning mode
 */
void DAlertControl::setAlert(bool isAlert)
{
    qCDebug(logUtils) << "Setting alert state:" << isAlert;
    D_D(DAlertControl);
    if (isAlert == d->isAlert || !d->target) {
        qCDebug(logUtils) << "Alert state unchanged or target is null";
        return;
    }

    d->isAlert = isAlert;

    DPalette p = d->target->palette();

    if (isAlert) {
        qCDebug(logUtils) << "Applying alert color to target";
        p.setColor(QPalette::Button, alertColor());
        d->target->setPalette(p);
    } else {
        qCDebug(logUtils) << "Resetting target palette";
        d->target->setPalette(QPalette());
    }
    d->target->update();

    Q_EMIT alertChanged(isAlert);
}

/*!
@~english
  \brief DAlertControl::alertBack to whether it is currently in a warning mode
 */
bool DAlertControl::isAlert() const
{
    D_DC(DAlertControl);
    const auto& result = d->isAlert;
    qCDebug(logUtils) << "Current alert state:" << result;
    return result;
}

/*!
@~english
  \brief DAlertControl::defaultAlertColorBack to the default alarm color
  \note The default color is consistent with the original DLINEEDIT
 */
QColor DAlertControl::defaultAlertColor() const
{
    const auto& color = QColor(241, 57, 50, qRound(0.15 * 255));
    qCDebug(logUtils) << "Default alert color:" << color;
    return color;
}

/*!
@~english
  \brief DAlertControl::setAlertColor Set alarm color
  \a c Alarm color
 */
void DAlertControl::setAlertColor(QColor c)
{
    qCDebug(logUtils) << "Setting alert color:" << c;
    D_D(DAlertControl);
    if (c == d->alertColor) {
        qCDebug(logUtils) << "Alert color unchanged";
        return;
    }

    d->alertColor = c;
    if (d->target) {
        qCDebug(logUtils) << "Updating target with new alert color";
        d->target->update();
    }
}

/*!
@~english
  \brief DAlertControl::alertColor Return to the current alarm color
 */
QColor DAlertControl::alertColor() const
{
    D_DC(DAlertControl);
    const auto& color = d->alertColor;
    qCDebug(logUtils) << "Current alert color:" << color;
    return color;
}

/*!
@~english
  \brief DAlertControl::setMessageAlignmentSpecify the alignment method Now only support the left, right, center, default left
  \note When the parameters are other, the default left
  \a alignment 消息对齐方式
 */
void DAlertControl::setMessageAlignment(Qt::Alignment alignment)
{
    qCDebug(logUtils) << "Setting message alignment:" << static_cast<int>(alignment);
    D_D(DAlertControl);
    d->alignment = alignment;
}

/*!
@~english
  \brief DAlertControl::messageAlignment Return to the current alarm Tooltips alignment method
 */
Qt::Alignment DAlertControl::messageAlignment() const
{
    D_DC(DAlertControl);
    const auto& alignment = d->alignment;
    qCDebug(logUtils) << "Current message alignment:" << static_cast<int>(alignment);
    return alignment;
}

/*!
@~english
  \brief DAlertControl::showAlertMessage Display warning message
 Display the specified text message, exceeding the warning message disappearing after the specified time.
  \note When the time parameter is -1, the warning message will always exist
  \a text Warning text
  \a duration Display time length, unit milliseconds
 */
void DAlertControl::showAlertMessage(const QString &text, int duration)
{
    qCDebug(logUtils) << "Showing alert message:" << text << "duration:" << duration;
    showAlertMessage(text, nullptr, duration);
}

/*!
@~english
  \brief DAlertControl::showAlertMessage Display warning message.

  Display the specified text message, exceeding the warning message disappearing after the specified time
  \note When the time parameter is -1, the warning message will always exist
  \a text Warning text
  \a follow Specify the object of the text message
  \a duration Display time length, unit milliseconds
 */
void DAlertControl::showAlertMessage(const QString &text, QWidget *follower, int duration)
{
    qCDebug(logUtils) << "Showing alert message:" << text << "follower:" << follower << "duration:" << duration;
    D_D(DAlertControl);
    if (!d->target) {
        qCWarning(logUtils) << "Target is null, cannot show alert message";
        return;
    }

    if (text.isEmpty()) {
        qCWarning(logUtils) << "Alert text is empty";
        return;
    }

    if (!d->tooltip) {
        qCDebug(logUtils) << "Creating new tooltip";
        d->tooltip = new DToolTip(text);
        d->tooltip->setObjectName("AlertTooltip");
        d->tooltip->setAccessibleName("DAlertControlAlertToolTip");
        d->tooltip->setForegroundRole(DPalette::TextWarning);
        d->tooltip->setWordWrap(true);

        d->frame = new DFloatingWidget;
        d->frame->setAccessibleName("DAlertControlFloatingWidget");
        d->frame->setFramRadius(DStyle::pixelMetric(d->target->style(), DStyle::PM_FrameRadius));
        d->frame->setBackgroundRole(QPalette::ToolTipBase);
        d->frame->setWidget(d->tooltip);
    }

    if (follower) {
        qCDebug(logUtils) << "Setting follower widget";
        d->frame->setParent(follower->topLevelWidget());
        d->follower = follower;
        d->follower->installEventFilter(this);
    } else {
        qCDebug(logUtils) << "Using target as follower";
        d->frame->setParent(d->target->topLevelWidget());
        d->follower = d->target;
        d->follower->installEventFilter(this);
    }
    d->follower->topLevelWidget()->installEventFilter(this);

    d->tooltip->setText(text);
    if (d->frame->parent()) {
        d->updateTooltipPos();
        // 如果 target 都隐藏了,显示警告消息毫无意义，只会让人困惑
        const auto& isVisible = d->target->isVisibleTo(d->target->topLevelWidget());
        qCDebug(logUtils) << "Target visible to top level widget:" << isVisible;
        d->frame->setVisible(isVisible);
        d->frame->adjustSize();
        d->frame->raise();
    }
    if (duration < 0) {
        qCDebug(logUtils) << "Stopping timer for persistent alert";
        d->timer.stop();
        return;
    }

    qCDebug(logUtils) << "Starting timer with duration:" << duration;
    d->timer.start(duration);
}

/*!
@~english
  \brief DAlertControl:: hideAlertMessage隐藏警告消息框
 */
void DAlertControl::hideAlertMessage()
{
    qCDebug(logUtils) << "Hiding alert message";
    Q_D(DAlertControl);

    if (d->frame) {
        d->frame->hide();
        if (d->follower) {
            //this->removeEventFilter(d->follower);
            d->follower->removeEventFilter(this);
            d->follower = nullptr;
        }
    }
}

DAlertControl::DAlertControl(DAlertControlPrivate &d, QObject *parent)
    : QObject(parent)
    , DObject(d)
{
    qCDebug(logUtils) << "Creating DAlertControl with private data";
}

bool DAlertControl::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(DAlertControl);
    if (watched == d->follower) {
        if (event->type() == QEvent::Move || event->type() == QEvent::Resize) {
            qCDebug(logUtils) << "Follower moved or resized, updating tooltip position";
            d->updateTooltipPos();
        }
    }

    if (d->follower && watched == d->follower->topLevelWidget()) {
        if (event->type() == QEvent::HoverMove || event->type() == QEvent::UpdateRequest) {
            qCDebug(logUtils) << "Top level widget hover or update, updating tooltip position";
            d->updateTooltipPos();
        }

        if (d->timer.isActive()) {
            const auto& visibleRegion = d->follower->visibleRegion();
            const auto& isVisible = !visibleRegion.isNull();
            qCDebug(logUtils) << "Timer active, setting frame visibility:" << isVisible;
            d->frame->setVisible(isVisible);
        }
    }
    return QObject::eventFilter(watched, event);
}

DWIDGET_END_NAMESPACE
