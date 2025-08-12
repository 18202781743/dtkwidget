// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "darrowrectangle.h"
#include "dplatformwindowhandle.h"
#include "dapplication.h"
#include "private/darrowrectangle_p.h"
#include "dgraphicsgloweffect.h"
#include "dstyle.h"

#include <DGuiApplicationHelper>
#include<qpa/qplatformnativeinterface.h>
#ifdef Q_OS_LINUX
#include <X11/extensions/shape.h>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QX11Info>
#else
#include <QtGui/private/qtx11extras_p.h>
#endif
#endif
#include <QApplication>
#include <QScreen>
#include <QEvent>
#include <QLoggingCategory>

DWIDGET_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(logWindowPlatform)

static bool isDwayland()
{
    const auto& isDwayland = qApp->platformName() == "dwayland" || qApp->property("_d_isDwayland").toBool();
    qCDebug(logWindowPlatform) << "Checking if platform is dwayland:" << isDwayland;
    return isDwayland;
}
/*!
@~english
  @class Dtk::Widget::DArrowRectangle
  @ingroup dtkwidget
  @brief The DArrowRectangle class provides a widget that has an arrow on one
  of its four borders.

  It's usually used as a container of some other widgets,
  see DArrowRectangle::setContent()

  \sa DArrowRectangle::setContent()
 */

/*!
@~english
  @enum Dtk::Widget::DArrowRectangle::ArrowDirection
  @brief The ArrowDirection enum contains the possible directions that
  the DArrowRectangle's arrow may point to.

  \value ArrowLeft
  indicates the arrow of this rectangle will point left
  \value ArrowRight
  indicates the arrow of this rectangle will point right
  \value ArrowTop
  indicates the arrow of this rectangle will point up
  \value ArrowBottom
  indicates the arrow of this rectangle will point down
 */

/*!
@~english
  @enum Dtk::Widget::DArrowRectangle::FloatMode
  @brief The FloatMode enum decide the WindowType when DArrowRectangle show
  
  The floating mode of the control indicates how the control is displayed in the layout. 
  DArrowRectangle::FloatWindow indicates that the control will be displayed as a separate window, 
  while DArrowRectangle::FloatWidget indicates that the control can only be displayed in the layout of its parent control and cannot exceed the size of the parent control.

  \value FloatWindow
  Window mode
  Window will show a separate window
  \value FloatWidget
  Widget mode
  Widget must by show in the rect of parentWidget
 */

/*!
@~english
  @brief DArrowRectangle::DArrowRectangle constructs an instance of DArrowRectangle.

  \a direction is used to initialize the direction of which the arrow
  points to.
  \a parent is the parent widget the arrow rectangle will be attached to.
 */
DArrowRectangle::DArrowRectangle(ArrowDirection direction, QWidget *parent) :
    QWidget(parent),
    DObject(*new DArrowRectanglePrivate(direction, this))
{
    qCDebug(logWindowPlatform) << "Creating DArrowRectangle with direction:" << static_cast<int>(direction) << "parent:" << parent;
    D_D(DArrowRectangle);

    d->init(FloatWindow);
}

/*!
@~english
  @brief get the DArrowRectangle instance and specify the floating mode.
  @brief DArrowRectangle::DArrowRectangle can set DArrowRectangle show as a window or
  a widget in parentWidget by floatMode

  \a direction Used to initialize the direction of the arrow
  \a floatMode 
  \a parent as its parent control
  @sa DArrowRectangle::FloatMode
 */
DArrowRectangle::DArrowRectangle(ArrowDirection direction, FloatMode floatMode, QWidget *parent) :
    QWidget(parent),
    DObject(*new DArrowRectanglePrivate(direction, this))
{
    qCDebug(logWindowPlatform) << "Creating DArrowRectangle with direction:" << static_cast<int>(direction) << "floatMode:" << static_cast<int>(floatMode) << "parent:" << parent;
    D_D(DArrowRectangle);

    d->init(floatMode);
}

/*!
@~english
  @brief DArrowRectangle::show shows the widget at the given coordinate.
  
  @note The coordiate is calculated to be the arrow head's position, so you
  don't need to calculate the position yourself.

  \a x is the x coordinate of the arrow head.
  \a y is the y coordinate of the arrow head.
 */
void DArrowRectangle::show(int x, int y)
{
    qCDebug(logWindowPlatform) << "Showing DArrowRectangle at position:" << x << y;
    D_D(DArrowRectangle);

    d->show(x, y);
}

/*!
@~english
  @brief DArrowRectangle::setContent sets the content of the arrow rectangle.

  \a content to display a content control
 */
void DArrowRectangle::setContent(QWidget *content)
{
    qCDebug(logWindowPlatform) << "Setting content widget:" << content;
    D_D(DArrowRectangle);

    d->setContent(content);
}

/*!
@~english
  @brief get content control.

  \return the content control being displayed
 */
QWidget *DArrowRectangle::getContent() const
{
    D_DC(DArrowRectangle);
    const auto& content = d->m_content;
    qCDebug(logWindowPlatform) << "Getting content widget:" << content;
    return content;
}

/*!
@~english
  @brief DArrowRectangle::resizeWithContent automatically adjust the rectangle's
  size to fit the its content.
 */
void DArrowRectangle::resizeWithContent()
{
    qCDebug(logWindowPlatform) << "Resizing DArrowRectangle with content";
    D_D(DArrowRectangle);

    d->resizeWithContent();
}

/*!
@~english
  @brief gets the size of the entire rectangular control.
  @brief DArrowRectangle::getFixedSize.

  \return the size of the whole widget.
 */
QSize DArrowRectangle::getFixedSize()
{
    D_D(DArrowRectangle);

    if (d->m_content) {
        qreal delta = (d->m_handle ? 0 : shadowBlurRadius() + d->m_shadowDistance) + margin();

        switch (d->m_arrowDirection) {
        case ArrowLeft:
        case ArrowRight:
            return QSize(d->m_content->width() + delta * 2 + d->m_arrowHeight - ((d->floatMode == FloatWidget) ? delta : 0),
                         d->m_content->height() + delta * 2);
        case ArrowTop:
        case ArrowBottom:
            return QSize(d->m_content->width() + delta * 2,
                         d->m_content->height() + delta * 2 + d->m_arrowHeight - ((d->floatMode == FloatWidget) ? delta : 0));
        }
    }

    return QSize(0, 0);
}

/*!
@~english
  @brief DArrowRectangle::move moves the widget to the coordinate that provided,
  
  Like the rules in DArrowRectangle::show(int x, int y), it moves the widget so
  that the arrow head's coordinate matches the one that provided.
  
  \a x is the x coordinate of the arrow head.
  \a y is the y coordinate of the arrow head.

  \sa DArrowRectangle::show(int x, int y)
  \sa DArrowRectangle::show
 */
void DArrowRectangle::move(int x, int y)
{
    qCDebug(logWindowPlatform) << "Moving DArrowRectangle to position:" << x << y;
    D_D(DArrowRectangle);

    switch (d->m_arrowDirection) {
    case ArrowLeft:
    case ArrowRight:
        qCDebug(logWindowPlatform) << "Using vertical move for left/right arrow";
        d->verticalMove(x, y);
        break;
    case ArrowTop:
    case ArrowBottom:
        qCDebug(logWindowPlatform) << "Using horizontal move for top/bottom arrow";
        d->horizontalMove(x, y);
        break;
    default:
        qCDebug(logWindowPlatform) << "Using default QWidget move";
        QWidget::move(x, y);
        break;
    }
    qCDebug(logWindowPlatform) << "Move completed";
}

// override methods
void DArrowRectangle::paintEvent(QPaintEvent *e)
{
    qCDebug(logWindowPlatform) << "Paint event received";
    D_D(DArrowRectangle);

    d->paintEvent(e);
    qCDebug(logWindowPlatform) << "Paint event handled";
}

void DArrowRectangle::resizeEvent(QResizeEvent *e)
{
    qCDebug(logWindowPlatform) << "Resize event received, new size:" << e->size();
    D_D(DArrowRectangle);

    d->resizeEvent(e);
    qCDebug(logWindowPlatform) << "Resize event handled";
}

bool DArrowRectangle::event(QEvent *e)
{
    qCDebug(logWindowPlatform) << "Event received, type:" << e->type();
    switch (e->type()) {
    case QEvent::WindowDeactivate:  
        qCDebug(logWindowPlatform) << "Window deactivate event, emitting signal";
        Q_EMIT windowDeactivate();    
        break;
    case QEvent::Polish: {
        qCDebug(logWindowPlatform) << "Polish event, checking radius";
        D_D(DArrowRectangle);
        if (d->m_radius < 0) {
            d->m_radius = DStyleHelper(style()).pixelMetric(DStyle::PM_TopLevelWindowRadius);
            qCDebug(logWindowPlatform) << "Updated radius to:" << d->m_radius;
        }
        break;
    }
    default:;
    }

    const auto& result = QWidget::event(e);
    qCDebug(logWindowPlatform) << "Event handled, result:" << result;
    return result;
}

const QRect DArrowRectanglePrivate::currentScreenRect(const int x, const int y)
{
    qCDebug(logWindowPlatform) << "Getting screen rect for position:" << x << y;
    if (floatMode == DArrowRectangle::FloatWidget) {
        qCDebug(logWindowPlatform) << "Using float widget mode";
        D_Q(DArrowRectangle);
        if (q->windowType() == Qt::Widget && q->parentWidget()) {
            const auto& rect = q->parentWidget()->rect();
            qCDebug(logWindowPlatform) << "Using parent widget rect:" << rect;
            return rect;
        }
    }

    qCDebug(logWindowPlatform) << "Searching for screen containing point";
    for (QScreen *screen : qApp->screens()) {
        if (screen->geometry().contains(x, y)) {
            const auto& rect = screen->geometry();
            qCDebug(logWindowPlatform) << "Found screen with geometry:" << rect;
            return rect;
        }
    }

    qCDebug(logWindowPlatform) << "No screen found containing point, returning empty rect";
    return QRect();
}

