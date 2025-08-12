// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "popupmenustyle.h"
#include <QDebug>
#include <QPixmap>
#include <QPixmapCache>
#include <QPainter>
#include <QStyleOption>
#include <QStyleOptionMenuItem>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logPlatformSpecific)

namespace {

const QString SUB_MENU_NORMAL_ICON =
        ":/images/resources/images/sub_menu_normal.png";
const QString SUB_MENU_HOVER_ICON =
        ":/images/resources/images/sub_menu_hover.png";

const qreal FRAME_BORDER_WIDTH = 0.5;
const int FRAME_BORDER_RADIUS = 4;
const int ITEM_HEIGHT = 21;
const int ICON_PIXEL_SIZE = 16;
const int ICON_LEFT_MARGIN = 4;
const int TEXT_LEFT_MARGIN = 4;
const int SEPARATOR_HEIGHT = 9;
const int SEPARATOR_LEFT_MARGIN = 11;
const int SEPARATOR_RIGHT_MARGIN = 10;
const int TEXT_FONT_PIXEL_SIZE = 12;
const int SHORTCUT_FONT_PIXEL_SIZE = 10;

const QColor FRAME_BORDER_COLOR = QColor(255, 255, 255, 51);
const QColor TEXT_COLOR = QColor("#ffffff");
const QColor SHORTCUT_COLOR = QColor(255, 255, 255, 153);
const QColor SHORTCUT_HOVER_COLOR = QColor(255, 255, 255, 255);
const QColor ITEM_BG_NORMAL_COLOR = QColor(0, 0, 0, 204);
const QColor ITEM_BG_HOVER_COLOR = QColor(0, 188, 255, 255);
const QColor SEPARATOR_COLOR = QColor(255, 255, 255, 51);

}  // namespace

PopupMenuStyle::PopupMenuStyle()
    : QProxyStyle()
{
    qCDebug(logPlatformSpecific) << "PopupMenuStyle constructor called";
}

