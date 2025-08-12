// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dimagevieweritems_p.h"

#include <QObject>
#include <QMovie>
#include <QPainter>
#include <QLoggingCategory>
#include <QStyleOption>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <DIconTheme>

DGUI_USE_NAMESPACE
DWIDGET_BEGIN_NAMESPACE

namespace {
Q_DECLARE_LOGGING_CATEGORY(logMediaImage)
}

DGraphicsPixmapItem::DGraphicsPixmapItem(QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent)
{
    qCDebug(logMediaImage) << "Creating graphics pixmap item with parent:" << parent;
}

DGraphicsPixmapItem::DGraphicsPixmapItem(const QPixmap &pixmap, QGraphicsItem *parent)
    : QGraphicsPixmapItem(pixmap, parent)
{
    qCDebug(logMediaImage) << "Creating graphics pixmap item with pixmap size:" << pixmap.size() << "and parent:" << parent;
}

DGraphicsPixmapItem::~DGraphicsPixmapItem()
{
    qCDebug(logMediaImage) << "Destroying graphics pixmap item";
    prepareGeometryChange();
}

void DGraphicsPixmapItem::setPixmap(const QPixmap &pixmap)
{
    qCDebug(logMediaImage) << "Setting pixmap, size:" << pixmap.size();
    cachePixmap = qMakePair(cachePixmap.first, pixmap);
    QGraphicsPixmapItem::setPixmap(pixmap);
}

void DGraphicsPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    qCDebug(logMediaImage) << "Painting graphics pixmap item";
    const QTransform ts = painter->transform();
    qCDebug(logMediaImage) << "Transform type:" << ts.type() << "scale:" << ts.m11();

    if (ts.type() == QTransform::TxScale && ts.m11() < 1) {
        QPixmap currentPixmap = pixmap();
        qCDebug(logMediaImage) << "Current pixmap size:" << currentPixmap.size();

        if (currentPixmap.width() < 10000 && currentPixmap.height() < 10000) {
            qCDebug(logMediaImage) << "Using optimized painting path";
            painter->setRenderHint(QPainter::SmoothPixmapTransform, (transformationMode() == Qt::SmoothTransformation));

            QPixmap pixmap;
            if (qIsNull(cachePixmap.first - ts.m11())) {
                qCDebug(logMediaImage) << "Using cached pixmap";
                pixmap = cachePixmap.second;
            } else {
                qCDebug(logMediaImage) << "Transforming pixmap";
                pixmap = currentPixmap.transformed(painter->transform(), transformationMode());
                cachePixmap = qMakePair(ts.m11(), pixmap);
            }

            qCDebug(logMediaImage) << "Setting device pixel ratio:" << painter->device()->devicePixelRatioF();
            pixmap.setDevicePixelRatio(painter->device()->devicePixelRatioF());
            painter->resetTransform();
            painter->drawPixmap(offset() + QPointF(ts.dx(), ts.dy()), pixmap);
            painter->setTransform(ts);
        } else {
            qCDebug(logMediaImage) << "Pixmap too large, using default painting";
            QGraphicsPixmapItem::paint(painter, option, widget);
        }
    } else {
        qCDebug(logMediaImage) << "Using default painting";
        QGraphicsPixmapItem::paint(painter, option, widget);
    }
    qCDebug(logMediaImage) << "Painting completed";
}

DGraphicsMovieItem::DGraphicsMovieItem(QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent)
{
    qCDebug(logMediaImage) << "Creating movie item with parent:" << parent;
    movie = new QMovie;
    QObject::connect(movie, &QMovie::frameChanged, this, &DGraphicsMovieItem::onMovieFrameChanged);
    qCDebug(logMediaImage) << "Movie item created and connected";
}