/*!
@~english
  @property DArrowRectangle::shadowYOffset

  @brief the offset of the widget and its shadow on y axis.

  Getter: DArrowRectangle::shadowYOffset Setter: DArrowRectangle::setShadowYOffset
  \sa DArrowRectangle::shadowXOffset
 */
qreal DArrowRectangle::shadowYOffset() const
{
    D_DC(DArrowRectangle);
    const auto& offset = d->m_shadowYOffset;
    qCDebug(logWindowPlatform) << "Getting shadow Y offset:" << offset;
    return offset;
}

void DArrowRectangle::setShadowYOffset(const qreal &shadowYOffset)
{
    qCDebug(logWindowPlatform) << "Setting shadow Y offset:" << shadowYOffset;
    D_D(DArrowRectangle);

    d->m_shadowYOffset = shadowYOffset;

    if (d->m_handle) {
        qCDebug(logWindowPlatform) << "Updating handle shadow offset";
        d->m_handle->setShadowOffset(QPoint(d->m_shadowXOffset, shadowYOffset));
    }
}

/*!
@~english
  @brief DArrowRectangle::setLeftRightRadius fillet when setting the left and right arrows.

  \a enable Whether to open.
 */
void DArrowRectangle::setLeftRightRadius(bool enable)
{
    qCDebug(logWindowPlatform) << "Setting left-right radius enabled:" << enable;
    D_D(DArrowRectangle);
    d->leftRightRadius = enable;
    qCDebug(logWindowPlatform) << "Left-right radius set successfully";
}

/*!
@~english
  @brief DArrowRectangle::setArrowStyleEnable Set Rounded Arrow Style.

  \a enable Whether to open.
 */
void DArrowRectangle::setRadiusArrowStyleEnable(bool enable)
{
    qCDebug(logWindowPlatform) << "Setting radius arrow style enabled:" << enable;
    D_D(DArrowRectangle);
    d->radiusArrowStyleEnable = enable;
    qCDebug(logWindowPlatform) << "Setting arrow dimensions for radius style - width:40 height:24";
    setArrowWidth(40);
    setArrowHeight(24);
    qCDebug(logWindowPlatform) << "Radius arrow style set successfully";
}

/*!
@~english
  @brief DArrowRectangle::setRadiusForceEnable Set fillet style.
  @brief By default, the window tube supports the fillet when the special effect is mixed, 
  and there is no fillet when there is no special effect. If it is enabled, there is always a fillet
  \a enable Whether to open.
 */
void DArrowRectangle::setRadiusForceEnable(bool enable)
{
    qCDebug(logWindowPlatform) << "Setting radius force enabled:" << enable;
    setProperty("_d_radius_force", enable);
    qCDebug(logWindowPlatform) << "Radius force property set successfully";
}
}

/*!
@~english
  @property DArrowRectangle::shadowXOffset

  @brief the offset of the widget and its shadow on x axis.
  
  Getter: DArrowRectangle::shadowXOffset Setter: DArrowRectangle::setShadowXOffset
  \sa DArrowRectangle::shadowYOffset
 */
qreal DArrowRectangle::shadowXOffset() const
{
    D_DC(DArrowRectangle);
    const auto& offset = d->m_shadowXOffset;
    qCDebug(logWindowPlatform) << "Getting shadow X offset:" << offset;
    return offset;
}

void DArrowRectangle::setShadowXOffset(const qreal &shadowXOffset)
{
    qCDebug(logWindowPlatform) << "Setting shadow X offset:" << shadowXOffset;
    D_D(DArrowRectangle);

    d->m_shadowXOffset = shadowXOffset;

    if (d->m_handle) {
        qCDebug(logWindowPlatform) << "Updating handle shadow offset";
        d->m_handle->setShadowOffset(QPoint(shadowXOffset, d->m_shadowYOffset));
    }
}

/*!
@~english
  @property DArrowRectangle::shadowBlurRadius

  @brief This property holds the blur radius of the widget's shadow.
  
  Getter: DArrowRectangle::shadowBlurRadius Setter: DArrowRectangle::setShadowBlurRadius
 */
qreal DArrowRectangle::shadowBlurRadius() const
{
    D_DC(DArrowRectangle);
    const auto& radius = d->m_shadowBlurRadius;
    qCDebug(logWindowPlatform) << "Getting shadow blur radius:" << radius;
    return radius;
}

void DArrowRectangle::setShadowBlurRadius(const qreal &shadowBlurRadius)
{
    qCDebug(logWindowPlatform) << "Setting shadow blur radius:" << shadowBlurRadius;
    D_D(DArrowRectangle);

    d->m_shadowBlurRadius = shadowBlurRadius;

    if (d->m_handle) {
        qCDebug(logWindowPlatform) << "Updating handle shadow radius";
        d->m_handle->setShadowRadius(shadowBlurRadius);
    }
}

/*!
@~english
  @property DArrowRectangle::borderColor

  @brief This property holds the border color of this widget.
  
  Getter: DArrowRectangle::borderColor , Setter: DArrowRectangle::setBorderColor
 */
QColor DArrowRectangle::borderColor() const
{
    D_DC(DArrowRectangle);
    const auto& color = d->m_borderColor;
    qCDebug(logWindowPlatform) << "Getting border color:" << color;
    return color;
}

void DArrowRectangle::setBorderColor(const QColor &borderColor)
{
    qCDebug(logWindowPlatform) << "Setting border color:" << borderColor;
    D_D(DArrowRectangle);

    d->m_borderColor = borderColor;

    if (d->m_handle) {
        qCDebug(logWindowPlatform) << "Updating handle border color";
        d->m_handle->setBorderColor(borderColor);
    }
}

/*!
@~english
  @property DArrowRectangle::borderWidth

  @brief This property holds the border width of this widget.
  
  Getter: DArrowRectangle::borderWidth , Setter: DArrowRectangle::setBorderWidth
 */
int DArrowRectangle::borderWidth() const
{
    D_DC(DArrowRectangle);
    const auto& width = d->m_borderWidth;
    qCDebug(logWindowPlatform) << "Getting border width:" << width;
    return width;
}

void DArrowRectangle::setBorderWidth(int borderWidth)
{
    qCDebug(logWindowPlatform) << "Setting border width:" << borderWidth;
    D_D(DArrowRectangle);

    d->m_borderWidth = borderWidth;

    if (d->m_handle) {
        qCDebug(logWindowPlatform) << "Updating handle border width";
        d->m_handle->setBorderWidth(borderWidth);
    }
}

/*!
@~english
  @property DArrowRectangle::backgroundColor

  @brief the background color of this rectangle.
  
  Getter: DArrowRectangle::backgroundColor , Setter: DArrowRectangle::setBackgroundColor
 */
QColor DArrowRectangle::backgroundColor() const
{
    D_DC(DArrowRectangle);
    const auto& color = d->m_backgroundColor;
    qCDebug(logWindowPlatform) << "Getting background color:" << color;
    return color;
}

/*!
@~english
  @property DArrowRectangle::arrowDirection

  @brief This property holds the direction of the rectangle's arrow points to.
  
  Getter: DArrowRectangle::arrowDirection , Setter: DArrowRectangle::setArrowDirection
 */
DArrowRectangle::ArrowDirection DArrowRectangle::arrowDirection() const
{
    D_DC(DArrowRectangle);
    const auto& direction = d->m_arrowDirection;
    qCDebug(logWindowPlatform) << "Getting arrow direction:" << static_cast<int>(direction);
    return direction;
}

void DArrowRectangle::setBackgroundColor(const QColor &backgroundColor)
{
    qCDebug(logWindowPlatform) << "Setting background color:" << backgroundColor;
    D_D(DArrowRectangle);

    d->m_backgroundColor = backgroundColor;

    bool isFloatWindow = d->floatMode == FloatWindow;
    if ((d->m_handle || (isFloatWindow && isDwayland())) && d->m_backgroundColor.toRgb().alpha() < 255) {
        qCDebug(logWindowPlatform) << "Background is semi-transparent, setting up blur effect";
        if (!d->m_blurBackground) {
            qCDebug(logWindowPlatform) << "Creating new blur background widget";
            d->m_blurBackground = new DBlurEffectWidget(this);
            d->m_blurBackground->setBlendMode(DBlurEffectWidget::BehindWindowBlend);
            d->m_blurBackground->resize(size());
            d->m_blurBackground->lower();
            d->m_blurBackground->show();
        }

        qCDebug(logWindowPlatform) << "Setting blur background mask color";
        d->m_blurBackground->setMaskColor(d->m_backgroundColor);
    } else {
        if (d->m_blurBackground) {
            qCDebug(logWindowPlatform) << "Removing blur background";
            d->m_blurBackground->hide();
            d->m_blurBackground->setParent(0);
            delete d->m_blurBackground;
            d->m_blurBackground = Q_NULLPTR;
        }
    }
}

/*!
@~english
  @brief DArrowRectangle::setBackgroundColor is an overloaded function.
  
  It sets the background color by modifing the mask color of the
  Dtk::Widget::DBlurEffectWidget.

  \a type is the mask color to set.
  \sa DArrowRectangle::backgroundColor DBlurEffectWidget::MaskColorType
 */
void DArrowRectangle::setBackgroundColor(DBlurEffectWidget::MaskColorType type)
{
    qCDebug(logWindowPlatform) << "Setting background color type:" << static_cast<int>(type);
    D_D(DArrowRectangle);

    if (d->m_blurBackground) {
        qCDebug(logWindowPlatform) << "Setting blur background mask color type";
        d->m_blurBackground->setMaskColor(type);
    } else {
        qCDebug(logWindowPlatform) << "No blur background exists, skipping";
    }
}

/*!
@~english
  @property DArrowRectangle::radius

  @brief radius of the rectangle
  
  Getter: DArrowRectangle::radius , Setter: DArrowRectangle::setRadius
 */
