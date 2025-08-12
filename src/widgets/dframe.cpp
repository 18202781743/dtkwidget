// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dframe.h"
#include "dpalettehelper.h"
#include "private/dframe_p.h"

#include <DObjectPrivate>

#include <QPainter>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logBasicWidgets, "dtk.widgets.basic")

DWIDGET_BEGIN_NAMESPACE

DFramePrivate::DFramePrivate(DFrame *qq)
    : DCORE_NAMESPACE::DObjectPrivate(qq)
    , frameRounded(true)
    , backType(DPalette::NoType)
{
    qCDebug(logBasicWidgets) << "Creating DFramePrivate";
}

/*!
@~english
  @brief DFrame::DFrame is used for other widget base class that require frame
  @param[in] parent
 */
DFrame::DFrame(QWidget *parent)
    : DFrame(*new DFramePrivate(this), parent)
{
    qCDebug(logBasicWidgets) << "Creating DFrame with parent";
}

/*!
@~english
  @brief DFrame::setFrameRounded sets whether the frame corners are rounded
  @param[in] on True means the corners will be rounded, while False means not
 */
void DFrame::setFrameRounded(bool on)
{
    qCDebug(logBasicWidgets) << "Setting frame rounded to:" << on;
    D_D(DFrame);

    if (d->frameRounded == on) {
        qCDebug(logBasicWidgets) << "Frame rounded state unchanged, skipping update";
        return;
    }

    qCDebug(logBasicWidgets) << "Frame rounded changed from" << d->frameRounded << "to" << on;
    d->frameRounded = on;
    update();
}

/*!
@~english
  @brief DFrame::setBackgroundRole sets role type of the frame background brush
  @param[in] type role type of the frame background brush
 */
void DFrame::setBackgroundRole(DGUI_NAMESPACE::DPalette::ColorType type)
{
    qCDebug(logBasicWidgets) << "Setting background role to:" << type;
    D_D(DFrame);

    if (d->backType == type) {
        qCDebug(logBasicWidgets) << "Background role unchanged, skipping update";
        return;
    }

    qCDebug(logBasicWidgets) << "Background role changed from" << d->backType << "to" << type;
    d->backType = type;
    update();
}

DFrame::DFrame(DFramePrivate &dd, QWidget *parent)
    : QFrame(parent)
    , DObject(dd)
{
    qCDebug(logBasicWidgets) << "Creating DFrame with private data";
    setBackgroundRole(QPalette::Base);
    setFrameShape(QFrame::StyledPanel);
}

void DFrame::paintEvent(QPaintEvent *event)
{
    qCDebug(logBasicWidgets) << "Frame paint event, rect:" << event->rect();
    Q_UNUSED(event)
    QStyleOptionFrame opt;
    initStyleOption(&opt);
    QPainter p(this);
    D_DC(DFrame);

    if (d->frameRounded) {
        qCDebug(logBasicWidgets) << "Adding rounded feature to frame";
        opt.features |= QStyleOptionFrame::Rounded;
    }

    const DPalette &dp = DPaletteHelper::instance()->palette(this);

    if (d->backType != DPalette::NoType) {
        qCDebug(logBasicWidgets) << "Setting background brush for type:" << d->backType;
        p.setBackground(dp.brush(d->backType));
    }

    p.setPen(QPen(dp.frameBorder(), opt.lineWidth));
    qCDebug(logBasicWidgets) << "Drawing frame with line width:" << opt.lineWidth;
    style()->drawControl(QStyle::CE_ShapedFrame, &opt, &p, this);
}

DWIDGET_END_NAMESPACE