DGraphicsMovieItem::DGraphicsMovieItem(const QString &fileName, QGraphicsItem *parent)
    : QGraphicsPixmapItem(fileName, parent)
{
    qCDebug(logMediaImage) << "Creating movie item with file:" << fileName << "and parent:" << parent;
    movie = new QMovie;
    QObject::connect(movie, &QMovie::frameChanged, this, &DGraphicsMovieItem::onMovieFrameChanged);
    setFileName(fileName);
    qCDebug(logMediaImage) << "Movie item created and initialized";
}

DGraphicsMovieItem::~DGraphicsMovieItem()
{
    qCDebug(logMediaImage) << "Destroying movie item";
    prepareGeometryChange();

    movie->stop();
    movie->deleteLater();
    movie = nullptr;
    qCDebug(logMediaImage) << "Movie item destroyed";
}

void DGraphicsMovieItem::onMovieFrameChanged()
{
    qCDebug(logMediaImage) << "Movie frame changed";
    setPixmap(movie->currentPixmap());
}

void DGraphicsMovieItem::setFileName(const QString &fileName)
{
    qCDebug(logMediaImage) << "Setting movie file name:" << fileName;
    movie->stop();
    movie->setFileName(fileName);
    movie->start();
    qCDebug(logMediaImage) << "Movie started with new file";

    update();
}

DGraphicsSVGItem::DGraphicsSVGItem(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    qCDebug(logMediaImage) << "Creating SVG item with parent:" << parent;
    renderer = new DSvgRenderer(this);
    qCDebug(logMediaImage) << "SVG renderer created";
}

DGraphicsSVGItem::DGraphicsSVGItem(const QString &fileName, QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    qCDebug(logMediaImage) << "Creating SVG item with file:" << fileName << "and parent:" << parent;
    renderer = new DSvgRenderer(this);
    renderer->load(fileName);
    updateDefaultSize();
    qCDebug(logMediaImage) << "SVG item created and initialized";
}

void DGraphicsSVGItem::setFileName(const QString &fileName)
{
    qCDebug(logMediaImage) << "Setting SVG file name:" << fileName;
    // Clear cached image.
    CacheMode mode = cacheMode();
    setCacheMode(QGraphicsItem::NoCache);
    renderer->load(fileName);
    updateDefaultSize();

    setCacheMode(mode);
    qCDebug(logMediaImage) << "SVG file loaded and cache updated";
    update();
}

QRectF DGraphicsSVGItem::boundingRect() const
{
    qCDebug(logMediaImage) << "Getting SVG bounding rect:" << imageRect;
    return imageRect;
}

void DGraphicsSVGItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    qCDebug(logMediaImage) << "Painting SVG item";
    if (!renderer->isValid()) {
        qCDebug(logMediaImage) << "SVG renderer is not valid, skipping paint";
        return;
    }
    renderer->render(painter, imageRect);
    qCDebug(logMediaImage) << "SVG item painted";
}

int DGraphicsSVGItem::type() const
{
    return Type;
}

void DGraphicsSVGItem::updateDefaultSize()
{
    qCDebug(logMediaImage) << "Updating SVG default size";
    QRectF bounds = QRectF(QPointF(0, 0), renderer->defaultSize());
    qCDebug(logMediaImage) << "New bounds:" << bounds;

    if (bounds.size() != imageRect.size()) {
        qCDebug(logMediaImage) << "Size changed, preparing geometry change";
        prepareGeometryChange();
        imageRect.setSize(bounds.size());
        qCDebug(logMediaImage) << "New image rect size:" << imageRect.size();
    }
}

DGraphicsCropItem::DGraphicsCropItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    qCDebug(logMediaImage) << "Creating crop item with parent:" << parent;
    updateContentItem(parent);
    setZValue(100);
    qCDebug(logMediaImage) << "Crop item created with z-value: 100";
}

void DGraphicsCropItem::updateContentItem(QGraphicsItem *parentItem)
{
    qCDebug(logMediaImage) << "Updating content item with parent:" << parentItem;
    setParentItem(parentItem);

    if (parentItem) {
        QRectF parentRect = parentItem->boundingRect();
        qCDebug(logMediaImage) << "Parent bounding rect:" << parentRect;
        originalParentRect = QRectF(0, 0, parentRect.width(), parentRect.height());
        itemRect = originalParentRect;
        qCDebug(logMediaImage) << "Set original and item rect to:" << originalParentRect;
    } else {
        qCDebug(logMediaImage) << "No parent item, setting empty rects";
        itemRect = QRectF();
        originalParentRect = QRectF();
    }

    update();
}