int DArrowRectangle::radius() const
{
    D_DC(DArrowRectangle);
    const auto& radius = d->m_radius;
    qCDebug(logWindowPlatform) << "Getting radius:" << radius;
    return radius;
}

/*!
@~english
  @property DArrowRectangle::radiusForceEnabled

  @brief Whether to force (Ignore special effects) open the fillet

  Getter: DArrowRectangle::radiusForceEnabled , Setter: DArrowRectangle::setRadiusForceEnable
 */
bool DArrowRectangle::radiusForceEnabled() const
{
    return property("_d_radius_force").toBool();
}

/*!
@~english
  @property DArrowRectangle::arrowHeight

  @brief height of rectangle's arrow
  
  Getter: DArrowRectangle::arrowHeight , Setter: DArrowRectangle::setArrowHeight
  \sa DArrowRectangle::arrowWidth
 */
int DArrowRectangle::arrowHeight() const
{
    D_DC(DArrowRectangle);
    const auto& height = d->m_arrowHeight;
    qCDebug(logWindowPlatform) << "Getting arrow height:" << height;
    return height;
}

/*!
@~english
  @property DArrowRectangle::arrowWidth

  @brief width of the rectangle's arrow
  
  Getter: DArrowRectangle::arrowWidth , Setter: DArrowRectangle::setArrowWidth
  \sa DArrowRectangle::arrowHeight
 */
int DArrowRectangle::arrowWidth() const
{
    D_DC(DArrowRectangle);
    const auto& width = d->m_arrowWidth;
    qCDebug(logWindowPlatform) << "Getting arrow width:" << width;
    return width;
}

/*!
@~english
  @property DArrowRectangle::arrowX

  @brief the x coordinate of the rectangle's arrow
  
  Getter: DArrowRectangle::arrowX , Setter: DArrowRectangle::setArrowX
  \sa DArrowRectangle::arrowY
 */
int DArrowRectangle::arrowX() const
{
    D_DC(DArrowRectangle);
    const auto& x = d->m_arrowX;
    qCDebug(logWindowPlatform) << "Getting arrow X coordinate:" << x;
    return x;
}

/*!
@~english
  @property DArrowRectangle::arrowY

  @brief the y coordinate of the rectangle's arrow
  
  Getter: DArrowRectangle::arrowY , Setter: DArrowRectangle::setArrowY
  \sa DArrowRectangle::arrowX
 */
int DArrowRectangle::arrowY() const
{
    D_DC(DArrowRectangle);
    const auto& y = d->m_arrowY;
    qCDebug(logWindowPlatform) << "Getting arrow Y coordinate:" << y;
    return y;
}

/*!
@~english
  @property DArrowRectangle::margin

  @brief This property holds the width of the margin
  
  The margin is the distance between the innermost pixel of the rectangle and the
  outermost pixel of its contents.
  The default margin is 0.
  Getter: DArrowRectangle::margin , Setter: DArrowRectangle::setMargin

  \sa DArrowRectangle::setMargin
 */
int DArrowRectangle::margin() const
{
    D_DC(DArrowRectangle);
    const auto& margin = d->m_margin;
    qCDebug(logWindowPlatform) << "Getting margin:" << margin;
    return margin;
}

/*!
@~english
  @brief this function is used to set the arrow direction.

  \a value arrow direction.

  \sa DArrowRectangle::arrowDirection
 */
void DArrowRectangle::setArrowDirection(ArrowDirection value)
{
    qCDebug(logWindowPlatform) << "Setting arrow direction:" << static_cast<int>(value);
    D_D(DArrowRectangle);

    d->m_arrowDirection = value;
}

/*!
@~english
  @brief This function is used to set the fixed width of the entire control
  
  \a value width Size
 */
void DArrowRectangle::setWidth(int value)
{
    qCDebug(logWindowPlatform) << "Setting width:" << value;
    setFixedWidth(value);
}

/*!
@~english
  @brief set the fixed height of the entire control
  
  \a value height Size
 */
void DArrowRectangle::setHeight(int value)
{
    qCDebug(logWindowPlatform) << "Setting height:" << value;
    setFixedHeight(value);
}

/*!
@~english
  @brief this function is used to set the fillet size.

  \a value fillet size.

  \sa DArrowRectangle::radius
 */
void DArrowRectangle::setRadius(int value)
{
    qCDebug(logWindowPlatform) << "Setting radius:" << value;
    D_D(DArrowRectangle);

    d->m_radius = value;
}

/*!
@~english
  @brief set arrow height.

  \a value arrow Height.

  \sa DArrowRectangle::arrowHeight
 */
void DArrowRectangle::setArrowHeight(int value)
{
    qCDebug(logWindowPlatform) << "Setting arrow height:" << value;
    D_D(DArrowRectangle);

    d->m_arrowHeight = value;
}

/*!
@~english
  @brief set arrow width.

  \a value arrow width.

  \sa DArrowRectangle::arrowWidth
 */
void DArrowRectangle::setArrowWidth(int value)
{
    qCDebug(logWindowPlatform) << "Setting arrow width:" << value;
    D_D(DArrowRectangle);

    d->m_arrowWidth = value;
}

/*!
@~english
  @brief set the value of the arrow x coordinate.

  \a value x value of coordinates.

  \sa DArrowRectangle::arrowX
 */
void DArrowRectangle::setArrowX(int value)
{
    qCDebug(logWindowPlatform) << "Setting arrow X:" << value;
    D_D(DArrowRectangle);

    d->m_arrowX = value;
}

/*!
@~english
  @brief set the value of arrow y coordinate.

  \a value y value of coordinates.

  \sa DArrowRectangle::arrowY
 */
void DArrowRectangle::setArrowY(int value)
{
    qCDebug(logWindowPlatform) << "Setting arrow Y:" << value;
    D_D(DArrowRectangle);

    d->m_arrowY = value;
    qCDebug(logWindowPlatform) << "Arrow Y set successfully";
}

/*!
@~english
  @brief set margin size.

  \a value margin size.

  \sa DArrowRectangle::margin
 */
void DArrowRectangle::setMargin(int value)
{
    qCDebug(logWindowPlatform) << "Setting margin:" << value;
    D_D(DArrowRectangle);

    d->m_margin = value;
}

