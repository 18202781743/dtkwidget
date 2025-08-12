// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dclipeffectwidget.h"
#include <DObjectPrivate>

#include <QEvent>
#include <QBackingStore>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logContainers, "dtk.widgets.containers")

#include <qpa/qplatformbackingstore.h>

DWIDGET_BEGIN_NAMESPACE

class DClipEffectWidgetPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    DClipEffectWidgetPrivate(DClipEffectWidget *qq);

    void updateImage();

    QImage image;
    QRectF imageGeometry;
    QPainterPath path;
    QMargins margins;

    QWidgetList parentList;

    Q_DECLARE_PUBLIC(DClipEffectWidget)
};

DClipEffectWidgetPrivate::DClipEffectWidgetPrivate(DClipEffectWidget *qq)
    : DObjectPrivate(qq)
{
    qCDebug(logContainers) << "Creating DClipEffectWidgetPrivate";
}

/*!
  \class Dtk::Widget::DClipEffectWidget
  \inmodule dtkwidget
  \brief 用于裁剪窗口的绘制内容.
  
  支持使用 QPainterPath 设置一个区域，位于区域外的窗口内容将被裁剪。被裁剪的对象
  是此控件的父控件, 且 DClipEffectWidget 会自动填充整个父控件，另外可以通过设置
  margins 控制有效的裁剪区域。如下图所示：
  \raw HTML
  <pre style="font-family: FreeMono, Consolas, Menlo, 'Noto Mono', 'Courier New', Courier, monospace;line-height: 100%;">
  ┏━━━━━━━━━━━━━━┯━━━━━━━━━━━━━━┓
  ┃              │              ┃
  ┃              │              ┃
  ┃     A        m              ┃
  ┃              │              ┃
  ┃              │              ┃
  ┃       ┏┅┅┅┅┅┅┷┅┅┅┅┅┅┓       ┃
  ┃       ┋B ╭───────╮  ┋       ┃
  ┃       ┋  │       │  ┋       ┃
  ┃── m ──┋  │   C   │  ┋── m ──┃
  ┃       ┋  │       │  ┋       ┃
  ┃       ┋  ╰───────╯  ┋       ┃
  ┃       ┗┅┅┅┅┅┅┯┅┅┅┅┅┅┛       ┃
  ┃              │              ┃
  ┃              │              ┃
  ┃              m              ┃
  ┃              │              ┃
  ┃              │              ┃
  ┗━━━━━━━━━━━━━━┷━━━━━━━━━━━━━━┛
  </pre>
  \endraw
  A为父控件区域， DClipEffectWidget 控件区域和父控件相同，B为有效的被裁剪区域，C
  为 clipPath 区域，m为 margins ，则被裁剪掉的区域为： B - C，裁掉的部分将显示
  下层控件内容，如果下层没有其它控件，将显示主窗口背景。
  
  此控件不接收任何输入事件，且不接受焦点
  \sa Qt::WA_TransparentForMouseEvents Qt::NoFocus
  \sa DGraphicsClipEffect
 */

/*!
  \fn void DClipEffectWidget::marginsChanged(QMargins margins)
  这个信号在 \a margins 改变时被发送
  */

/*!
  \fn void DClipEffectWidget::clipPathChanged(QPainterPath clipPath)
  这个信号在 \a clipPath 改变时被发送
  */

/*!
  \brief 构造 DClipEffectWidget 对象，和普通 QWidget 一样，可以传入一个 QWidget 对象
  指针作为其父对象
  \a parent 父对象
  \warning 必须要传入一个有效的父对象，将此控件作为顶级窗口没有任何意义
 */
DClipEffectWidget::DClipEffectWidget(QWidget *parent)
    : QWidget(parent)
    , DObject(*new DClipEffectWidgetPrivate(this))
{
    qCDebug(logContainers) << "Creating DClipEffectWidget";
    Q_ASSERT(parent);

    setAttribute(Qt::WA_TransparentForMouseEvents);
    setFocusPolicy(Qt::NoFocus);
}

/*!
  \property DClipEffectWidget::margins
  \brief 用于调整控件的被裁剪区域，可以和 clipPath 组合更加灵活的裁剪控件
 */
QMargins DClipEffectWidget::margins() const
{
    qCDebug(logContainers) << "Getting margins:" << d->margins;
    D_DC(DClipEffectWidget);

    return d->margins;
}