void DGraphicsCropItem::setCropMode(CropMode mode)
{
    qCDebug(logMediaImage) << "Setting crop mode to:" << mode;
    internalCropMode = mode;

    // CropFree will do nothing, using current item rect.
    switch (internalCropMode) {
        case CropOriginal:
            qCDebug(logMediaImage) << "Using original crop mode";
            itemRect = originalParentRect;
            break;
        case AspectRatio1x1:
            qCDebug(logMediaImage) << "Setting 1:1 aspect ratio";
            setAspectRatio(1.0, 1.0);
            break;
        case AspectRatio16x9:
            qCDebug(logMediaImage) << "Setting 16:9 aspect ratio";
            setAspectRatio(16, 9);
            break;
        case AspectRatio9x16:
            qCDebug(logMediaImage) << "Setting 9:16 aspect ratio";
            setAspectRatio(9.0, 16.0);
            break;
        case AspectRatio4x3:
            qCDebug(logMediaImage) << "Setting 4:3 aspect ratio";
            setAspectRatio(4.0, 3.0);
            break;
        case AspectRatio3x4:
            qCDebug(logMediaImage) << "Setting 3:4 aspect ratio";
            setAspectRatio(3.0, 4.0);
            break;
        case AspectRatio3x2:
            qCDebug(logMediaImage) << "Setting 3:2 aspect ratio";
            setAspectRatio(3.0, 2.0);
            break;
        case AspectRatio2x3:
            qCDebug(logMediaImage) << "Setting 2:3 aspect ratio";
            setAspectRatio(2.0, 3.0);
            break;
        default:
            qCDebug(logMediaImage) << "Using default crop mode";
            break;
    }

    update();
}

DGraphicsCropItem::CropMode DGraphicsCropItem::cropMode() const
{
    qCDebug(logMediaImage) << "Getting current crop mode:" << internalCropMode;
    return internalCropMode;
}

void DGraphicsCropItem::setAspectRatio(qreal w, qreal h)
{
    qCDebug(logMediaImage) << "Setting aspect ratio:" << w << ":" << h;
    qreal calcWidth = originalParentRect.width();
    qreal calcHeight = calcWidth * h / w;
    // Check parent item rotate.
    if (parentItem()) {
        int rotate = qRound(parentItem()->rotation());
        qCDebug(logMediaImage) << "Parent rotation:" << rotate;
        if (rotate % 180) {
            qCDebug(logMediaImage) << "Adjusting height for rotation";
            calcHeight = calcWidth * w / h;
        }
    }

    // Make sure parent rectangle contain item rectangle.
    if (calcWidth > originalParentRect.width() || calcHeight > originalParentRect.height()) {
        qCDebug(logMediaImage) << "Adjusting dimensions to fit parent rect";
        qreal aspectRatio = originalParentRect.width() / originalParentRect.height();
        qreal calcRatio = calcWidth / calcHeight;
        if (calcRatio > aspectRatio) {
            calcWidth = originalParentRect.width();
            calcHeight = (calcWidth / calcRatio);
            qCDebug(logMediaImage) << "Adjusted to width:" << calcWidth << "height:" << calcHeight;
        } else {
            calcHeight = originalParentRect.height();
            calcWidth = calcHeight * calcRatio;
            qCDebug(logMediaImage) << "Adjusted to width:" << calcWidth << "height:" << calcHeight;
        }
    }

    // Move rectangle to center.
    QPointF center = originalParentRect.center();
    QPointF topLeft = center - QPointF(calcWidth / 2, calcHeight / 2);
    QPointF bottomRight = center + QPointF(calcWidth / 2, calcHeight / 2);
    itemRect = QRectF(topLeft, bottomRight);
    qCDebug(logMediaImage) << "Set item rect to:" << itemRect;

    update();
}