QPainterPath DArrowRectanglePrivate::getLeftCornerPath()
{
    qCDebug(logWindowPlatform) << "Getting left corner path";
    D_Q(DArrowRectangle);

    QRect rect = q->rect();
    qCDebug(logWindowPlatform) << "Initial rect:" << rect;

    if (!m_handle) {
        qCDebug(logWindowPlatform) << "No handle, calculating margins";
        qreal delta = q->shadowBlurRadius() + m_shadowDistance;
        qCDebug(logWindowPlatform) << "Delta for margins:" << delta;

        if (DApplication::isDXcbPlatform()) {
            qCDebug(logWindowPlatform) << "Using DXcb platform margins";
            rect = rect.marginsRemoved(QMargins((DArrowRectangle::FloatWidget == floatMode) ? 0 : delta, delta, delta - q->margin(), delta));
        } else {
            qCDebug(logWindowPlatform) << "Using non-DXcb platform margins";
            QMargins margins((DArrowRectangle::FloatWidget == floatMode) ? 0 : 0, delta,
                             (DArrowRectangle::FloatWidget == floatMode) ? delta - q->margin() : delta *2, delta);
            if (rect.width() > (margins.left() + margins.right()) &&
                rect.height() > (margins.top() + margins.bottom())) {
                qCDebug(logWindowPlatform) << "Rect is large enough, removing margins";
                rect = rect.marginsRemoved(margins);
            } else {
                qCDebug(logWindowPlatform) << "Rect too small for margins";
            }
        }
    }

    QPoint cornerPoint(rect.x(), rect.y() + (m_arrowY > 0 ? m_arrowY : (rect.height() / 2)));
    QPoint topLeft(rect.x() + m_arrowHeight, rect.y());
    QPoint topRight(rect.x() + rect.width(), rect.y());
    QPoint bottomRight(rect.x() + rect.width(), rect.y() + rect.height());
    QPoint bottomLeft(rect.x() + m_arrowHeight, rect.y() + rect.height());
    qCDebug(logWindowPlatform) << "Corner points - corner:" << cornerPoint << "topLeft:" << topLeft 
                              << "topRight:" << topRight << "bottomRight:" << bottomRight 
                              << "bottomLeft:" << bottomLeft;

    int radius = m_radius > (rect.height() / 2) ? (rect.height() / 2) : m_radius;
    int arrowWidth = m_arrowWidth - m_arrowHeight;
    int widgetRadius = rect.height() > 4 * radius ? 2 * radius : radius;
    qCDebug(logWindowPlatform) << "Calculated dimensions - radius:" << radius 
                              << "arrowWidth:" << arrowWidth << "widgetRadius:" << widgetRadius;

    if (!radiusEnabled()) {
        qCDebug(logWindowPlatform) << "Radius disabled, setting to 0";
        radius = 0;
    }

    QPainterPath border;
    qCDebug(logWindowPlatform) << "Creating border path";
    border.moveTo(topLeft.x() + arrowWidth, topLeft.y());
    qCDebug(logWindowPlatform) << "Drawing top edge";
    border.lineTo(topRight.x() - radius, topRight.y());
    qCDebug(logWindowPlatform) << "Drawing top-right corner arc";
    border.arcTo(topRight.x() - 2 * radius, topRight.y(), 2 * radius, 2 * radius, 90, -90);
    qCDebug(logWindowPlatform) << "Drawing right edge";
    border.lineTo(bottomRight.x(), bottomRight.y() - radius);
    qCDebug(logWindowPlatform) << "Drawing bottom-right corner arc";
    border.arcTo(bottomRight.x() - 2 * radius, bottomRight.y() - 2 * radius, 2 * radius, 2 * radius, 0, -90);

    if (radiusArrowStyleEnable) {
        qCDebug(logWindowPlatform) << "Using radius arrow style";
        border.lineTo(bottomLeft.x() + radius, bottomLeft.y());
        qCDebug(logWindowPlatform) << "Drawing bottom-left corner arc";
        border.arcTo(bottomLeft.x(), bottomLeft.y() - widgetRadius, widgetRadius, widgetRadius, -90, -90);
        qCDebug(logWindowPlatform) << "Drawing path to arrow start";
        border.lineTo(bottomLeft.x(), cornerPoint.y() + m_arrowWidth / 2 + radius / 2);

        qCDebug(logWindowPlatform) << "Drawing first cubic curve for arrow";
        border.cubicTo(QPointF(bottomLeft.x(), cornerPoint.y() + m_arrowWidth / 2 + radius / 2),
                       QPointF(bottomLeft.x(), cornerPoint.y() + m_arrowWidth / 2),
                       QPointF(cornerPoint.x() + m_arrowHeight / 2, cornerPoint.y() + m_arrowWidth / 4.5));

        qCDebug(logWindowPlatform) << "Drawing second cubic curve for arrow";
        border.cubicTo(QPointF(cornerPoint.x() + m_arrowHeight / 2, cornerPoint.y() + m_arrowWidth / 4.5),
                       QPointF(cornerPoint),
                       QPointF(cornerPoint.x() + m_arrowHeight / 2, cornerPoint.y() - m_arrowWidth / 4.5));

        qCDebug(logWindowPlatform) << "Drawing third cubic curve for arrow";
        border.cubicTo(QPointF(cornerPoint.x() + m_arrowHeight / 2, cornerPoint.y() - m_arrowWidth / 4.5),
                       QPointF(cornerPoint.x() + m_arrowHeight, cornerPoint.y() - m_arrowWidth / 2),
                       QPointF(topLeft.x(), cornerPoint.y() - m_arrowWidth / 2 - radius / 2));

        qCDebug(logWindowPlatform) << "Drawing path to top-left corner";
        border.lineTo(topLeft.x(), cornerPoint.y() - m_arrowWidth / 2 - radius / 2);
        qCDebug(logWindowPlatform) << "Drawing top-left corner arc";
        border.arcTo(topLeft.x(), topLeft.y(), widgetRadius, widgetRadius, -180, -90);

        qCDebug(logWindowPlatform) << "Completing border path";
        border.lineTo(topLeft.x() + radius, topLeft.y());
    } else {
        qCDebug(logWindowPlatform) << "Using standard arrow style";
        if (leftRightRadius) {
            qCDebug(logWindowPlatform) << "Drawing with left-right radius";
            border.lineTo(bottomLeft.x() + radius, bottomLeft.y());
            qCDebug(logWindowPlatform) << "Drawing bottom-left corner arc";
            border.arcTo(bottomLeft.x(), bottomLeft.y() - widgetRadius, widgetRadius, widgetRadius, -90, -90);
            qCDebug(logWindowPlatform) << "Drawing path to arrow start";
            border.lineTo(bottomLeft.x(), cornerPoint.y() + m_arrowWidth / 2);
        } else {
            qCDebug(logWindowPlatform) << "Drawing without left-right radius";
            border.lineTo(bottomLeft.x(), bottomLeft.y());

            if (cornerPoint.y() > m_arrowWidth) {
                qCDebug(logWindowPlatform) << "Drawing extended bottom path";
                border.lineTo(bottomLeft.x(), cornerPoint.y() + m_arrowWidth / 2);
            }
        }

        qCDebug(logWindowPlatform) << "Drawing arrow point";
        border.lineTo(cornerPoint);

        if (leftRightRadius) {
            qCDebug(logWindowPlatform) << "Drawing top path with radius";
            border.lineTo(topLeft.x(), cornerPoint.y() - m_arrowWidth / 2);
            border.lineTo(topLeft.x(), topLeft.y() + radius);
            qCDebug(logWindowPlatform) << "Drawing top-left corner arc";
            border.arcTo(topLeft.x(), topLeft.y(), widgetRadius, widgetRadius, -180, -90);
        } else {
            if (cornerPoint.y() > m_arrowWidth) {
                qCDebug(logWindowPlatform) << "Drawing extended top path";
                border.lineTo(topLeft.x(), cornerPoint.y() - m_arrowWidth / 2);
            }

            qCDebug(logWindowPlatform) << "Drawing final top edge";
            border.lineTo(topLeft.x(), topLeft.y());
        }
    }

    return border;
}

QPainterPath DArrowRectanglePrivate::getRightCornerPath()
{
    qCDebug(logWindowPlatform) << "Getting right corner path";
    D_Q(DArrowRectangle);

    QRect rect = q->rect();
    qCDebug(logWindowPlatform) << "Initial rect:" << rect;

    if (!m_handle) {
        qCDebug(logWindowPlatform) << "No handle, calculating margins";
        qreal delta = q->shadowBlurRadius() + m_shadowDistance;
        qCDebug(logWindowPlatform) << "Delta for margins:" << delta;

        if (DApplication::isDXcbPlatform()) {
            qCDebug(logWindowPlatform) << "Using DXcb platform margins";
            rect = rect.marginsRemoved(QMargins((DArrowRectangle::FloatWidget == floatMode) ? delta - q->margin() : delta, delta, (DArrowRectangle::FloatWidget == floatMode) ? 0 : delta, delta));
        } else {
            qCDebug(logWindowPlatform) << "Using non-DXcb platform margins";
            QMargins margins((DArrowRectangle::FloatWidget == floatMode) ? delta -q->margin() : delta *2, delta, (DArrowRectangle::FloatWidget == floatMode) ? 0 : 0, delta);
            if (rect.width() > (margins.left() + margins.right()) &&
                rect.height() > (margins.top() + margins.bottom())) {
                qCDebug(logWindowPlatform) << "Rect is large enough, removing margins";
                rect = rect.marginsRemoved(margins);
            } else {
                qCDebug(logWindowPlatform) << "Rect too small for margins";
            }
        }
    }

    QPoint cornerPoint(rect.x() + rect.width(), rect.y() + (m_arrowY > 0 ? m_arrowY : rect.height() / 2));
    QPoint topLeft(rect.x(), rect.y());
    QPoint topRight(rect.x() + rect.width() - m_arrowHeight, rect.y());
    QPoint bottomRight(rect.x() + rect.width() - m_arrowHeight, rect.y() + rect.height());
    QPoint bottomLeft(rect.x(), rect.y() + rect.height());
    qCDebug(logWindowPlatform) << "Corner points - corner:" << cornerPoint << "topLeft:" << topLeft 
                              << "topRight:" << topRight << "bottomRight:" << bottomRight 
                              << "bottomLeft:" << bottomLeft;

    int radius = this->m_radius > (rect.height() / 2) ? rect.height() / 2 : this->m_radius;
    int widgetRadius = rect.height() >= 4 * radius ? 2 * radius : radius;
    qCDebug(logWindowPlatform) << "Calculated dimensions - radius:" << radius 
                              << "widgetRadius:" << widgetRadius;

    if (!radiusEnabled()) {
        qCDebug(logWindowPlatform) << "Radius disabled, setting to 0";
        radius = 0;
    }

    QPainterPath border;
    qCDebug(logWindowPlatform) << "Creating border path";
    border.moveTo(topLeft.x(), topLeft.y());

    if (radiusArrowStyleEnable) {
        qCDebug(logWindowPlatform) << "Using radius arrow style";
        qCDebug(logWindowPlatform) << "Drawing top edge";
        border.lineTo(topRight.x() - radius, topRight.y());
        qCDebug(logWindowPlatform) << "Drawing top-right corner arc";
        border.arcTo(topRight.x() - widgetRadius, topRight.y(), widgetRadius, widgetRadius, 90, -90);
        qCDebug(logWindowPlatform) << "Drawing path to arrow start";
        border.lineTo(topRight.x(), cornerPoint.y() - m_arrowWidth / 2 - radius / 2);

        qCDebug(logWindowPlatform) << "Drawing first cubic curve for arrow";
        border.cubicTo(QPointF(topRight.x(), cornerPoint.y() - m_arrowWidth / 2 - radius / 2),
                       QPointF(topRight.x(), cornerPoint.y() - m_arrowWidth / 2),
                       QPointF(cornerPoint.x() - m_arrowHeight / 2, cornerPoint.y() - m_arrowWidth / 4.5));

        qCDebug(logWindowPlatform) << "Drawing second cubic curve for arrow";
        border.cubicTo(QPointF(cornerPoint.x() - m_arrowHeight / 2, cornerPoint.y() - m_arrowWidth / 4.5),
                       QPointF(cornerPoint),
                       QPointF(cornerPoint.x() - m_arrowHeight / 2, cornerPoint.y() + m_arrowWidth / 4.5));

        qCDebug(logWindowPlatform) << "Drawing third cubic curve for arrow";
        border.cubicTo(QPointF(cornerPoint.x() - m_arrowHeight / 2, cornerPoint.y() + m_arrowWidth / 4.5),
                       QPointF(bottomRight.x(), cornerPoint.y() + m_arrowWidth / 2),
                       QPointF(bottomRight.x(), cornerPoint.y() + m_arrowWidth / 2 + radius / 2));

        qCDebug(logWindowPlatform) << "Drawing path to bottom-right corner";
        border.lineTo(bottomRight.x(), cornerPoint.y() + m_arrowWidth / 2 + radius / 2);
        qCDebug(logWindowPlatform) << "Drawing bottom-right corner arc";
        border.arcTo(bottomRight.x() - widgetRadius, bottomRight.y() - widgetRadius, widgetRadius, widgetRadius, 0, -90);

        qCDebug(logWindowPlatform) << "Drawing bottom edge";
        border.lineTo(bottomRight.x() - radius, bottomRight.y());
    } else {
        if (leftRightRadius) {
            border.lineTo(topRight.x() - radius, topRight.y());
            border.arcTo(topRight.x() - widgetRadius, topRight.y(), widgetRadius, widgetRadius, 90, -90);
            border.lineTo(topRight.x(), cornerPoint.y() - m_arrowWidth / 2);
        } else {
            border.lineTo(topRight.x(), topRight.y());

            if (cornerPoint.y() > m_arrowWidth)
                border.lineTo(topRight.x(), cornerPoint.y() - m_arrowWidth / 2);
        }

        border.lineTo(cornerPoint);

        if (leftRightRadius) {
            border.lineTo(bottomRight.x(), cornerPoint.y() + m_arrowWidth / 2);
            border.lineTo(bottomRight.x(), bottomRight.y() - radius);
            border.arcTo(bottomRight.x() - widgetRadius, bottomRight.y() - widgetRadius, widgetRadius, widgetRadius, 0, -90);

        } else if (!leftRightRadius){
            if (cornerPoint.y() > m_arrowWidth)
                border.lineTo(bottomRight.x(), cornerPoint.y() + m_arrowWidth / 2);

            border.lineTo(bottomRight.x(), bottomRight.y());
        }
    }

    border.lineTo(bottomLeft.x() + radius, bottomLeft.y());
    border.arcTo(bottomLeft.x(), bottomLeft.y() - 2 * radius, 2 * radius, 2 * radius, -90, -90);
    border.lineTo(topLeft.x(), topLeft.y() + radius);
    border.arcTo(topLeft.x(), topLeft.y(), 2 * radius, 2 * radius, 180, -90);

    return border;
}