int PopupMenuStyle::styleHint(QStyle::StyleHint hint,
                              const QStyleOption* option,
                              const QWidget* widget,
                              QStyleHintReturn* returnData) const
{
    qCDebug(logPlatformSpecific) << "styleHint called with hint:" << hint;
    switch (hint) {
    case QStyle::SH_Menu_Scrollable:
        qCDebug(logPlatformSpecific) << "returning scrollable menu hint: 1";
        return 1;
    case QStyle::SH_Menu_KeyboardSearch:
        qCDebug(logPlatformSpecific) << "returning keyboard search hint: 0";
        return 0;
    default:
        qCDebug(logPlatformSpecific) << "using default style hint";
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
}

int PopupMenuStyle::pixelMetric(QStyle::PixelMetric metric,
                                const QStyleOption* option,
                                const QWidget* widget) const
{
    qCDebug(logPlatformSpecific) << "pixelMetric called with metric:" << metric;
    switch (metric) {
    case QStyle::PM_MenuScrollerHeight:
        qCDebug(logPlatformSpecific) << "returning menu scroller height: 15";
        return 15;
    case QStyle::PM_MenuDesktopFrameWidth:
        qCDebug(logPlatformSpecific) << "returning menu desktop frame width: 0";
        return 0;
    case QStyle::PM_SubMenuOverlap:
        qCDebug(logPlatformSpecific) << "returning submenu overlap: -1";
        return -1;
    case QStyle::PM_MenuVMargin:
        qCDebug(logPlatformSpecific) << "returning menu vertical margin: 8";
        return 8;
    default:
        qCDebug(logPlatformSpecific) << "using default pixel metric";
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

void PopupMenuStyle::drawControl(QStyle::ControlElement control,
                                 const QStyleOption* option,
                                 QPainter* painter,
                                 const QWidget* widget) const
{
    qCDebug(logPlatformSpecific) << "drawControl called with control:" << control;
    switch (control) {
    case QStyle::CE_MenuItem:
        qCDebug(logPlatformSpecific) << "drawing menu item";
        drawMenuItem(option, painter);
        break;
        //    case QStyle::CE_MenuScroller:
        //        drawScroller(option, painter);
        //        break;
    default:
        qCDebug(logPlatformSpecific) << "using default draw control";
        QProxyStyle::drawControl(control, option, painter, widget);
    }
}

void PopupMenuStyle::drawPrimitive(QStyle::PrimitiveElement element,
                                   const QStyleOption *option,
                                   QPainter *painter,
                                   const QWidget *widget) const
{
    qCDebug(logPlatformSpecific) << "drawPrimitive called with element:" << element;
    if (element == QStyle::PE_PanelMenu) {
        qCDebug(logPlatformSpecific) << "drawing menu panel";
        painter->setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        // Skill
        // Make path's corner draw smooth
        // http://www.cnblogs.com/smileEvday/p/iOS_PixelVsPoint.html
        QRectF rect = option->rect;
        rect.setTopLeft(QPointF(0.5, 0.5));
        rect.setBottomRight(QPointF(rect.width() - 1, rect.height() - 1));

        path.addRoundedRect(rect,
                            FRAME_BORDER_RADIUS, FRAME_BORDER_RADIUS);
        painter->fillPath(path, QBrush(ITEM_BG_NORMAL_COLOR));

        // Do not draw outside border if it's sub-menu
        if (widget->parentWidget()) {
            qCDebug(logPlatformSpecific) << "drawing sub-menu border";
            // Skill
            // Draw outside border
            QPainterPathStroker stroker;
            stroker.setWidth(FRAME_BORDER_WIDTH);
            stroker.setJoinStyle(Qt::RoundJoin);
            QPainterPath borderPath = stroker.createStroke(path);
            QPen pen(FRAME_BORDER_COLOR, FRAME_BORDER_WIDTH);
            painter->setPen(pen);
            painter->drawPath(borderPath);
        }
    }
    else {
        qCDebug(logPlatformSpecific) << "using default draw primitive";
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
}

QSize PopupMenuStyle::sizeFromContents(QStyle::ContentsType type,
                                       const QStyleOption* option,
                                       const QSize& contentsSize,
                                       const QWidget* widget) const
{
    qCDebug(logPlatformSpecific) << "sizeFromContents called with type:" << type;
    QSize originSize = QProxyStyle::sizeFromContents(type, option, contentsSize,
                                                     widget);
    switch (type) {
    case QStyle::CT_MenuItem:
        qCDebug(logPlatformSpecific) << "calculating menu item size";
        return getItemSize(option, originSize);
    default:
        qCDebug(logPlatformSpecific) << "using default size from contents";
        return originSize;
    }
}

void PopupMenuStyle::drawScroller(const QStyleOption *option,
                                  QPainter *painter) const
{
    qCDebug(logPlatformSpecific) << "drawScroller called (unimplemented)";
    Q_UNUSED(option)
    Q_UNUSED(painter)
    //    QRect rect = option->rect;
    //    // Draw background
    //    painter->drawPixmap(rect, QPixmap(kItemNormalBackground));
    //    // Draw arrow
    //    QPixmap arrow_pixmap = getArrowPixmap(option->state);
    //    QRect arrow_rect(rect.x() + (rect.width() - arrow_pixmap.width()) / 2,
    //                     rect.y() + (rect.height() - arrow_pixmap.height()) / 2,
    //                     arrow_pixmap.width(),
    //                     arrow_pixmap.height());
    //    painter->drawPixmap(arrow_rect, arrow_pixmap);
}

void PopupMenuStyle::drawMenuItem(const QStyleOption* option,
                                  QPainter* painter) const
{
    qCDebug(logPlatformSpecific) << "drawMenuItem called";
    if (const QStyleOptionMenuItem* menuItem =
            qstyleoption_cast<const QStyleOptionMenuItem* >(option)) {

        if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
            qCDebug(logPlatformSpecific) << "drawing menu separator";
            drawSeparator(menuItem, painter);
        } else {
            qCDebug(logPlatformSpecific) << "drawing menu item with type:" 
                                        << menuItem->menuItemType;
            // Draw background
            painter->fillRect(menuItem->rect,
                              QBrush(getItemBGColor(menuItem->state)));

            drawMenuItemIcon(menuItem, painter);
            drawMenuItemText(menuItem, painter);
            drawMenuItemSubIcon(menuItem, painter);
            drawMenuItemCheckedIcon(menuItem, painter);
        }
    }
}

void PopupMenuStyle::drawSeparator(const QStyleOptionMenuItem *menuItem,
                                   QPainter *painter) const
{
    qCDebug(logPlatformSpecific) << "drawSeparator called";
//    painter->fillRect(menuItem->rect, QBrush(ITEM_BG_NORMAL_COLOR));

    const int x1 = menuItem->rect.x() + SEPARATOR_LEFT_MARGIN;
    const int x2 = menuItem->rect.x() + menuItem->rect.width() -
            SEPARATOR_RIGHT_MARGIN;
    const int y = menuItem->rect.y() + (SEPARATOR_HEIGHT - 1) / 2;
    // Note pixel calculate kill, make sure draw the line is 1px height
    QLineF line(x1, y + 0.5, x2, y + 0.5);
    QPen pen(SEPARATOR_COLOR, 1);
    painter->setPen(pen);
    painter->drawLine(line);
}

void PopupMenuStyle::drawMenuItemIcon(const QStyleOptionMenuItem* menuItem,
                                      QPainter* painter) const
{
    qCDebug(logPlatformSpecific) << "drawMenuItemIcon called";
    QPixmap pixmap = getIconPixmap(menuItem->state,
                                   menuItem->icon,
                                   QSize(ICON_PIXEL_SIZE,
                                         ICON_PIXEL_SIZE));
    if (pixmap.isNull()) {
        qCDebug(logPlatformSpecific) << "menu item icon is null, skipping";
        return;
    }

    int y = menuItem->rect.y() + (menuItem->rect.height() - pixmap.height()) / 2;
    painter->drawPixmap(ICON_LEFT_MARGIN, y, pixmap.width(), pixmap.height(),
                        pixmap);
}

void PopupMenuStyle::drawMenuItemText(const QStyleOptionMenuItem* menuItem,
                                      QPainter* painter) const
{
    qCDebug(logPlatformSpecific) << "drawMenuItemText called";
    int width = menuItem->rect.width();
    int height = menuItem->rect.height();
    int y = menuItem->rect.y();

    // Draw shortcut
    QFont shortcutFont;
    shortcutFont.setPixelSize(SHORTCUT_FONT_PIXEL_SIZE);
    QFontMetrics shortcutFM(shortcutFont);
    const QString shortcutContent = menuItem->text.split(SHORTCUT_SPLIT_FLAG).last();
    const int shortcutWidth = shortcutFM.width(shortcutContent.trimmed());
    const int shortcutX = width - SEPARATOR_RIGHT_MARGIN - shortcutWidth;
    QPen shortcutPen((menuItem->state & QStyle::State_Selected) ?
                         SHORTCUT_HOVER_COLOR : SHORTCUT_COLOR);
    painter->setPen(shortcutPen);
    painter->setFont(shortcutFont);
    painter->drawText(shortcutX, y, shortcutWidth, height,
                      Qt::AlignRight | Qt::AlignVCenter,
                      shortcutContent);

    // Draw title
    QFont titleFont;
    titleFont.setPixelSize(TEXT_FONT_PIXEL_SIZE);
    QFontMetrics titleFM(titleFont);
    const QString titleContent = menuItem->text.split(SHORTCUT_SPLIT_FLAG).first();

    int leftSpacing = ICON_LEFT_MARGIN + ICON_PIXEL_SIZE + TEXT_LEFT_MARGIN;
    const int titleWidth = width - leftSpacing - shortcutWidth - SEPARATOR_LEFT_MARGIN;
    int titleX = leftSpacing;

    QPen titlePen(TEXT_COLOR);
    painter->setPen(titlePen);
    painter->setFont(titleFont);
    painter->drawText(titleX, y, titleWidth, height,
                      Qt::AlignLeft | Qt::AlignVCenter,
                      titleFM.elidedText(titleContent, Qt::ElideRight, titleWidth));
}

void PopupMenuStyle::drawMenuItemSubIcon(const QStyleOptionMenuItem* menuItem,
                                         QPainter* painter) const
{
    // Draw sub-menu icon
    if (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu) {
        qCDebug(logPlatformSpecific) << "drawing sub-menu icon";
        QPixmap pixmap(getSubMenuPixmap(menuItem->state));

        int height = menuItem->rect.height();
        int x = menuItem->rect.width() - TEXT_LEFT_MARGIN - ICON_PIXEL_SIZE;
        int y = menuItem->rect.y() + (height - pixmap.height()) / 2;
        painter->drawPixmap(x, y, pixmap.width(), pixmap.height(), pixmap);
    }
}

void PopupMenuStyle::drawMenuItemCheckedIcon(const QStyleOptionMenuItem* menuItem,
                                             QPainter* painter) const
{
    qCDebug(logPlatformSpecific) << "drawMenuItemCheckedIcon called (unimplemented)";
    Q_UNUSED(menuItem)
    Q_UNUSED(painter)
    //    if (menuItem->checked) {
    //        QPixmap pixmap;
    //        QPixmapCache::find(kCheckedIconCacheKey, &pixmap);

    //        int height = menuItem->rect.height();
    //        int x = menuItem->rect.width() - TEXT_LEFT_MARGIN - ICON_PIXEL_SIZE;
    //        int y = menuItem->rect.y() + (height - pixmap.height()) / 2;
    //        painter->drawPixmap(x, y, pixmap.width(), pixmap.height(), pixmap);
    //    }
}

QSize PopupMenuStyle::getItemSize(const QStyleOption* option,
                                  const QSize& size) const
{
    qCDebug(logPlatformSpecific) << "getItemSize called";
    const int m = SEPARATOR_LEFT_MARGIN + SEPARATOR_RIGHT_MARGIN;
    if (const QStyleOptionMenuItem* menuItem =
            qstyleoption_cast<const QStyleOptionMenuItem* >(option)) {
        switch (menuItem->menuItemType) {
        case QStyleOptionMenuItem::Separator:
            qCDebug(logPlatformSpecific) << "returning separator size";
            return QSize(size.width() / 2 + m, SEPARATOR_HEIGHT);
        case QStyleOptionMenuItem::Normal:
        case QStyleOptionMenuItem::SubMenu:
            qCDebug(logPlatformSpecific) << "returning menu item size";
            return QSize(size.width() / 2 + m, ITEM_HEIGHT);
        default:
            qCDebug(logPlatformSpecific) << "returning default size";
            return size;
        }
    }
    else {
        qCDebug(logPlatformSpecific) << "returning original size";
        return size;
    }
}

QColor PopupMenuStyle::getItemBGColor(const QStyle::State state) const
{
    qCDebug(logPlatformSpecific) << "getItemBGColor called with state:" << state;
    // QStyle::State_MouseOver not working, don't know why
    if (state & QStyle::State_Selected) {
        qCDebug(logPlatformSpecific) << "returning hover background color";
        return ITEM_BG_HOVER_COLOR;
    } else {
        qCDebug(logPlatformSpecific) << "returning transparent background color";
        return QColor(0, 0, 0, 0);
    }
}

QPixmap PopupMenuStyle::getSubMenuPixmap(const QStyle::State state) const
{
    qCDebug(logPlatformSpecific) << "getSubMenuPixmap called with state:" << state;
    if (state & QStyle::State_Selected) {
        qCDebug(logPlatformSpecific) << "returning hover sub-menu pixmap";
        return SUB_MENU_HOVER_ICON;
    } else {
        qCDebug(logPlatformSpecific) << "returning normal sub-menu pixmap";
        return SUB_MENU_NORMAL_ICON;
    }
}

QPixmap PopupMenuStyle::getIconPixmap(const QStyle::State state,
                                      const QIcon& icon,
                                      const QSize& size) const
{
    qCDebug(logPlatformSpecific) << "getIconPixmap called with state:" << state;
    QIcon::Mode iconMode = QIcon::Normal;
    QIcon::State iconState = QIcon::Off;
    if (state & QStyle::State_Active) {
        iconMode = QIcon::Active;
    } else if (state & QStyle::State_Selected) {
        iconMode = QIcon::Selected;
    } else if (!(state & QStyle::State_Enabled)) {
        iconMode = QIcon::Disabled;
    }

    if (state & QStyle::State_On) {
        iconState = QIcon::On;
    }

    return icon.pixmap(size, iconMode, iconState);
}