void DGraphicsCropItem::setRect(const QRectF &rect)
{
    qCDebug(logMediaImage) << "Setting crop rect to:" << rect;
    prepareGeometryChange();
    itemRect = validRect(rect);
    qCDebug(logMediaImage) << "Validated rect set to:" << itemRect;

    update();
}

void DGraphicsCropItem::setSize(qreal width, qreal height)
{
    qCDebug(logMediaImage) << "Setting crop size to width:" << width << "height:" << height;
    prepareGeometryChange();
    itemRect = validRect(itemRect.adjusted(0, 0, width, height));
    qCDebug(logMediaImage) << "New item rect after size adjustment:" << itemRect;
    update();
}

void DGraphicsCropItem::move(qreal x, qreal y)
{
    qCDebug(logMediaImage) << "Moving crop item to:" << x << "," << y;
    prepareGeometryChange();
    itemRect.moveTo(x, y);
    qCDebug(logMediaImage) << "Item rect moved to:" << itemRect;
    update();
}

QRect DGraphicsCropItem::cropRect() const
{
    QRect rect = mapRectToParent(itemRect).toRect();
    qCDebug(logMediaImage) << "Getting crop rect in parent coordinates:" << rect;
    return rect;
}

void DGraphicsCropItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    qCDebug(logMediaImage) << "Starting to paint crop item";
    painter->setClipping(false);
    QColor activeColor = QColor(Qt::cyan);
    qreal penWidth = 0;
    if (parentItem() && !qFuzzyIsNull(parentItem()->rotation())) {
        qCDebug(logMediaImage) << "Adjusting pen width for rotated parent";
        auto transform = painter->worldTransform();
        transform.rotate(-parentItem()->rotation());
        penWidth = 1.0 / transform.m11();
    } else {
        penWidth = 1.0 / painter->worldTransform().m11();
    }
    qCDebug(logMediaImage) << "Using pen width:" << penWidth;

    QRectF rct = itemRect.adjusted(penWidth, penWidth, -penWidth, -penWidth);
    QPen pen;
    pen.setStyle(Qt::SolidLine);
    pen.setColor(QColor("#EDEDED"));
    pen.setWidthF(penWidth * 3);

    // Draw the lines that divide the rectangle.
    qCDebug(logMediaImage) << "Drawing trisector lines";
    painter->save();
    painter->setPen(pen);
    drawTrisectorRect(painter);
    painter->restore();

    // Draw border shadow, shadow witdh is 5 times the width of pen.
    qCDebug(logMediaImage) << "Drawing border shadow";
    painter->save();
    QPen rectPen(pen);
    rectPen.setStyle(Qt::SolidLine);
    QColor rectColor(activeColor);
    rectColor.setAlpha(26);
    rectPen.setColor(rectColor);
    rectPen.setWidthF(penWidth * 5);
    painter->setPen(rectPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rct);
    painter->restore();

    qCDebug(logMediaImage) << "Drawing dashed border";
    painter->save();
    rectPen.setStyle(Qt::DashLine);
    rectPen.setColor(activeColor);
    rectPen.setWidthF(penWidth);
    painter->setPen(rectPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rct);
    painter->restore();

    qCDebug(logMediaImage) << "Drawing corner handles";
    painter->save();
    drawCornerHandle(painter);
    painter->restore();

    // Draw cross fill background.
    qCDebug(logMediaImage) << "Drawing background fill";
    painter->save();
    QRectF sceneRct = scene()->sceneRect();
    QRectF itemSceneRect = sceneBoundingRect();
    QRegion r1(sceneRct.toRect());
    QRegion r2(itemSceneRect.toRect());
    QRegion r3 = r2.subtracted(r1);
    QPainterPath path;
    path.addRegion(r3);

    QColor background(activeColor);
    background.setAlpha(26);
    painter->setPen(Qt::NoPen);
    painter->setBrush(background);
    painter->drawPath(path);
    painter->restore();
    qCDebug(logMediaImage) << "Crop item painting completed";
}