QPainterPath DArrowRectanglePrivate::getTopCornerPath()
{
    qCDebug(logWindowPlatform) << "Getting top corner path";
    D_Q(DArrowRectangle);

    QRect rect = q->rect();
    qCDebug(logWindowPlatform) << "Initial rect:" << rect;

    if (!m_handle) {
        qCDebug(logWindowPlatform) << "No handle, calculating margins";
        qreal delta = q->shadowBlurRadius() + m_shadowDistance;
        qCDebug(logWindowPlatform) << "Delta for margins:" << delta;

        if (DApplication::isDXcbPlatform()) {
            qCDebug(logWindowPlatform) << "Using DXcb platform margins";
            rect = rect.marginsRemoved(QMargins(delta, (DArrowRectangle::FloatWidget == floatMode) ? 0 : delta,
                                                delta, (DArrowRectangle::FloatWidget == floatMode) ? delta -q->margin() : delta));
        } else {
            qCDebug(logWindowPlatform) << "Using non-DXcb platform margins";
            QMargins margins(delta, 0, delta, (DArrowRectangle::FloatWidget == floatMode) ? delta - q->margin() : delta * 2);
            if (rect.width() > (margins.left() + margins.right()) &&
                rect.height() > (margins.top() + margins.bottom())) {
                qCDebug(logWindowPlatform) << "Rect is large enough, removing margins";
                rect = rect.marginsRemoved(margins);
            } else {
                qCDebug(logWindowPlatform) << "Rect too small for margins";
            }
        }
    }

    QPoint cornerPoint(rect.x() + (m_arrowX > 0 ? m_arrowX : rect.width() / 2), rect.y());
    QPoint topLeft(rect.x(), rect.y() + m_arrowHeight);
    QPoint topRight(rect.x() + rect.width(), rect.y() + m_arrowHeight);
    QPoint bottomRight(rect.x() + rect.width(), rect.y() + rect.height());
    QPoint bottomLeft(rect.x(), rect.y() + rect.height());
    qCDebug(logWindowPlatform) << "Corner points - corner:" << cornerPoint << "topLeft:" << topLeft 
                              << "topRight:" << topRight << "bottomRight:" << bottomRight 
                              << "bottomLeft:" << bottomLeft;

    int radius = this->m_radius > (rect.height() / 2 - m_arrowHeight) ? rect.height() / 2 - m_arrowHeight : this->m_radius;
    qCDebug(logWindowPlatform) << "Calculated radius:" << radius;

    if (!radiusEnabled()) {
        qCDebug(logWindowPlatform) << "Radius disabled, setting to 0";
        radius = 0;
    }

    QPainterPath border;
    qCDebug(logWindowPlatform) << "Creating border path";
    border.moveTo(topLeft.x() + radius, topLeft.y());

    if (radiusArrowStyleEnable) {
        qCDebug(logWindowPlatform) << "Using radius arrow style";
        qCDebug(logWindowPlatform) << "Drawing path to arrow start";
        border.lineTo(cornerPoint.x() - m_arrowWidth / 2 - radius / 2, cornerPoint.y() + m_arrowHeight);

        qCDebug(logWindowPlatform) << "Drawing first cubic curve for arrow";
        border.cubicTo(QPointF(cornerPoint.x() - m_arrowWidth / 2 - radius / 2, topLeft.y()),
                       QPointF(cornerPoint.x() - m_arrowWidth / 2, topLeft.y()),
                       QPointF(cornerPoint.x() - m_arrowWidth / 4.5, cornerPoint.y() + m_arrowHeight / 2));

        qCDebug(logWindowPlatform) << "Drawing second cubic curve for arrow";
        border.cubicTo(QPointF(cornerPoint.x() - m_arrowWidth / 4.5, cornerPoint.y() + m_arrowHeight / 2),
                       QPointF(cornerPoint),
                       QPointF(cornerPoint.x() + m_arrowWidth / 4.5, cornerPoint.y() + m_arrowHeight / 2));

        qCDebug(logWindowPlatform) << "Drawing third cubic curve for arrow";
        border.cubicTo(QPointF(cornerPoint.x() + m_arrowWidth / 4.5, cornerPoint.y() + m_arrowHeight / 2),
                       QPointF(cornerPoint.x() + m_arrowWidth / 2, topLeft.y()),
                       QPointF(cornerPoint.x() + m_arrowWidth / 2 + radius / 2, cornerPoint.y() + m_arrowHeight));
        qCDebug(logWindowPlatform) << "Drawing path to arrow end";
        border.lineTo(cornerPoint.x() + m_arrowWidth / 2 + radius / 2, cornerPoint.y() + m_arrowHeight);
    } else {
        qCDebug(logWindowPlatform) << "Using standard arrow style";
        qCDebug(logWindowPlatform) << "Drawing left side of arrow";
        border.lineTo(cornerPoint.x() - m_arrowWidth / 2, cornerPoint.y() + m_arrowHeight);
        qCDebug(logWindowPlatform) << "Drawing arrow point";
        border.lineTo(cornerPoint);
        qCDebug(logWindowPlatform) << "Drawing right side of arrow";
        border.lineTo(cornerPoint.x() + m_arrowWidth / 2, cornerPoint.y() + m_arrowHeight);
    }

    qCDebug(logWindowPlatform) << "Drawing top edge";
    border.lineTo(topRight.x() - radius, topRight.y());
    qCDebug(logWindowPlatform) << "Drawing top-right corner arc";
    border.arcTo(topRight.x() - 2 * radius, topRight.y(), 2 * radius, 2 * radius, 90, -90);
    qCDebug(logWindowPlatform) << "Drawing right edge";
    border.lineTo(bottomRight.x(), bottomRight.y() - radius);
    qCDebug(logWindowPlatform) << "Drawing bottom-right corner arc";
    border.arcTo(bottomRight.x() - 2 * radius, bottomRight.y() - 2 * radius, 2 * radius, 2 * radius, 0, -90);
    qCDebug(logWindowPlatform) << "Drawing bottom edge";
    border.lineTo(bottomLeft.x() + radius, bottomLeft.y());
    qCDebug(logWindowPlatform) << "Drawing bottom-left corner arc";
    border.arcTo(bottomLeft.x(), bottomLeft.y() - 2 * radius, 2 * radius, 2 * radius, - 90, -90);
    qCDebug(logWindowPlatform) << "Drawing left edge";
    border.lineTo(topLeft.x(), topLeft.y() + radius);
    qCDebug(logWindowPlatform) << "Drawing top-left corner arc";
    border.arcTo(topLeft.x(), topLeft.y(), 2 * radius, 2 * radius, 180, -90);

    return border;
}

