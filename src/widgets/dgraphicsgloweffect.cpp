// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dgraphicsgloweffect.h"
#include <QLoggingCategory>

QT_BEGIN_NAMESPACE
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
Q_DECLARE_LOGGING_CATEGORY(logContainers)

/*!
  \class Dtk::Widget::DGraphicsGlowEffect
  \inmodule dtkwidget

  \brief 控件发散效果，同时也是Dtk默认的窗口特效.
  \brief Draw a glow effect of widget, It's the default border effect of deepin windows.
 */

/*!
  \brief 构造一个DGraphicsGlowEffect对象，并指定其父对象。
  \brief Constructs a new DGraphicsGlowEffect instance having the specified parent.

  \a parent
 */
DGraphicsGlowEffect::DGraphicsGlowEffect(QObject *parent) :
    QGraphicsEffect(parent),
    m_xOffset(0),
    m_yOffset(0),
    m_distance(4.0),
    m_blurRadius(10.0),
    m_color(0, 0, 0, 80)
{
    qCDebug(logContainers) << "create glow effect";
}

/*!
  \brief DGraphicsGlowEffect::draw set a blur effect image to painter.
  \brief 将控件的扩散效果绘制到指定的painter上.

  \a painter
 */
void DGraphicsGlowEffect::draw(QPainter *painter)
{
    qCDebug(logContainers) << "draw glow";
    // if nothing to show outside the item, just draw source
    if ((blurRadius() + distance()) <= 0) {
        drawSource(painter);
        return;
    }

    PixmapPadMode mode = QGraphicsEffect::PadToEffectiveBoundingRect;
    QPoint offset;
    const QPixmap sourcePx = sourcePixmap(Qt::DeviceCoordinates, &offset, mode);

    // return if no source
    if (sourcePx.isNull()) {
        return;
    }

    qreal restoreOpacity = painter->opacity();
    painter->setOpacity(m_opacity);

    // save world transform
    QTransform restoreTransform = painter->worldTransform();
    painter->setWorldTransform(QTransform());

    // Calculate size for the background image
    QSize scaleSize(sourcePx.size().width() + 2 * distance(),
                    sourcePx.size().height() + 2 * distance());

    QImage tmpImg(scaleSize, QImage::Format_ARGB32_Premultiplied);
    QPixmap scaled = sourcePx.scaled(scaleSize);
    tmpImg.fill(0);
    QPainter tmpPainter(&tmpImg);
    tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
    tmpPainter.drawPixmap(QPointF(-distance(), -distance()), scaled);
    tmpPainter.end();

    // blur the alpha channel
    QImage blurred(tmpImg.size(), QImage::Format_ARGB32_Premultiplied);
    blurred.fill(0);
    QPainter blurPainter(&blurred);
    qt_blurImage(&blurPainter, tmpImg, blurRadius(), false, true);
    blurPainter.end();

    tmpImg = blurred;

    // blacken the image...
    tmpPainter.begin(&tmpImg);
    tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    tmpPainter.fillRect(tmpImg.rect(), color());
    tmpPainter.end();

    // draw the blurred shadow...
    painter->drawImage(offset, tmpImg);

    // draw the actual pixmap...
    painter->drawPixmap(offset, sourcePx, QRectF());

    // restore world transform
    painter->setWorldTransform(restoreTransform);
    painter->setOpacity(restoreOpacity);
}

/*!
  \brief 有效的边界区域
  \brief Calc the effective bounding rectangle

  \a rect 为控件的几何区域
  \a rect is the widget rectangle
  \return Effective bounding rectangle
  \return 有效的边界区域
 */
QRectF DGraphicsGlowEffect::boundingRectFor(const QRectF &rect) const
{
    qCDebug(logContainers) << "bounding rect for" << rect;
    qreal delta = blurRadius() + distance();
    return rect.united(rect.adjusted(-delta - xOffset(), -delta - yOffset(), delta - xOffset(), delta - yOffset()));
}

DWIDGET_END_NAMESPACE