void DGraphicsCropItem::drawTrisectorRect(QPainter *painter)
{
    qCDebug(logMediaImage) << "Drawing trisector rectangle";
    qreal penWidth = 0;
    if (parentItem() && !qFuzzyIsNull(parentItem()->rotation())) {
        qCDebug(logMediaImage) << "Adjusting pen width for rotated parent";
        auto transform = painter->worldTransform();
        transform.rotate(-parentItem()->rotation());
        penWidth = 1.0 / transform.m11();
    } else {
        penWidth = 1.0 / painter->worldTransform().m11();
    }
    qCDebug(logMediaImage) << "Using pen width:" << penWidth;

    QPainterPath path;
    QRectF rct = itemRect.adjusted(penWidth, penWidth, -penWidth, -penWidth);
    qCDebug(logMediaImage) << "Drawing horizontal trisector lines";
    path.moveTo(rct.x(), rct.y() + rct.height() / 3);
    path.lineTo(rct.x() + rct.width(), rct.y() + rct.height() / 3);

    path.moveTo(rct.x(), rct.y() + rct.height() / 3 * 2);
    path.lineTo(rct.x() + rct.width(), rct.y() + rct.height() / 3 * 2);

    qCDebug(logMediaImage) << "Drawing vertical trisector lines";
    path.moveTo(rct.x() + rct.width() / 3, rct.y());
    path.lineTo(rct.x() + rct.width() / 3, rct.y() + rct.height());

    path.moveTo(rct.x() + rct.width() / 3 * 2, rct.y());
    path.lineTo(rct.x() + rct.width() / 3 * 2, rct.y() + rct.height());

    painter->drawPath(path);
    qCDebug(logMediaImage) << "Trisector rectangle drawing completed";
}