QPainterPath DArrowRectanglePrivate::getBottomCornerPath()
{
    qCDebug(logWindowPlatform) << "Getting bottom corner path";
    D_QC(DArrowRectangle);

    QRect rect = q->rect();
    qCDebug(logWindowPlatform) << "Initial rect:" << rect;

    if (!m_handle) {
        qCDebug(logWindowPlatform) << "No handle, calculating margins";
        qreal delta = q->shadowBlurRadius() + m_shadowDistance;
        qCDebug(logWindowPlatform) << "Delta for margins:" << delta;

        if (DApplication::isDXcbPlatform()) {
            qCDebug(logWindowPlatform) << "Using DXcb platform margins";
            rect = rect.marginsRemoved(QMargins(delta, (DArrowRectangle::FloatWidget == floatMode) ? delta -q->margin() : delta, delta, (DArrowRectangle::FloatWidget == floatMode) ? 0 : delta));
        } else {
            qCDebug(logWindowPlatform) << "Using non-DXcb platform margins";
            QMargins margins(delta, (DArrowRectangle::FloatWidget == floatMode) ? delta - q->margin() : delta *2, delta, 0);
            if (rect.width() > (margins.left() + margins.right()) &&
                rect.height() > (margins.top() + margins.bottom())) {
                qCDebug(logWindowPlatform) << "Rect is large enough, removing margins";
                rect = rect.marginsRemoved(margins);
            } else {
                qCDebug(logWindowPlatform) << "Rect too small for margins";
            }
        }
    }

    QPoint cornerPoint(rect.x() + (m_arrowX > 0 ? m_arrowX : qRound(double(rect.width()) / 2)), rect.y() + rect.height());
    QPoint topLeft(rect.x(), rect.y());
    QPoint topRight(rect.x() + rect.width(), rect.y());
    QPoint bottomRight(rect.x() + rect.width(), rect.y() + rect.height() - m_arrowHeight);
    QPoint bottomLeft(rect.x(), rect.y() + rect.height() - m_arrowHeight);
    qCDebug(logWindowPlatform) << "Corner points - corner:" << cornerPoint << "topLeft:" << topLeft 
                              << "topRight:" << topRight << "bottomRight:" << bottomRight 
                              << "bottomLeft:" << bottomLeft;

    int radius = this->m_radius > (rect.height() / 2 - m_arrowHeight) ? rect.height() / 2 - m_arrowHeight : this->m_radius;
    qCDebug(logWindowPlatform) << "Calculated radius:" << radius;

    if (!radiusEnabled()) {
        qCDebug(logWindowPlatform) << "Radius disabled, setting to 0";
        radius = 0;
    }

    QPainterPath border;
    qCDebug(logWindowPlatform) << "Creating border path";
    border.moveTo(topLeft.x() + radius, topLeft.y());
    qCDebug(logWindowPlatform) << "Drawing top edge";
    border.lineTo(topRight.x() - radius, topRight.y());
    qCDebug(logWindowPlatform) << "Drawing top-right corner arc";
    border.arcTo(topRight.x() - 2 * radius, topRight.y(), 2 * radius, 2 * radius, 90, -90);
    qCDebug(logWindowPlatform) << "Drawing right edge";
    border.lineTo(bottomRight.x(), bottomRight.y() - radius);
    qCDebug(logWindowPlatform) << "Drawing bottom-right corner arc";
    border.arcTo(bottomRight.x() - 2 * radius, bottomRight.y() - 2 * radius, 2 * radius, 2 * radius, 0, -90);

    if (radiusArrowStyleEnable) {
        qCDebug(logWindowPlatform) << "Using radius arrow style";
        qCDebug(logWindowPlatform) << "Drawing path to arrow start";
        border.lineTo(cornerPoint.x() + m_arrowWidth / 2 + radius / 2, cornerPoint.y() - m_arrowHeight);

        qCDebug(logWindowPlatform) << "Drawing first cubic curve for arrow";
        border.cubicTo(QPointF(cornerPoint.x() + m_arrowWidth / 2 + radius / 2, cornerPoint.y() - m_arrowHeight),
                       QPointF(cornerPoint.x() + m_arrowWidth / 2, bottomRight.y()),
                       QPointF(cornerPoint.x() + m_arrowWidth / 4.5, cornerPoint.y() - m_arrowHeight / 2));

        qCDebug(logWindowPlatform) << "Drawing second cubic curve for arrow";
        border.cubicTo(QPointF(cornerPoint.x() + m_arrowWidth / 4.5 , cornerPoint.y() - m_arrowHeight / 2),
                       QPointF(cornerPoint),
                       QPointF(cornerPoint.x() - m_arrowWidth / 4.5 , cornerPoint.y() - m_arrowHeight / 2));

        qCDebug(logWindowPlatform) << "Drawing third cubic curve for arrow";
        border.cubicTo(QPointF(cornerPoint.x() - m_arrowWidth / 4.5 , cornerPoint.y() - m_arrowHeight / 2),
                       QPointF(cornerPoint.x() - m_arrowWidth / 2, bottomLeft.y()),
                       QPointF(cornerPoint.x() - m_arrowWidth / 2 - radius / 2, cornerPoint.y() - m_arrowHeight));

        qCDebug(logWindowPlatform) << "Drawing path to arrow end";
        border.lineTo(cornerPoint.x() - m_arrowWidth / 2 - radius / 2, cornerPoint.y() - m_arrowHeight);
    } else {
        qCDebug(logWindowPlatform) << "Using standard arrow style";
        qCDebug(logWindowPlatform) << "Drawing right side of arrow";
        border.lineTo(cornerPoint.x() + m_arrowWidth / 2, cornerPoint.y() - m_arrowHeight);
        qCDebug(logWindowPlatform) << "Drawing arrow point";
        border.lineTo(cornerPoint);
        qCDebug(logWindowPlatform) << "Drawing left side of arrow";
        border.lineTo(cornerPoint.x() - m_arrowWidth / 2, cornerPoint.y() - m_arrowHeight);
    }

    qCDebug(logWindowPlatform) << "Drawing bottom edge";
    border.lineTo(bottomLeft.x() + radius, bottomLeft.y());
    qCDebug(logWindowPlatform) << "Drawing bottom-left corner arc";
    border.arcTo(bottomLeft.x(), bottomLeft.y() - 2 * radius, 2 * radius, 2 * radius, -90, -90);
    qCDebug(logWindowPlatform) << "Drawing left edge";
    border.lineTo(topLeft.x(), topLeft.y() + radius);
    qCDebug(logWindowPlatform) << "Drawing top-left corner arc";
    border.arcTo(topLeft.x(), topLeft.y(), 2 * radius, 2 * radius, 180, -90);

    return border;
}

void DArrowRectanglePrivate::verticalMove(int x, int y)
{
    qCDebug(logWindowPlatform) << "Vertical move to position:" << x << y;
    D_Q(DArrowRectangle);

    const QRect dRect = currentScreenRect(x, y);
    qreal delta = m_handle ? 0 : (q->shadowBlurRadius() - m_shadowDistance);
    qCDebug(logWindowPlatform) << "Screen rect:" << dRect << "delta:" << delta;

    int lRelativeY = y - dRect.y() - (q->height() - delta) / 2;
    int rRelativeY = y - dRect.y() + (q->height() - delta) / 2 - dRect.height();
    int absoluteY = 0;
    qCDebug(logWindowPlatform) << "Relative positions - left:" << lRelativeY << "right:" << rRelativeY;

    if (lRelativeY < 0) { //out of screen in top side
        qCDebug(logWindowPlatform) << "Out of screen in top side";
        //arrowY use relative coordinates
        q->setArrowY(q->height() / 2 - delta + lRelativeY);
        absoluteY = dRect.y() - delta;
    } else if (rRelativeY > 0) { //out of screen in bottom side
        qCDebug(logWindowPlatform) << "Out of screen in bottom side";
        q->setArrowY(q->height() / 2 - delta / 2 + rRelativeY);
        absoluteY = dRect.y() + dRect.height() - q->height() + delta;
    } else {
        qCDebug(logWindowPlatform) << "Within screen bounds";
        q->setArrowY(0);
        absoluteY = y - q->height() / 2;
    }

    qCDebug(logWindowPlatform) << "Final absolute Y position:" << absoluteY;
    switch (m_arrowDirection) {
    case DArrowRectangle::ArrowLeft:
        qCDebug(logWindowPlatform) << "Moving with left arrow to:" << x << absoluteY;
        q->QWidget::move(x, absoluteY);
        break;
    case DArrowRectangle::ArrowRight:
        qCDebug(logWindowPlatform) << "Moving with right arrow to:" << (x - q->width()) << absoluteY;
        q->QWidget::move(x - q->width(), absoluteY);
        break;
    default:
        qCDebug(logWindowPlatform) << "Ignoring move for invalid arrow direction";
        break;
    }
    qCDebug(logWindowPlatform) << "Vertical move completed";
}

void DArrowRectanglePrivate::horizontalMove(int x, int y)
{
    qCDebug(logWindowPlatform) << "Horizontal move to position:" << x << y;
    D_Q(DArrowRectangle);

    const QRect dRect = currentScreenRect(x, y);
    qreal delta = m_handle ? 0 : (q->shadowBlurRadius() - m_shadowDistance);
    qCDebug(logWindowPlatform) << "Screen rect:" << dRect << "delta:" << delta;

    int lRelativeX = x - dRect.x() - (q->width() - delta) / 2;
    int rRelativeX = x - dRect.x() + (q->width() - delta) / 2 - dRect.width();
    int absoluteX = 0;
    qCDebug(logWindowPlatform) << "Relative positions - left:" << lRelativeX << "right:" << rRelativeX;

    if (lRelativeX < 0) { //out of screen in left side
        qCDebug(logWindowPlatform) << "Out of screen in left side";
        //arrowX use relative coordinates
        q->setArrowX((q->width() - delta) / 2 + lRelativeX);
        absoluteX = dRect.x() - delta;
    } else if (rRelativeX > 0) { //out of screen in right side
        qCDebug(logWindowPlatform) << "Out of screen in right side";
        q->setArrowX(q->width() / 2 - delta * 2 + rRelativeX);
        absoluteX = dRect.x() + dRect.width() - q->width() + delta;
    } else {
        qCDebug(logWindowPlatform) << "Within screen bounds";
        q->setArrowX(0);
        absoluteX = x - (m_arrowX > 0 ? m_arrowX : (q->width() / 2));
    }

    qCDebug(logWindowPlatform) << "Final absolute X position:" << absoluteX;
    switch (m_arrowDirection) {
    case DArrowRectangle::ArrowTop:
        qCDebug(logWindowPlatform) << "Moving with top arrow to:" << absoluteX << y;
        q->QWidget::move(absoluteX, y);
        break;
    case DArrowRectangle::ArrowBottom:
        qCDebug(logWindowPlatform) << "Moving with bottom arrow to:" << absoluteX << (y - q->height());
        q->QWidget::move(absoluteX, y - q->height());
        break;
    default:
        qCDebug(logWindowPlatform) << "Ignoring move for invalid arrow direction";
        break;
    }
    qCDebug(logWindowPlatform) << "Horizontal move completed";
}