/*!
  \property DClipEffectWidget::clipPath
  \brief 被裁剪的区域，控件真实裁剪区域 = clipPath & (rect() - margins)
  \warning clipPath 必须是一个封闭的环状路径
  \sa QPainterPath::closeSubpath
 */
QPainterPath DClipEffectWidget::clipPath() const
{
    qCDebug(logContainers) << "Getting clip path, element count:" << d->path.elementCount();
    D_DC(DClipEffectWidget);

    return d->path;
}

/*!
  \a margins
 */
void DClipEffectWidget::setMargins(QMargins margins)
{
    qCDebug(logContainers) << "Setting margins to" << margins;
    D_D(DClipEffectWidget);

    if (d->margins == margins) {
        qCDebug(logContainers) << "Margins unchanged, skipping update";
        return;
    }

    QMargins oldMargins = d->margins;
    d->margins = margins;
    qCDebug(logContainers) << "Margins changed from" << oldMargins << "to" << margins;
    update();

    qCDebug(logContainers) << "Emitting marginsChanged signal";
    Q_EMIT marginsChanged(margins);
}

/*!
  \a path
 */
void DClipEffectWidget::setClipPath(const QPainterPath &path)
{
    qCDebug(logContainers) << "Setting clip path with" << path.elementCount() << "elements";
    D_D(DClipEffectWidget);

    if (d->path == path) {
        qCDebug(logContainers) << "Clip path unchanged, skipping update";
        return;
    }

    qCDebug(logContainers) << "Clip path changed, clearing cached image";
    d->path = path;
    d->image = QImage();

    qCDebug(logContainers) << "Emitting clipPathChanged signal";
    Q_EMIT clipPathChanged(d->path);

    update();
}

inline QRectF multiply(const QRectF &rect, qreal scale)
{
    return QRectF(rect.topLeft() * scale, rect.size() * scale);
}

inline QRectF divide(const QRectF &rect, qreal scale)
{
    return multiply(rect, 1.0 / scale);
}

bool DClipEffectWidget::eventFilter(QObject *watched, QEvent *event)
{
    qCDebug(logContainers) << "Event filter, event type:" << event->type() << "watched:" << watched;
    D_D(DClipEffectWidget);

    if (event->type() == QEvent::Move) {
        qCDebug(logContainers) << "Move event detected, clearing cached image";
        d->image = QImage();
    }

    if (watched != parent()) {
        qCDebug(logContainers) << "Event not from parent widget, ignoring";
        return false;
    }

    if (event->type() == QEvent::Paint) {
        qCDebug(logContainers) << "Paint event from parent widget";
        const QPoint &offset = mapTo(window(), QPoint(0, 0));
        qCDebug(logContainers) << "Offset to window:" << offset;
        
        if (!window() || !window()->backingStore()) {
            qCDebug(logContainers) << "No window or backing store available";
            return false;
        }
        
        const QImage &image = window()->backingStore()->handle()->toImage();
        qreal scale = devicePixelRatioF();
        qCDebug(logContainers) << "Device pixel ratio:" << scale << "image size:" << image.size();

        d->imageGeometry = QRectF(image.rect()) & multiply(QRect(offset, size()), scale);
        qCDebug(logContainers) << "Image geometry:" << d->imageGeometry;

        if (d->image.isNull() || d->imageGeometry.size() != d->image.size()) {
            qCDebug(logContainers) << "Creating new cached image";
            d->image = image.copy(d->imageGeometry.toRect());
            d->image.setDevicePixelRatio(scale);
        } else {
            qCDebug(logContainers) << "Updating existing cached image";
            QPaintEvent *e = static_cast<QPaintEvent*>(event);
            QPainter p;
            // 此控件位置一直为 0,0，且大小和父控件一致，所以offset也是父控件相对于顶级窗口的偏移
            const QRectF &rect = QRectF(image.rect()) & multiply(e->rect().translated(offset), scale);
            qCDebug(logContainers) << "Update rect:" << rect;

            d->image.setDevicePixelRatio(image.devicePixelRatio());

            if (p.begin(&d->image)) {
                p.setCompositionMode(QPainter::CompositionMode_Source);
                p.drawImage(rect.topLeft() - d->imageGeometry.topLeft(), image.copy(rect.toRect()));
                p.end();
                qCDebug(logContainers) << "Image update completed";
            } else {
                qCDebug(logContainers) << "Failed to begin painting on cached image";
            }

            d->image.setDevicePixelRatio(scale);
        }
    } else if (event->type() == QEvent::Resize) {
        qCDebug(logContainers) << "Resize event from parent widget";
        if (parentWidget()) {
            qCDebug(logContainers) << "Resizing to parent size:" << parentWidget()->size();
            resize(parentWidget()->size());
        } else {
            qCDebug(logContainers) << "No parent widget for resize";
        }
    }

    return false;
}