void DGraphicsCropItem::drawCornerHandle(QPainter *painter)
{
    qCDebug(logMediaImage) << "Drawing corner handles";
    QGraphicsView *view = contentView();
    if (!view) {
        qCDebug(logMediaImage) << "No graphics view available, skipping corner handles";
        return;
    }

    qreal penWidth = 0;
    if (parentItem() && !qFuzzyIsNull(parentItem()->rotation())) {
        qCDebug(logMediaImage) << "Adjusting pen width for rotated parent";
        auto transform = painter->worldTransform();
        transform.rotate(-parentItem()->rotation());
        penWidth = 1.0 / transform.m11();
    } else {
        penWidth = 1.0 / painter->worldTransform().m11();
    }
    qCDebug(logMediaImage) << "Using pen width:" << penWidth;

    QRectF rct = itemRect.adjusted(penWidth, penWidth, -penWidth, -penWidth);

    const qreal minlenth = 24;
    qreal scale = view->transform().m11();
    qreal showW = itemRect.width() * scale;
    qreal showH = itemRect.height() * scale;
    qCDebug(logMediaImage) << "Item display dimensions - width:" << showW << "height:" << showH;

    if (showW < minlenth || showH < minlenth) {
        qCDebug(logMediaImage) << "Drawing simplified corner handles for small item";
        // When the clipping area is smaller than the corner picture, the brush is drawn
        QPen pen(painter->pen());
        pen.setWidthF(1.0);
        pen.setColor(QColor("#EDEDED"));
        pen.setStyle(Qt::SolidLine);

        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(rct);

        QLineF topLine = QLineF(rct.topLeft(), rct.topRight());
        QLineF botLine = QLineF(rct.bottomLeft(), rct.bottomRight());
        QLineF leftLine = QLineF(rct.topLeft(), rct.bottomLeft());
        QLineF rightLine = QLineF(rct.topRight(), rct.bottomRight());

        qCDebug(logMediaImage) << "Drawing corner lines";
        painter->drawLine(topLine.p1(), topLine.center());
        painter->drawLine(rightLine.p1(), rightLine.center());
        painter->drawLine(botLine.center(), botLine.p2());
        painter->drawLine(leftLine.center(), leftLine.p2());

        painter->drawLine(topLine.center(), topLine.p2());
        painter->drawLine(rightLine.center(), rightLine.p2());
        painter->drawLine(botLine.center(), botLine.p1());
        painter->drawLine(leftLine.center(), leftLine.p1());
    } else {
        qCDebug(logMediaImage) << "Drawing corner handle icons";
        painter->save();

        int pixWidth = 20;
        int offset = 2;
        int offsetWidth = 18;

        auto painterTopLeft = view->mapFromScene(sceneBoundingRect().topLeft());
        auto painterBottomRight = view->mapFromScene(sceneBoundingRect().bottomRight());
        rct = QRectF(painterTopLeft, painterBottomRight);
        qCDebug(logMediaImage) << "Corner handle rect:" << rct;

        // Reset tranform, keep corner same size
        painter->resetTransform();

        // Draw four corner handles.
        qCDebug(logMediaImage) << "Drawing top-left corner handle";
        QPixmap cornerPixmap = DIconTheme::findQIcon("selection_topleft").pixmap(QSize(pixWidth, pixWidth));
        QRectF cornerRect = QRectF(painterTopLeft + QPointF(-offset, -offset), painterTopLeft + QPointF(pixWidth, pixWidth));
        painter->drawPixmap(cornerRect, cornerPixmap, QRectF(0, 0, cornerPixmap.width(), cornerPixmap.height()));

        qCDebug(logMediaImage) << "Drawing top-right corner handle";
        cornerPixmap = DIconTheme::findQIcon("selection_topright").pixmap(QSize(pixWidth, pixWidth));
        cornerRect =
            QRectF(QPointF(painterBottomRight.x() - offsetWidth, painterTopLeft.y() - offset), QSizeF(pixWidth, pixWidth));
        painter->drawPixmap(cornerRect, cornerPixmap, QRectF(0, 0, cornerPixmap.width(), cornerPixmap.height()));

        qCDebug(logMediaImage) << "Drawing bottom-right corner handle";
        cornerPixmap = DIconTheme::findQIcon("selection_bottomright").pixmap(QSize(pixWidth, pixWidth));
        cornerRect = QRectF(rct.bottomRight() + QPointF(-pixWidth, -pixWidth), rct.bottomRight() + QPointF(offset, offset));
        painter->drawPixmap(cornerRect, cornerPixmap, QRectF(0, 0, cornerPixmap.width(), cornerPixmap.height()));

        qCDebug(logMediaImage) << "Drawing bottom-left corner handle";
        cornerPixmap = DIconTheme::findQIcon("selection_bottomleft").pixmap(QSize(pixWidth, pixWidth));
        cornerRect = QRectF(QPointF(rct.x() - offset, rct.y() + rct.height() - offsetWidth), QSizeF(pixWidth, pixWidth));
        painter->drawPixmap(cornerRect, cornerPixmap, QRectF(0, 0, cornerPixmap.width(), cornerPixmap.height()));

        painter->restore();
    }
    qCDebug(logMediaImage) << "Corner handles drawing completed";
}

QRectF DGraphicsCropItem::boundingRect() const
{
    return itemRect;
}

void DGraphicsCropItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!mousePressed) {
        handleType = detectHandleType(event->scenePos());
        mousePressed = true;
    }
}

void DGraphicsCropItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (mousePressed) {
        QPointF offset = event->pos() - event->lastPos();
        QRectF newRect = itemRect;
        updateRect(newRect, offset, handleType);

        if (newRect != itemRect) {
            itemRect = newRect;
            update();
        }
    }
}

void DGraphicsCropItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    if (mousePressed) {
        // Reset handle mode.
        handleType = Move;
        mousePressed = false;
    }
}