void DArrowRectanglePrivate::updateClipPath()
{
    qCDebug(logWindowPlatform) << "Updating clip path";
    D_Q(DArrowRectangle);

    if (!isDwayland() && !m_handle) {
        qCDebug(logWindowPlatform) << "Skipping clip path update - not Wayland and no handle";
        return;
    }

    QPainterPath path;
    qCDebug(logWindowPlatform) << "Getting path for arrow direction:" << static_cast<int>(m_arrowDirection);

    switch (m_arrowDirection) {
    case DArrowRectangle::ArrowLeft:
        qCDebug(logWindowPlatform) << "Using left corner path";
        path = getLeftCornerPath();
        break;
    case DArrowRectangle::ArrowRight:
        qCDebug(logWindowPlatform) << "Using right corner path";
        path = getRightCornerPath();
        break;
    case DArrowRectangle::ArrowTop:
        qCDebug(logWindowPlatform) << "Using top corner path";
        path = getTopCornerPath();
        break;
    case DArrowRectangle::ArrowBottom:
        qCDebug(logWindowPlatform) << "Using bottom corner path";
        path = getBottomCornerPath();
        break;
    default:
        qCDebug(logWindowPlatform) << "Using default right corner path";
        path = getRightCornerPath();
    }

    if (m_handle) {
        qCDebug(logWindowPlatform) << "Setting clip path on handle";
        m_handle->setClipPath(path);
    } else if (DArrowRectangle::FloatWindow == floatMode && isDwayland()) {
        qCDebug(logWindowPlatform) << "Setting clip path for Wayland float window";
        // clipPath without handle
        QPainterPathStroker stroker;
        stroker.setCapStyle(Qt::RoundCap);
        stroker.setJoinStyle(Qt::RoundJoin);
        stroker.setWidth(2);
        QPainterPath outPath = stroker.createStroke(path);
        QPolygon polygon = outPath.united(path).toFillPolygon().toPolygon();

        qCDebug(logWindowPlatform) << "Setting window mask";
        q->clearMask();
        q->setMask(polygon);
        if (m_blurBackground) {
            qCDebug(logWindowPlatform) << "Setting blur background mask path";
            m_blurBackground->setMaskPath(path);
        }

        if (QWidget *widget = q->window()) {
            if (QWindow *w = widget->windowHandle()) {
                qCDebug(logWindowPlatform) << "Setting window blur paths";
                QList<QPainterPath> painterPaths;
                painterPaths << outPath.united(path);
                // 背景模糊也要设置一个 path
                qApp->platformNativeInterface()->setWindowProperty(w->handle(), "_d_windowBlurPaths",
                                                                   QVariant::fromValue(painterPaths));
            }
        }
    }
    qCDebug(logWindowPlatform) << "Clip path update completed";
}

bool DArrowRectanglePrivate::radiusEnabled()
{
    qCDebug(logWindowPlatform) << "Checking if radius is enabled";
    D_Q(DArrowRectangle);
    if (q->radiusForceEnabled()) {
        qCDebug(logWindowPlatform) << "Radius force enabled, returning true";
        return true;
    }

    if (m_wmHelper && !m_wmHelper->hasComposite()) {
        qCDebug(logWindowPlatform) << "No composite available, radius disabled";
        return false;
    }

    qCDebug(logWindowPlatform) << "Radius enabled by default";
    return true;
}

DArrowRectangle::~DArrowRectangle()
{
    qCDebug(logWindowPlatform) << "Destroying DArrowRectangle";
}

Dtk::Widget::DArrowRectanglePrivate::DArrowRectanglePrivate(DArrowRectangle::ArrowDirection direction, DArrowRectangle *q)
    : DObjectPrivate(q)
    , m_arrowDirection(direction)
{
    qCDebug(logWindowPlatform) << "Creating DArrowRectanglePrivate with direction:" << static_cast<int>(direction);
    if (direction == DArrowRectangle::ArrowLeft || direction == DArrowRectangle::ArrowRight) {
        qCDebug(logWindowPlatform) << "Setting arrow dimensions for left/right direction";
        m_arrowWidth = 16;
        m_arrowHeight = 16;
    }
    qCDebug(logWindowPlatform) << "DArrowRectanglePrivate created";
}

void DArrowRectanglePrivate::init(DArrowRectangle::FloatMode mode)
{
    qCDebug(logWindowPlatform) << "Initializing DArrowRectanglePrivate with mode:" << static_cast<int>(mode);
    D_Q(DArrowRectangle);

    floatMode = mode;

    if (floatMode) {
        qCDebug(logWindowPlatform) << "Setting up float mode window flags";
        q->setWindowFlags(Qt::Widget);
        q->setAttribute(Qt::WA_TranslucentBackground);
        qCDebug(logWindowPlatform) << "Float mode window flags set: Qt::Widget, WA_TranslucentBackground";
    } else {
        qCDebug(logWindowPlatform) << "Setting up non-float mode window flags";
        q->setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
        q->setAttribute(Qt::WA_TranslucentBackground);
        qCDebug(logWindowPlatform) << "Non-float mode window flags set: FramelessWindowHint | ToolTip, WA_TranslucentBackground";
    }

    const bool hasBlurWindow = DWindowManagerHelper::instance()->hasBlurWindow();
    const bool isTablet = DGuiApplicationHelper::instance()->isTabletEnvironment();
    qCDebug(logWindowPlatform) << "Environment check - hasBlurWindow:" << hasBlurWindow << "isTablet:" << isTablet;

    if (!hasBlurWindow && isTablet) {
        qCDebug(logWindowPlatform) << "Disabling translucent background for tablet environment without blur";
        q->setAttribute(Qt::WA_TranslucentBackground, false);
    }

    if (DApplication::isDXcbPlatform() && (DArrowRectangle::FloatWindow == floatMode)) {
        qCDebug(logWindowPlatform) << "Setting up DXcb float window";
        m_handle = new DPlatformWindowHandle(q, q);
        m_handle->setTranslucentBackground(true);
        qCDebug(logWindowPlatform) << "DPlatformWindowHandle created with translucent background";

        m_blurBackground = new DBlurEffectWidget(q);
        m_blurBackground->setBlendMode(DBlurEffectWidget::BehindWindowBlend);
        m_blurBackground->setAccessibleName("DArrowRectangleBlurBackground");
        qCDebug(logWindowPlatform) << "Blur background widget created with blend mode:" << static_cast<int>(DBlurEffectWidget::BehindWindowBlend);

        m_wmHelper = DWindowManagerHelper::instance();

        qCDebug(logWindowPlatform) << "Connecting composite change signal";
        q->connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged, q, [q, this](){
            qCDebug(logWindowPlatform) << "Composite changed, updating window";
            q->update();
            this->updateClipPath();
        }, Qt::QueuedConnection);
        qCDebug(logWindowPlatform) << "Composite change signal connected with QueuedConnection";
    } else if (DArrowRectangle::FloatWidget == floatMode) {
        qCDebug(logWindowPlatform) << "Setting up float widget mode";
        DGraphicsGlowEffect *glowEffect = new DGraphicsGlowEffect;
        glowEffect->setBlurRadius(q->shadowBlurRadius());
        glowEffect->setDistance(m_shadowDistance);
        glowEffect->setXOffset(q->shadowXOffset());
        glowEffect->setYOffset(q->shadowYOffset());
        qCDebug(logWindowPlatform) << "Glow effect configured - blurRadius:" << q->shadowBlurRadius() 
                                  << "distance:" << m_shadowDistance << "xOffset:" << q->shadowXOffset() 
                                  << "yOffset:" << q->shadowYOffset();
        q->setGraphicsEffect(glowEffect);
        qCDebug(logWindowPlatform) << "Glow effect applied to widget";
    } else {
#ifndef QT_DEBUG
        qCDebug(logWindowPlatform) << "Using default mode - wayland:" << isDwayland() << "floatMode:" << floatMode;
#endif
    }
    qCDebug(logWindowPlatform) << "Initialization completed successfully";
}

void DArrowRectanglePrivate::show(int x, int y)
{
    qCDebug(logWindowPlatform) << "Showing DArrowRectangle at position:" << x << y << "with float mode:" << static_cast<int>(floatMode);
    D_Q(DArrowRectangle);

    // if show with FloatMode, must has a parent
    if (DArrowRectangle::FloatWidget == floatMode && !q->parentWidget()) {
        qCritical() << q << "Must has parentWidget when show in FloatWidget mode";
        qCDebug(logWindowPlatform) << "Error: Missing parent widget in float widget mode, current parent:" << q->parentWidget();
        Q_ASSERT_X(q->parentWidget(), "DArrowRectanglePrivate::show", "Must has parentWidget when show in FloatWidget mode");
    }

    qCDebug(logWindowPlatform) << "Resizing with content, current size:" << q->size();
    q->resizeWithContent();
    qCDebug(logWindowPlatform) << "Size after resize:" << q->size();

    qCDebug(logWindowPlatform) << "Updating last position from:" << m_lastPos << "to:" << QPoint(x, y);
    m_lastPos = QPoint(x, y);
    qCDebug(logWindowPlatform) << "Moving widget to position:" << x << y;
    q->move(x, y);//Overload function

    const bool wasVisible = q->isVisible();
    if (!wasVisible) {
        qCDebug(logWindowPlatform) << "Widget not visible, showing and activating window";
        q->QWidget::show();
        q->QWidget::activateWindow();
        qCDebug(logWindowPlatform) << "Widget visibility state changed from hidden to shown";
    } else {
        qCDebug(logWindowPlatform) << "Widget already visible, skipping show/activate";
    }

    qCDebug(logWindowPlatform) << "Updating widget appearance";
    q->update();
    qCDebug(logWindowPlatform) << "Updating clip path for current geometry:" << q->geometry();
    updateClipPath();
    qCDebug(logWindowPlatform) << "Show operation completed successfully";
}