void DClipEffectWidget::paintEvent(QPaintEvent *event)
{
    qCDebug(logContainers) << "Paint event, rect:" << event->rect();
    D_DC(DClipEffectWidget);

    if (d->image.isNull()) {
        qCDebug(logContainers) << "Cached image is null, skipping paint";
        return;
    }

    qreal devicePixelRatio = devicePixelRatioF();
    qCDebug(logContainers) << "Device pixel ratio:" << devicePixelRatio;
    
    const QRectF &rect = QRectF(event->rect()) & QRectF(this->rect()).marginsRemoved(d->margins);
    qCDebug(logContainers) << "Effective paint rect:" << rect << "margins:" << d->margins;
    
    const QPoint &offset = mapTo(window(), QPoint(0, 0));
    const QRectF &imageRect = multiply(rect, devicePixelRatio) & d->imageGeometry.translated(-offset * devicePixelRatio);
    qCDebug(logContainers) << "Image rect:" << imageRect << "offset:" << offset;

    if (!imageRect.isValid()) {
        qCDebug(logContainers) << "Image rect is invalid, skipping paint";
        return;
    }

    QPainter p(this);
    QPainterPath newPath;

    newPath.addRect(this->rect());
    newPath -= d->path;
    qCDebug(logContainers) << "Clip path has" << newPath.elementCount() << "elements";

    p.setRenderHint(QPainter::Antialiasing);
    p.setClipPath(newPath);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.drawImage(imageRect.topLeft() / devicePixelRatio, d->image, imageRect);
    qCDebug(logContainers) << "Paint completed";
}

void DClipEffectWidget::resizeEvent(QResizeEvent *event)
{
    qCDebug(logContainers) << "Resize event, old size:" << event->oldSize() << "new size:" << event->size();
    D_D(DClipEffectWidget);

    qCDebug(logContainers) << "Clearing cached image due to resize";
    d->image = QImage();

    QWidget::resizeEvent(event);
}

void DClipEffectWidget::showEvent(QShowEvent *event)
{
    qCDebug(logContainers) << "Show event";
    D_D(DClipEffectWidget);

    d->parentList.clear();
    qCDebug(logContainers) << "Cleared parent list";

    QWidget *pw = parentWidget();
    qCDebug(logContainers) << "Starting parent widget traversal from:" << pw;

    while (pw && !pw->isWindow()) {
        qCDebug(logContainers) << "Adding parent widget to list and installing event filter:" << pw;
        d->parentList << pw;

        pw->installEventFilter(this);
        pw = pw->parentWidget();
    }

    qCDebug(logContainers) << "Parent list size after traversal:" << d->parentList.size();
    
    if (parentWidget()) {
        qCDebug(logContainers) << "Resizing to parent size:" << parentWidget()->size();
        resize(parentWidget()->size());
    } else {
        qCDebug(logContainers) << "No parent widget found for resizing";
    }

    QWidget::showEvent(event);
}

void DClipEffectWidget::hideEvent(QHideEvent *event)
{
    qCDebug(logContainers) << "Hide event, parent list size:" << d->parentList.size();
    D_D(DClipEffectWidget);

    int removedCount = 0;
    for (QWidget *w : d->parentList) {
        if (w) {
            qCDebug(logContainers) << "Removing event filter from parent widget:" << w;
            w->removeEventFilter(this);
            removedCount++;
        } else {
            qCDebug(logContainers) << "Found null parent widget in list";
        }
    }

    qCDebug(logContainers) << "Removed event filters from" << removedCount << "parent widgets";
    d->parentList.clear();
    qCDebug(logContainers) << "Parent list cleared";

    QWidget::hideEvent(event);
}

DWIDGET_END_NAMESPACE