DGraphicsCropItem::MoveHandleType DGraphicsCropItem::detectHandleType(const QPointF &mousePoint) const
{
    MoveHandleType type = Move;
    QGraphicsView *view = contentView();
    if (view) {
        QRectF itemSceneRect = sceneBoundingRect();
        QPointF itemViewTopLeft = view->mapFromScene(itemSceneRect.topLeft());
        QPointF itemViewBottomRight = view->mapFromScene(itemSceneRect.bottomRight());
        QRectF itemViewRect(itemViewTopLeft, itemViewBottomRight);
        QPointF mouseViewPoint = view->mapFromScene(mousePoint);

        // Detect which corner handle.
        if (itemViewRect.left() <= mouseViewPoint.x() && mouseViewPoint.x() <= itemViewRect.left() + HandleSize) {
            if (itemViewRect.top() <= mouseViewPoint.y() && mouseViewPoint.y() <= itemViewRect.top() + HandleSize) {
                type = TopLeft;
            } else if (itemViewRect.bottom() - HandleSize <= mouseViewPoint.y() && mouseViewPoint.y() <= itemViewRect.bottom()) {
                type = BottomLeft;
            }
        } else if (itemViewRect.right() - HandleSize <= mouseViewPoint.x() && mouseViewPoint.x() <= itemViewRect.right()) {
            if (itemViewRect.top() <= mouseViewPoint.y() && mouseViewPoint.y() <= itemViewRect.top() + HandleSize) {
                type = TopRight;
            } else if (itemViewRect.bottom() - HandleSize <= mouseViewPoint.y() && mouseViewPoint.y() <= itemViewRect.bottom()) {
                type = BottomRight;
            }
        }
    }

    // Warning: Must refactor code while the enum MoveHandleType changed.
    if (Move != type && parentItem()) {
        // Adjust handle type while parent item is rotate.
        int rotation = qRound(parentItem()->rotation());
        int rotate90Count = rotation / 90;

        // 4 is corner enum count.
        type = MoveHandleType((int(type) + 4 - rotate90Count) % 4);
    }

    return type;
}

void DGraphicsCropItem::updateRect(QRectF &rect, const QPointF &offset, MoveHandleType type)
{
    switch (type) {
        case Move:  // Move top left point, not resize.
            rect.moveTop(qBound<qreal>(0, rect.top() + offset.y(), originalParentRect.height() - rect.height()));
            rect.moveLeft(qBound<qreal>(0, rect.left() + offset.x(), originalParentRect.width() - rect.width()));
            break;
        case TopLeft:  // Will change item rectangle.
            rect.setTop(qBound<qreal>(0, rect.top() + offset.y(), rect.bottom() - MinimalSize));
            rect.setLeft(qBound<qreal>(0, rect.left() + offset.x(), rect.right() - MinimalSize));
            break;
        case TopRight:
            rect.setTop(qBound<qreal>(0, rect.top() + offset.y(), rect.bottom() - MinimalSize));
            rect.setRight(qBound<qreal>(MinimalSize, rect.right() + offset.x(), originalParentRect.width()));
            break;
        case BottomRight:
            rect.setBottom(qBound<qreal>(MinimalSize, rect.bottom() + offset.y(), originalParentRect.height()));
            rect.setRight(qBound<qreal>(MinimalSize, rect.right() + offset.x(), originalParentRect.width()));
            break;
        case BottomLeft:
            rect.setBottom(qBound<qreal>(MinimalSize, rect.bottom() + offset.y(), originalParentRect.height()));
            rect.setLeft(qBound<qreal>(0, rect.left() + offset.x(), rect.right() - MinimalSize));
            break;
        default:
            break;
    }
}

QRectF DGraphicsCropItem::validRect(const QRectF &rect) const
{
    if (parentItem()) {
        return rect.intersected(parentItem()->boundingRect());
    }
    return rect;
}

QGraphicsView *DGraphicsCropItem::contentView() const
{
    if (scene()) {
        auto viewList = scene()->views();
        if (!viewList.isEmpty()) {
            return viewList.first();
        }
    }

    return nullptr;
}

DWIDGET_END_NAMESPACE