void DArrowRectanglePrivate::setContent(QWidget *content)
{
    qCDebug(logWindowPlatform) << "Setting content widget:" << content << "previous content:" << m_content;
    D_Q(DArrowRectangle);

    if (!content) {
        qCDebug(logWindowPlatform) << "Content widget is null, returning without changes";
        return;
    }

    m_content = content;
    qCDebug(logWindowPlatform) << "Setting parent for content widget to:" << q;
    m_content->setParent(q);
    qCDebug(logWindowPlatform) << "Showing content widget";
    m_content->show();
    qCDebug(logWindowPlatform) << "Content widget visibility state:" << m_content->isVisible();

    const bool hasHandle = m_handle != nullptr;
    qCDebug(logWindowPlatform) << "Calculating deltas with handle:" << hasHandle;
    qreal delta = (hasHandle ? 0 : q->shadowBlurRadius() + m_shadowDistance);
    qreal deltaMargin = (hasHandle ? 0 : q->shadowBlurRadius() + m_shadowDistance) + q->margin();
    qCDebug(logWindowPlatform) << "Calculated deltas - base:" << delta << "margin:" << deltaMargin 
                              << "shadowBlurRadius:" << q->shadowBlurRadius() 
                              << "shadowDistance:" << m_shadowDistance 
                              << "margin:" << q->margin();

    qCDebug(logWindowPlatform) << "Resizing widget with new content, current size:" << q->size();
    q->resizeWithContent();
    qCDebug(logWindowPlatform) << "Size after resize:" << q->size();

    const bool isDXcb = DApplication::isDXcbPlatform();
    const bool isFloatWidget = DArrowRectangle::FloatWidget == floatMode;
    qCDebug(logWindowPlatform) << "Positioning content - isDXcb:" << isDXcb 
                              << "isFloatWidget:" << isFloatWidget 
                              << "arrowDirection:" << static_cast<int>(m_arrowDirection);

    QPoint newPos;
    switch (m_arrowDirection) {
    case DArrowRectangle::ArrowLeft:
        if (isDXcb) {
            qCDebug(logWindowPlatform) << "Calculating left arrow position (DXcb)";
            newPos = QPoint(m_arrowHeight + (isFloatWidget ? q->margin() : deltaMargin), deltaMargin);
        } else {
            qCDebug(logWindowPlatform) << "Calculating left arrow position (non-DXcb)";
            newPos = QPoint(m_arrowHeight + q->margin(), deltaMargin);
        }
        break;
    case DArrowRectangle::ArrowRight:
        if (isDXcb) {
            qCDebug(logWindowPlatform) << "Calculating right arrow position (DXcb)";
            newPos = QPoint(isFloatWidget ? deltaMargin - q->margin() : deltaMargin, deltaMargin);
        } else {
            qCDebug(logWindowPlatform) << "Calculating right arrow position (non-DXcb)";
            newPos = QPoint(isFloatWidget ? delta : delta * 2 + q->margin(), deltaMargin);
        }
        break;
    case DArrowRectangle::ArrowTop:
        if (isDXcb) {
            qCDebug(logWindowPlatform) << "Calculating top arrow position (DXcb)";
            newPos = QPoint(deltaMargin, (isFloatWidget ? q->margin() : deltaMargin) + m_arrowHeight);
        } else {
            qCDebug(logWindowPlatform) << "Calculating top arrow position (non-DXcb)";
            newPos = QPoint(deltaMargin, m_arrowHeight + deltaMargin - delta);
        }
        break;
    case DArrowRectangle::ArrowBottom:
        if (isDXcb) {
            qCDebug(logWindowPlatform) << "Calculating bottom arrow position (DXcb)";
            newPos = QPoint(deltaMargin, (isFloatWidget ? deltaMargin - q->margin() : deltaMargin));
        } else {
            qCDebug(logWindowPlatform) << "Calculating bottom arrow position (non-DXcb)";
            newPos = QPoint(deltaMargin, isFloatWidget ? delta : deltaMargin + delta);
        }
        break;
    }

    qCDebug(logWindowPlatform) << "Moving content from:" << m_content->pos() << "to:" << newPos;
    m_content->move(newPos);
    qCDebug(logWindowPlatform) << "Content positioned at:" << m_content->pos() 
                              << "with geometry:" << m_content->geometry();

    qCDebug(logWindowPlatform) << "Updating widget appearance";
    q->update();
    qCDebug(logWindowPlatform) << "Content setup completed successfully";
}

void DArrowRectanglePrivate::resizeWithContent()
{
    qCDebug(logWindowPlatform) << "Resizing with content, current size:" << m_content->size();
    D_Q(DArrowRectangle);

    if (m_content.isNull()) {
        qCDebug(logWindowPlatform) << "Content is null, skipping resize operation";
        return;
    }

    const auto& newSize = q->getFixedSize();
    qCDebug(logWindowPlatform) << "Calculated new fixed size:" << newSize 
                              << "based on content size:" << m_content->size()
                              << "and current widget size:" << q->size();
    q->setFixedSize(newSize);
    qCDebug(logWindowPlatform) << "Fixed size set successfully, new widget size:" << q->size();

#ifdef Q_OS_LINUX
    const qreal ratio = qApp->devicePixelRatio();
    qCDebug(logWindowPlatform) << "Device pixel ratio for X11 shape calculations:" << ratio;

    const bool hasHandle = m_handle != nullptr;
    const bool isXWindowPlatform = DGUI_NAMESPACE::DGuiApplicationHelper::instance()->isXWindowPlatform();
    qCDebug(logWindowPlatform) << "Platform check - hasHandle:" << hasHandle 
                              << "floatMode:" << static_cast<int>(floatMode)
                              << "isXWindowPlatform:" << isXWindowPlatform;

    if (!hasHandle && !floatMode && isXWindowPlatform) {
        qCDebug(logWindowPlatform) << "Setting up X11 shape for content";
        XRectangle m_contentXRect;
        const QPoint contentPos = m_content->pos();
        const QSize contentSize = m_content->size();
        
        m_contentXRect.x = contentPos.x() * ratio;
        m_contentXRect.y = contentPos.y() * ratio;
        m_contentXRect.width = contentSize.width() * ratio;
        m_contentXRect.height = contentSize.height() * ratio;
        
        qCDebug(logWindowPlatform) << "X11 shape rectangle - position:" << QPoint(m_contentXRect.x, m_contentXRect.y)
                                  << "size:" << QSize(m_contentXRect.width, m_contentXRect.height)
                                  << "from content position:" << contentPos
                                  << "and size:" << contentSize;

        const WId windowId = q->winId();
        qCDebug(logWindowPlatform) << "Setting X11 shape on window:" << windowId;
        XShapeCombineRectangles(QX11Info::display(), windowId, ShapeInput,
                                0, 0, &m_contentXRect, 1, ShapeSet, YXBanded);
        qCDebug(logWindowPlatform) << "X11 shape set successfully on window:" << windowId;
    } else {
        qCDebug(logWindowPlatform) << "Skipping X11 shape setup - not needed for current configuration";
    }
#endif
    qCDebug(logWindowPlatform) << "Resize with content operation completed successfully";
}

void DArrowRectanglePrivate::paintEvent(QPaintEvent *e)
{
    qCDebug(logWindowPlatform) << "Paint event received for rect:" << e->rect();
    D_Q(DArrowRectangle);

    if (m_blurBackground) {
        qCDebug(logWindowPlatform) << "Blur background widget exists, skipping direct painting";
        return;
    }

    QColor bk_color;
    if (!m_backgroundColor.isValid()) {
        bk_color = qApp->palette().window().color();
        qCDebug(logWindowPlatform) << "Using window color from application palette:" << bk_color;
    } else {
        bk_color = m_backgroundColor;
        qCDebug(logWindowPlatform) << "Using custom background color:" << bk_color 
                                  << "alpha:" << bk_color.alpha();
    }

    QPainter painter(q);
    qCDebug(logWindowPlatform) << "Painter created for widget:" << q;

    if (m_handle) {
        qCDebug(logWindowPlatform) << "Using handle-based painting for rect:" << e->rect();
        painter.fillRect(e->rect(), bk_color);
        qCDebug(logWindowPlatform) << "Rectangle filled with color:" << bk_color;
    } else {
        qCDebug(logWindowPlatform) << "Using path-based painting with antialiasing";
        painter.setRenderHint(QPainter::Antialiasing);

        QPainterPath border;
        qCDebug(logWindowPlatform) << "Getting border path for arrow direction:" << static_cast<int>(m_arrowDirection)
                                  << "current widget geometry:" << q->geometry();

        switch (m_arrowDirection) {
        case DArrowRectangle::ArrowLeft:
            qCDebug(logWindowPlatform) << "Creating left corner path";
            border = getLeftCornerPath();
            break;
        case DArrowRectangle::ArrowRight:
            qCDebug(logWindowPlatform) << "Creating right corner path";
            border = getRightCornerPath();
            break;
        case DArrowRectangle::ArrowTop:
            qCDebug(logWindowPlatform) << "Creating top corner path";
            border = getTopCornerPath();
            break;
        case DArrowRectangle::ArrowBottom:
            qCDebug(logWindowPlatform) << "Creating bottom corner path";
            border = getBottomCornerPath();
            break;
        default:
            qCDebug(logWindowPlatform) << "Using default right corner path for unknown direction";
            border = getRightCornerPath();
        }

        qCDebug(logWindowPlatform) << "Setting clip path and filling with background color";
        painter.setClipPath(border);
        painter.fillPath(border, QBrush(bk_color));

        qCDebug(logWindowPlatform) << "Configuring border stroke - color:" << m_borderColor 
                                  << "width:" << m_borderWidth;
        QPen strokePen;
        strokePen.setColor(m_borderColor);
        strokePen.setWidth(m_borderWidth);
        qCDebug(logWindowPlatform) << "Drawing border path with configured pen";
        painter.strokePath(border, strokePen);
    }
    qCDebug(logWindowPlatform) << "Paint event completed successfully";
}

void DArrowRectanglePrivate::resizeEvent(QResizeEvent *e)
{
    qCDebug(logWindowPlatform) << "Resize event received, new size:" << e->size();
    D_Q(DArrowRectangle);

    q->QWidget::resizeEvent(e);

    if (m_blurBackground) {
        qCDebug(logWindowPlatform) << "Resizing blur background";
        m_blurBackground->resize(e->size());
    }

    qCDebug(logWindowPlatform) << "Updating clip path";
    updateClipPath();
    qCDebug(logWindowPlatform) << "Resize event completed";
}

DWIDGET_END_NAMESPACE
