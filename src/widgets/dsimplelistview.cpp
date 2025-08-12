// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsimplelistview.h"
#include <DObjectPrivate>
#include <DIcon>
#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QMenu>
#include <QStyleFactory>
#include <QWheelEvent>
#include <QtMath>
#include <QPointer>
#include <QPainterPath>
#include <QLoggingCategory>

DCORE_USE_NAMESPACE
DGUI_USE_NAMESPACE
DWIDGET_BEGIN_NAMESPACE
Q_DECLARE_LOGGING_CATEGORY(logListWidgets)

class DSimpleListViewPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    DSimpleListViewPrivate(DSimpleListView *parent)
        : DObjectPrivate(parent)
    {
    }

    QList<DSimpleListItem*> getSearchItems(QList<DSimpleListItem*> items);
    int getItemsTotalHeight();
    int getTopRenderOffset();
    void sortItemsByColumn(int column, bool descendingSort);

    QPointer<DSimpleListItem> lastHoverItem = nullptr;
    QPointer<DSimpleListItem> lastSelectItem = nullptr;
    QPointer<DSimpleListItem> drawHoverItem = nullptr;
    QPointer<DSimpleListItem> mouseHoverItem = nullptr;
    QList<DSimpleListItem*> *listItems = nullptr;
    QList<DSimpleListItem*> *renderItems = nullptr;
    QList<DSimpleListItem*> *selectionItems = nullptr;
    QList<QString> columnTitles = {};
    QList<SortAlgorithm> *sortingAlgorithms = nullptr;
    QList<bool> *sortingOrderes = nullptr;
    QList<int> columnWidths = {};
    QString searchContent = "";
    QTimer *hideScrollbarTimer = nullptr;
    SearchAlgorithm searchAlgorithm = nullptr;
    bool defaultSortingOrder = false;
    bool mouseAtScrollArea = false;
    bool mouseDragScrollbar =false;
    bool drawFrame = false;
    bool isKeepSelectWhenClickBlank;
    bool isSingleSelect = false;
    int alwaysVisibleColumn = 0;
    int clipRadius = 0;
    int defaultSortingColumn = 0;
    int hideScrollbarDuration = 0;
    int lastHoverColumnIndex = 0;
    int oldRenderOffset = 0;
    int renderOffset = 0;
    int rowHeight = 0;
    int scrollDistance = 0;
    int scrollStartY = 0;
    int scrollUnit = 0;
    int scrollbarDefaultWidth = 0;
    int scrollbarDragWidth = 0;
    int scrollbarMinHeight = 0;
    int scrollbarPadding = 0;
    int titleArrowPadding = 0;
    int titleHeight = 0;
    int titleHoverColumn = 0;
    int titlePadding = 0;
    int titlePressColumn = 0;

    D_DECLARE_PUBLIC(DSimpleListView)
};

/*!
  \class Dtk::Widget::DSimpleListView
  \inmodule dtkwidget
  \brief DSimpleListView 是 deepin 基于 QWidget 从零绘制的列表控件.

  开发者只要懂得怎么使用 QPainter 进行图形绘制，开发者就可以在 DSimpleListItem 中绘制任意行内容，
  包括文本、图片、任意控件甚至每行都可以画一个小电影，而代码的复杂度不会随着绘制行内容而发生变化，
  所有的行内容都源于怎么使用 QPainter.
 */

DSimpleListView::DSimpleListView(QWidget *parent) : QWidget(parent), DObject(*new DSimpleListViewPrivate(this))
{
    qCDebug(logListWidgets) << "create DSimpleListView";
    D_D(DSimpleListView);

    // Init.
    installEventFilter(this);   // add event filter
    setMouseTracking(true);    // make MouseMove can response

    d->scrollDistance = 0;
    d->renderOffset = 0;
    d->titleHeight = 0;
    d->titleArrowPadding = 4;
    d->titlePadding = 14;
    d->rowHeight = 36;

    d->searchContent = "";
    d->searchAlgorithm = NULL;

    d->titleHoverColumn = -1;
    d->titlePressColumn = -1;

    arrowUpDarkNormalImage = DIcon::loadNxPixmap(":/images/dark/images/arrow_up_normal.svg");
    arrowUpDarkHoverImage = DIcon::loadNxPixmap(":/images/dark/images/arrow_up_hover.svg");
    arrowUpDarkPressImage = DIcon::loadNxPixmap(":/images/dark/images/arrow_up_press.svg");
    arrowDownDarkNormalImage = DIcon::loadNxPixmap(":/images/dark/images/arrow_down_normal.svg");
    arrowDownDarkHoverImage = DIcon::loadNxPixmap(":/images/dark/images/arrow_down_hover.svg");
    arrowDownDarkPressImage = DIcon::loadNxPixmap(":/images/dark/images/arrow_down_press.svg");

    arrowUpLightNormalImage = DIcon::loadNxPixmap(":/images/light/images/arrow_up_normal.svg");
    arrowUpLightHoverImage = DIcon::loadNxPixmap(":/images/light/images/arrow_up_hover.svg");
    arrowUpLightPressImage = DIcon::loadNxPixmap(":/images/light/images/arrow_up_press.svg");
    arrowDownLightNormalImage = DIcon::loadNxPixmap(":/images/light/images/arrow_down_normal.svg");
    arrowDownLightHoverImage = DIcon::loadNxPixmap(":/images/light/images/arrow_down_hover.svg");
    arrowDownLightPressImage = DIcon::loadNxPixmap(":/images/light/images/arrow_down_press.svg");

    arrowUpNormalImage = arrowUpLightNormalImage;
    arrowUpHoverImage = arrowUpLightHoverImage;
    arrowUpPressImage = arrowUpLightPressImage;
    arrowDownNormalImage = arrowDownLightNormalImage;
    arrowDownHoverImage = arrowDownLightHoverImage;
    arrowDownPressImage = arrowDownLightPressImage;

    d->listItems = new QList<DSimpleListItem*>();
    d->renderItems = new QList<DSimpleListItem*>();
    d->selectionItems = new QList<DSimpleListItem*>();
    d->lastSelectItem = NULL;
    d->lastHoverItem = NULL;
    d->lastHoverColumnIndex = -1;
    d->drawHoverItem = NULL;
    d->mouseHoverItem = NULL;

    d->mouseAtScrollArea = false;
    d->mouseDragScrollbar = false;
    d->drawFrame = false;
    d->isKeepSelectWhenClickBlank = false;
    d->isSingleSelect = false;
    d->scrollbarDefaultWidth = 4;
    d->scrollbarDragWidth = 8;
    d->scrollbarMinHeight = 30;
    d->scrollbarPadding = 4;
    d->hideScrollbarDuration = 2000;

    d->oldRenderOffset = 0;
    d->clipRadius = 0;

    d->hideScrollbarTimer = NULL;

    d->sortingAlgorithms = new QList<SortAlgorithm>();
    d->sortingOrderes = new QList<bool>();
}

DSimpleListView::~DSimpleListView()
{
    qCDebug(logListWidgets) << "destroy DSimpleListView";
    D_D(DSimpleListView);

    delete d->lastHoverItem.data();
    delete d->lastSelectItem.data();
    delete d->drawHoverItem.data();
    delete d->mouseHoverItem.data();
    delete d->listItems;
    delete d->renderItems;
    delete d->selectionItems;
    delete d->sortingAlgorithms;
    delete d->sortingOrderes;
    delete d->hideScrollbarTimer;
}

/*!
  \brief 设置行高.

  \a height 行高.
 */
void DSimpleListView::setRowHeight(int height)
{
    qCDebug(logListWidgets) << "Setting row height to:" << height;
    D_D(DSimpleListView);

    if (d->rowHeight == height) {
        qCDebug(logListWidgets) << "Row height unchanged, skipping update";
        return;
    }

    d->rowHeight = height;
    d->scrollUnit = d->rowHeight;
    qCDebug(logListWidgets) << "Row height updated successfully";
}

/*!
  \brief 设置列标题信息

  \a titles 标题列表， \a widths 宽度列表, \a height 高度.
 */
void DSimpleListView::setColumnTitleInfo(QList<QString> titles, QList<int> widths, int height)
{
    qCDebug(logListWidgets) << "Setting column title info - titles count:" << titles.size() << "widths count:" << widths.size() << "height:" << height;
    D_D(DSimpleListView);

    // Set column titles.
    d->columnTitles = titles;

    // Calculate column title widths.
    d->columnWidths.clear();

    QFont font;
    font.setPointSize(titleSize);
    QFontMetrics fm(font);

    for (int i = 0; i < widths.length(); i++) {
        if (widths[i] == -1) {
            qCDebug(logListWidgets) << "Column" << i << "using auto width";
            d->columnWidths << widths[i];
        } else {
            int renderTitleWidth = fm.horizontalAdvance(titles[i]) + d->titlePadding + arrowUpNormalImage.width() / arrowUpNormalImage.devicePixelRatio() + d->titleArrowPadding * 2;
            int finalWidth = std::max(widths[i], renderTitleWidth);
            qCDebug(logListWidgets) << "Column" << i << "width:" << finalWidth << "(requested:" << widths[i] << "calculated:" << renderTitleWidth << ")";
            d->columnWidths << finalWidth;
        }
    }

    // Set title height.
    d->titleHeight = height;
    qCDebug(logListWidgets) << "Column title info set successfully";
}

/*!
  \brief 设置列隐藏标志
  \a toggleHideFlags 用于控制列的隐藏标志，用于切换显示/隐藏。
  \a visibleColumnIndex 列始终可见的列索引，默认值为 -1，表示列始终不可见。
 */
void DSimpleListView::setColumnHideFlags(QList<bool> toggleHideFlags, int visibleColumnIndex)
{
    qCDebug(logListWidgets) << "Setting column hide flags - count:" << toggleHideFlags.count() << "always visible column:" << visibleColumnIndex;
    D_D(DSimpleListView);

    //Q_ASSERT_X(toggleHideFlags.contains(false), "toggleHideFlags", "at least have one 'false' in list.");
    Q_ASSERT_X(toggleHideFlags.count() == d->columnTitles.count(), "toggleHideFlags", "hide flags length is not same as titles list.");

    d->alwaysVisibleColumn = visibleColumnIndex;

    columnVisibles.clear();
    for (int i = 0; i < toggleHideFlags.count(); i++) {
        qCDebug(logListWidgets) << "Column" << i << "visible:" << toggleHideFlags[i];
        columnVisibles.append(toggleHideFlags[i]);
    }
    qCDebug(logListWidgets) << "Column hide flags set successfully";
}

/*!
  \brief 设置列排序算法.

  \a algorithms 列排序算法 , \a sortColumn 排序的列项 , \a descendingSort 是否为降序排序.
 */
void DSimpleListView::setColumnSortingAlgorithms(QList<SortAlgorithm> *algorithms, int sortColumn, bool descendingSort)
{
    qCDebug(logListWidgets) << "Setting column sorting algorithms - count:" << (algorithms ? algorithms->count() : 0) << "sort column:" << sortColumn << "descending:" << descendingSort;
    D_D(DSimpleListView);

    // Add sort algorithms.
    d->sortingAlgorithms = algorithms;

    if (algorithms) {
        for (int i = 0; i < d->sortingAlgorithms->count(); i++) {
            d->sortingOrderes->append(false);
        }
        qCDebug(logListWidgets) << "Added" << d->sortingAlgorithms->count() << "sorting algorithms";
    } else {
        qCDebug(logListWidgets) << "No sorting algorithms provided";
    }

    // If sort column is -1, don't sort default.
    d->defaultSortingColumn = sortColumn;
    d->defaultSortingOrder = descendingSort;
    
    if (sortColumn == -1) {
        qCDebug(logListWidgets) << "No default sorting column set";
    } else {
        qCDebug(logListWidgets) << "Default sorting set for column" << sortColumn;
    }
}

/*!
  \brief 设置搜索算法.

  \a algorithm 搜索算法.
 */
void DSimpleListView::setSearchAlgorithm(SearchAlgorithm algorithm)
{
    qCDebug(logListWidgets) << "Setting search algorithm";
    D_D(DSimpleListView);

    d->searchAlgorithm = algorithm;
    qCDebug(logListWidgets) << "Search algorithm set successfully";
}

/*!
  \brief 设置圆角半径.

  \a radius 圆角大小.
 */
void DSimpleListView::setClipRadius(int radius)
{
    qCDebug(logListWidgets) << "Setting clip radius to:" << radius;
    D_D(DSimpleListView);

    if (d->clipRadius == radius) {
        qCDebug(logListWidgets) << "Clip radius unchanged, skipping update";
        return;
    }

    d->clipRadius = radius;
    qCDebug(logListWidgets) << "Clip radius updated successfully";
}

/*!
  \brief 设置边角细节.

  \a enableFrame 是否绘制边角
  \a color 边角颜色
  \a opacity 不透明度
 */
void DSimpleListView::setFrame(bool enableFrame, QColor color, double opacity)
{
    qCDebug(logListWidgets) << "Setting frame - enabled:" << enableFrame << "color:" << color.name() << "opacity:" << opacity;
    D_D(DSimpleListView);

    d->drawFrame = enableFrame;
    frameColor = color;
    frameOpacity = opacity;
    qCDebug(logListWidgets) << "Frame settings updated successfully";
}

/*!
  \brief 添加 DSimpleListItem 列表到视图中.

  \a items 视图项列表.
 */
void DSimpleListView::addItems(QList<DSimpleListItem*> items)
{
    qCDebug(logListWidgets) << "Adding" << items.size() << "items to list view";
    D_D(DSimpleListView);

    // Add item to list.
    d->listItems->append(items);
    QList<DSimpleListItem*> searchItems = d->getSearchItems(items);
    d->renderItems->append(searchItems);
    qCDebug(logListWidgets) << "Items added - total list items:" << d->listItems->size() << "render items:" << d->renderItems->size();

    // If user has click title to sort, sort items after add items to list.
    if (d->defaultSortingColumn != -1) {
        qCDebug(logListWidgets) << "Sorting items by default column:" << d->defaultSortingColumn;
        d->sortItemsByColumn(d->defaultSortingColumn, d->defaultSortingOrder);
    } else {
        qCDebug(logListWidgets) << "No default sorting applied";
    }

    // Repaint after add items.
    qCDebug(logListWidgets) << "Repainting after adding items";
    repaint();
}

/*!
  \brief 删除一个列表项.

  \a item 列表项指针.
 */
void DSimpleListView::removeItem(DSimpleListItem* item)
{
    qCDebug(logListWidgets) << "Removing item from list view";
    D_D(DSimpleListView);

    if (!item) {
        qCDebug(logListWidgets) << "Item is null, skipping removal";
        return;
    }

    bool removedFromList = d->listItems->removeOne(item);
    bool removedFromRender = d->renderItems->removeOne(item);
    qCDebug(logListWidgets) << "Item removed - from list:" << removedFromList << "from render:" << removedFromRender;

    if (d->renderOffset >= d->getItemsTotalHeight() - rect().height()) {
        int oldOffset = d->renderOffset;
        d->renderOffset = adjustRenderOffset(d->renderOffset - d->rowHeight);
        qCDebug(logListWidgets) << "Adjusted render offset from" << oldOffset << "to" << d->renderOffset;
    }

    qCDebug(logListWidgets) << "Repainting after item removal";
    repaint();
}

/*!
  \brief 删除所有列表项
 */
void DSimpleListView::clearItems()
{
    qCDebug(logListWidgets) << "Clearing all items from list view";
    D_D(DSimpleListView);

    int itemCount = d->listItems->size();
    qCDebug(logListWidgets) << "Deleting" << itemCount << "items to avoid memory leak";

    // NOTE:
    // We need delete items in QList before clear QList to avoid *MEMORY LEAK* .
    qDeleteAll(d->listItems->begin(), d->listItems->end());
    d->listItems->clear();
    d->renderItems->clear();
    qCDebug(logListWidgets) << "All items cleared successfully";
}

/*!
  \brief 添加 DSimpleListItem 列表以在视图中选择效果
  \a recordLastSelection 为 true 则清除最后一个选择项
  \a items 列表项.
 */
void DSimpleListView::addSelections(QList<DSimpleListItem*> items, bool recordLastSelection)
{
    qCDebug(logListWidgets) << "Adding selections - items count:" << items.size() << "record last:" << recordLastSelection;
    D_D(DSimpleListView);

    // Add item to selection list.
    d->selectionItems->append(items);

    // Record last selection item to make selected operation continuously.
    if (recordLastSelection && d->selectionItems->count() > 0) {
        qCDebug(logListWidgets) << "Recording last selection item";
        d->lastSelectItem = d->selectionItems->last();
    }
}

/*!
  \brief 移除所有选择项.

  \a clearLastSelection 是否清除最后选中的列表项.
 */
void DSimpleListView::clearSelections(bool clearLastSelection)
{
    qCDebug(logListWidgets) << "Clearing selections - clear last:" << clearLastSelection;
    D_D(DSimpleListView);

    // Clear selection list.
    d->selectionItems->clear();

    if (clearLastSelection) {
        qCDebug(logListWidgets) << "Clearing last selection item";
        d->lastSelectItem = NULL;
    }
}

/*!
  \brief 获取所有选择项.

  \return 返回选中的列表项.
 */
QList<DSimpleListItem*> DSimpleListView::getSelections()
{
    qCDebug(logListWidgets) << "Getting selections - count:" << d->selectionItems->size();
    D_D(DSimpleListView);

    return *d->selectionItems;
}

/*!
  \brief 刷新所有项.

  \a items 列表项.
 */
void DSimpleListView::refreshItems(QList<DSimpleListItem*> items)
{
    qCDebug(logListWidgets) << "Refreshing items - count:" << items.size();
    D_D(DSimpleListView);

    // Init.
    QList<DSimpleListItem*> *newSelectionItems = new QList<DSimpleListItem*>();
    DSimpleListItem *newLastSelectionItem = NULL;
    DSimpleListItem *newLastHoverItem = NULL;

    // Save selection items and last selection item.
    for (DSimpleListItem *item:items) {
        for (DSimpleListItem *selectionItem:*d->selectionItems) {
            if (item->sameAs(selectionItem)) {
                newSelectionItems->append(item);
                break;
            }
        }
    }

    if (d->lastSelectItem != NULL) {
        for (DSimpleListItem *item:items) {
            if (item->sameAs(d->lastSelectItem)) {
                newLastSelectionItem = item;
                break;
            }
        }
    }

    if (d->lastHoverItem != NULL) {
        for (DSimpleListItem *item:items) {
            if (item->sameAs(d->lastHoverItem)) {
                newLastHoverItem = item;
                break;
            }
        }
    }
    d->lastHoverItem = NULL;

    // Update items.
    clearItems();
    d->listItems->append(items);
    QList<DSimpleListItem*> searchItems = d->getSearchItems(items);
    d->renderItems->append(searchItems);

    // Sort once if default sort column hasn't init.
    if (d->defaultSortingColumn != -1) {
        d->sortItemsByColumn(d->defaultSortingColumn, d->defaultSortingOrder);
    }

    // Restore selection items and last selection item.
    clearSelections();
    addSelections(*newSelectionItems, false);
    d->lastSelectItem = newLastSelectionItem;
    d->lastHoverItem = newLastHoverItem;

    // Keep scroll position.
    d->renderOffset = adjustRenderOffset(d->renderOffset);

    // Render.
    repaint();
}

/*!
  \brief 搜索.

  \a content 被搜索内容.
 */
void DSimpleListView::search(QString content)
{
    qCDebug(logListWidgets) << "Searching with content:" << content;
    D_D(DSimpleListView);

    if (content == "" && d->searchContent != content) {
        qCDebug(logListWidgets) << "Clearing search, showing all items";
        d->searchContent = content;

        d->renderItems->clear();
        d->renderItems->append(*d->listItems);
        qCDebug(logListWidgets) << "Showing all" << d->listItems->size() << "items";
    } else {
        d->searchContent = content;

        QList<DSimpleListItem*> searchItems = d->getSearchItems(*d->listItems);
        d->renderItems->clear();
        d->renderItems->append(searchItems);
        qCDebug(logListWidgets) << "Search found" << searchItems.size() << "matching items";
    }

    qCDebug(logListWidgets) << "Repainting after search";
    repaint();
}

/*!
  \brief 设置单一选择.

  \a singleSelect 是否单一选择.
 */
void DSimpleListView::setSingleSelect(bool singleSelect)
{
    qCDebug(logListWidgets) << "Setting single select mode to:" << singleSelect;
    D_D(DSimpleListView);

    if (d->isSingleSelect == singleSelect) {
        qCDebug(logListWidgets) << "Single select mode unchanged, skipping update";
        return;
    }

    d->isSingleSelect = singleSelect;
    qCDebug(logListWidgets) << "Single select mode updated successfully";
}

/*!
  \brief 单击空白区域时保持选择项.

  \a keep 是否保持选中项.
 */
void DSimpleListView::keepSelectWhenClickBlank(bool keep)
{
    qCDebug(logListWidgets) << "Setting keep select when click blank to:" << keep;
    D_D(DSimpleListView);

    if (d->isKeepSelectWhenClickBlank == keep) {
        qCDebug(logListWidgets) << "Keep select setting unchanged, skipping update";
        return;
    }

    d->isKeepSelectWhenClickBlank = keep;
    qCDebug(logListWidgets) << "Keep select setting updated successfully";
}

/*!
  \brief 选择所有列表项.
 */
void DSimpleListView::selectAllItems()
{
    qCDebug(logListWidgets) << "Selecting all items";
    D_D(DSimpleListView);

    if (!d->isSingleSelect) {
        qCDebug(logListWidgets) << "Multi-select mode, proceeding with select all";
        // Record old render offset to control scrollbar whether display.
        d->oldRenderOffset = d->renderOffset;

        // Select all items.
        clearSelections();
        addSelections(*d->renderItems);
        qCDebug(logListWidgets) << "Selected" << d->renderItems->size() << "items";

        // Scroll to top.
        d->renderOffset = d->getTopRenderOffset();
        qCDebug(logListWidgets) << "Scrolled to top";

        // Repaint.
        repaint();
    } else {
        qCDebug(logListWidgets) << "Single select mode, cannot select all items";
    }
}

/*!
  \brief 选择第一项.
 */
void DSimpleListView::selectFirstItem()
{
    qCDebug(logListWidgets) << "Selecting first item";
    D_D(DSimpleListView);

    if (d->renderItems->isEmpty()) {
        qCDebug(logListWidgets) << "No items to select";
        return;
    }

    // Record old render offset to control scrollbar whether display.
    d->oldRenderOffset = d->renderOffset;

    // Select first item.
    clearSelections();

    QList<DSimpleListItem*> items = QList<DSimpleListItem*>();
    items << d->renderItems->first();
    addSelections(items);
    qCDebug(logListWidgets) << "First item selected";

    // Scroll to top.
    d->renderOffset = d->getTopRenderOffset();
    qCDebug(logListWidgets) << "Scrolled to top";

    // Repaint.
    repaint();
}

/*!
  \brief 选择最后一项.
 */
void DSimpleListView::selectLastItem()
{
    qCDebug(logListWidgets) << "Selecting last item";
    D_D(DSimpleListView);

    if (d->renderItems->isEmpty()) {
        qCDebug(logListWidgets) << "No items to select";
        return;
    }

    // Record old render offset to control scrollbar whether display.
    d->oldRenderOffset = d->renderOffset;

    // Select last item.
    clearSelections();

    QList<DSimpleListItem*> items = QList<DSimpleListItem*>();
    items << d->renderItems->last();
    addSelections(items);
    qCDebug(logListWidgets) << "Last item selected";

    // Scroll to bottom.
    d->renderOffset = getBottomRenderOffset();
    qCDebug(logListWidgets) << "Scrolled to bottom";

    // Repaint.
    repaint();
}

/*!
  \brief 移动到上一项.
 */
void DSimpleListView::selectPrevItem()
{
    qCDebug(logListWidgets) << "Selecting previous item";
    selectPrevItemWithOffset(1);
}

/*!
  \brief 移动到下一项.
 */
void DSimpleListView::selectNextItem()
{
    qCDebug(logListWidgets) << "Selecting next item";
    selectNextItemWithOffset(1);
}

/*!
  \brief 从当前项选择到下一页的项.
 */
void DSimpleListView::shiftSelectPageDown()
{
    D_D(DSimpleListView);

    if (!d->isSingleSelect) {
        shiftSelectNextItemWithOffset(getScrollAreaHeight() / d->rowHeight);
    }
}

/*!
  \brief 从当前项选择到上一页的项.
 */
void DSimpleListView::shiftSelectPageUp()
{
    D_D(DSimpleListView);

    if (!d->isSingleSelect) {
        shiftSelectPrevItemWithOffset(getScrollAreaHeight() / d->rowHeight);
    }
}

/*!
  \brief 从当前项选择到末尾项.
 */
void DSimpleListView::shiftSelectToEnd()
{
    D_D(DSimpleListView);

    if (!d->isSingleSelect) {
        // Select last item if nothing selected yet.
        if (d->selectionItems->empty()) {
            selectLastItem();
        }
        // Select items from last selected item to last item.
        else {
            // Found last selected index and do select operation.
            int lastSelectionIndex = d->renderItems->indexOf(d->lastSelectItem);
            shiftSelectItemsWithBound(lastSelectionIndex, d->renderItems->count() - 1);

            // Scroll to bottom.
            d->renderOffset = getBottomRenderOffset();

            // Repaint.
            repaint();
        }
    }

}

/*!
  \brief 从当前项选择到起始项.
 */
void DSimpleListView::shiftSelectToHome()
{
    D_D(DSimpleListView);

    if (!d->isSingleSelect) {
        // Select first item if nothing selected yet.
        if (d->selectionItems->empty()) {
            selectFirstItem();
        }
        // Select items from last selected item to first item.
        else {
            // Found last selected index and do select operation.
            int lastSelectionIndex = d->renderItems->indexOf(d->lastSelectItem);
            shiftSelectItemsWithBound(0, lastSelectionIndex);

            // Scroll to top.
            d->renderOffset = d->getTopRenderOffset();

            // Repaint.
            repaint();
        }
    }

}

/*!
  \brief 从当前项选择到下一项.
 */
void DSimpleListView::shiftSelectToNext()
{
    D_D(DSimpleListView);

    if (!d->isSingleSelect) {
        shiftSelectNextItemWithOffset(1);
    }
}

/*!
  \brief 从当前项选择到上一项.
 */
void DSimpleListView::shiftSelectToPrev()
{
    D_D(DSimpleListView);

    if (!d->isSingleSelect) {
        shiftSelectPrevItemWithOffset(1);
    }
}

/*!
  \brief 滚动到下一页.
 */
void DSimpleListView::scrollPageDown()
{
    D_D(DSimpleListView);

    selectNextItemWithOffset(getScrollAreaHeight() / d->rowHeight);
}

/*!
  \brief 滚动到上一页.
 */
void DSimpleListView::scrollPageUp()
{
    D_D(DSimpleListView);

    selectPrevItemWithOffset(getScrollAreaHeight() / d->rowHeight);
}

void DSimpleListView::ctrlScrollPageUp()
{
    qCDebug(logListWidgets) << "Ctrl scroll page up";
    D_D(DSimpleListView);

    int oldOffset = d->renderOffset;
    d->renderOffset = adjustRenderOffset(d->renderOffset - getScrollAreaHeight());
    qCDebug(logListWidgets) << "Render offset changed from" << oldOffset << "to" << d->renderOffset;

    repaint();
}

void DSimpleListView::ctrlScrollPageDown()
{
    qCDebug(logListWidgets) << "Ctrl scroll page down";
    D_D(DSimpleListView);

    int oldOffset = d->renderOffset;
    d->renderOffset = adjustRenderOffset(d->renderOffset + getScrollAreaHeight());
    qCDebug(logListWidgets) << "Render offset changed from" << oldOffset << "to" << d->renderOffset;

    repaint();
}

void DSimpleListView::ctrlScrollToHome()
{
    qCDebug(logListWidgets) << "Ctrl scroll to home";
    D_D(DSimpleListView);

    int oldOffset = d->renderOffset;
    d->renderOffset = d->getTopRenderOffset();
    qCDebug(logListWidgets) << "Render offset changed from" << oldOffset << "to" << d->renderOffset;

    repaint();
}

void DSimpleListView::ctrlScrollToEnd()
{
    qCDebug(logListWidgets) << "Ctrl scroll to end";
    D_D(DSimpleListView);

    int oldOffset = d->renderOffset;
    d->renderOffset = getBottomRenderOffset();
    qCDebug(logListWidgets) << "Render offset changed from" << oldOffset << "to" << d->renderOffset;

    repaint();
}

void DSimpleListView::leaveEvent(QEvent * event)
{
    qCDebug(logListWidgets) << "Mouse leave event";
    D_D(DSimpleListView);

    d->lastHoverItem.clear();
    d->drawHoverItem.clear();
    d->mouseHoverItem.clear();
    qCDebug(logListWidgets) << "Cleared hover items";

    hideScrollbar();

    QWidget::leaveEvent(event);
}

void DSimpleListView::hideScrollbar()
{
    qCDebug(logListWidgets) << "Hiding scrollbar";
    D_D(DSimpleListView);

    // Record old render offset to control scrollbar whether display.
    d->mouseAtScrollArea = false;
    d->oldRenderOffset = d->renderOffset;
    qCDebug(logListWidgets) << "Mouse at scroll area set to false";

    repaint();
}

bool DSimpleListView::eventFilter(QObject *obj, QEvent *event)
{
    qCDebug(logListWidgets) << "Event filter called with event type:" << event->type();
    Q_UNUSED(obj);
    Q_UNUSED(event);
    return false;
}

void DSimpleListView::keyPressEvent(QKeyEvent *keyEvent)
{
    qCDebug(logListWidgets) << "Key press event - key:" << keyEvent->key() << "modifiers:" << keyEvent->modifiers();
    
    if (keyEvent->key() == Qt::Key_Home) {
        if (keyEvent->modifiers() == Qt::ControlModifier) {
            qCDebug(logListWidgets) << "Ctrl+Home pressed - scrolling to home";
            ctrlScrollToHome();
        } else if (keyEvent->modifiers() == Qt::ShiftModifier) {
            qCDebug(logListWidgets) << "Shift+Home pressed - shift selecting to home";
            shiftSelectToHome();
        }else {
            qCDebug(logListWidgets) << "Home pressed - selecting first item";
            selectFirstItem();
        }
    } else if (keyEvent->key() == Qt::Key_End) {
        if (keyEvent->modifiers() == Qt::ControlModifier) {
            qCDebug(logListWidgets) << "Ctrl+End pressed - scrolling to end";
            ctrlScrollToEnd();
        } else if (keyEvent->modifiers() == Qt::ShiftModifier) {
            qCDebug(logListWidgets) << "Shift+End pressed - shift selecting to end";
            shiftSelectToEnd();
        } else {
            qCDebug(logListWidgets) << "End pressed - selecting last item";
            selectLastItem();
        }
    } else if (keyEvent->key() == Qt::Key_Up) {
        if (keyEvent->modifiers() == Qt::ShiftModifier) {
            qCDebug(logListWidgets) << "Shift+Up pressed - shift selecting previous";
            shiftSelectToPrev();
        } else {
            qCDebug(logListWidgets) << "Up pressed - selecting previous item";
            selectPrevItem();
        }
    } else if (keyEvent->key() == Qt::Key_Down) {
        if (keyEvent->modifiers() == Qt::ShiftModifier) {
            qCDebug(logListWidgets) << "Shift+Down pressed - shift selecting next";
            shiftSelectToNext();
        } else {
            qCDebug(logListWidgets) << "Down pressed - selecting next item";
            selectNextItem();
        }
    } else if (keyEvent->key() == Qt::Key_PageUp) {
        if (keyEvent->modifiers() == Qt::ControlModifier) {
            qCDebug(logListWidgets) << "Ctrl+PageUp pressed - scrolling page up";
            ctrlScrollPageUp();
        } else if (keyEvent->modifiers() == Qt::ShiftModifier) {
            qCDebug(logListWidgets) << "Shift+PageUp pressed - shift selecting page up";
            shiftSelectPageUp();
        } else {
            qCDebug(logListWidgets) << "PageUp pressed - scrolling page up";
            scrollPageUp();
        }
    } else if (keyEvent->key() == Qt::Key_PageDown) {
        if (keyEvent->modifiers() == Qt::ControlModifier) {
            qCDebug(logListWidgets) << "Ctrl+PageDown pressed - scrolling page down";
            ctrlScrollPageDown();
        } else if (keyEvent->modifiers() == Qt::ShiftModifier) {
            qCDebug(logListWidgets) << "Shift+PageDown pressed - shift selecting page down";
            shiftSelectPageDown();
        } else {
            qCDebug(logListWidgets) << "PageDown pressed - scrolling page down";
            scrollPageDown();
        }
    } else if (keyEvent->key() == Qt::Key_A) {
        if (keyEvent->modifiers() == Qt::ControlModifier) {
            qCDebug(logListWidgets) << "Ctrl+A pressed - selecting all items";
            selectAllItems();
        }
    } else {
        qCDebug(logListWidgets) << "Unhandled key press";
    }
}

void DSimpleListView::mouseMoveEvent(QMouseEvent *mouseEvent)
{
    qCDebug(logListWidgets) << "Mouse move event";
    D_D(DSimpleListView);

    // Scroll if mouse drag at scrollbar.
    if (d->mouseDragScrollbar) {
        qCDebug(logListWidgets) << "Dragging scrollbar";
        int barHeight = getScrollbarHeight();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        d->renderOffset = adjustRenderOffset((mouseEvent->y() - barHeight / 2 - d->titleHeight) / (getScrollAreaHeight() * 1.0) * d->getItemsTotalHeight());
#else
        d->renderOffset = adjustRenderOffset((mouseEvent->position().y() - barHeight / 2 - d->titleHeight) / (getScrollAreaHeight() * 1.0) * d->getItemsTotalHeight());
#endif
        repaint();
    }
    // Update scrollbar status with mouse position.
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    else if (isMouseAtScrollArea(mouseEvent->x()) != d->mouseAtScrollArea) {
        qCDebug(logListWidgets) << "Mouse scroll area status changed";
        d->mouseAtScrollArea = isMouseAtScrollArea(mouseEvent->x());
#else
    else if (isMouseAtScrollArea(mouseEvent->position().x()) != d->mouseAtScrollArea) {
        qCDebug(logListWidgets) << "Mouse scroll area status changed";
        d->mouseAtScrollArea = isMouseAtScrollArea(mouseEvent->position().x());
#endif
        repaint();
    }
    // Otherwise to check titlebar arrow status.
    else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        bool atTitleArea = isMouseAtTitleArea(mouseEvent->y());
#else
        bool atTitleArea = isMouseAtTitleArea(mouseEvent->position().y());
#endif

        if (atTitleArea) {
            int hoverColumn = -1;

            if (d->sortingAlgorithms->count() != 0 && d->sortingAlgorithms->count() == d->columnTitles.count() && d->sortingOrderes->count() == d->columnTitles.count()) {
                // Calculate title widths;
                QList<int> renderWidths = getRenderWidths();

                int columnCounter = 0;
                int columnRenderX = 0;
                for (int renderWidth:renderWidths) {
                  if (renderWidth > 0) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                        auto mx = mouseEvent->x();
#else
                        auto mx = mouseEvent->position().x();
#endif
                        if (mx > columnRenderX && mx < columnRenderX + renderWidth) {
                            hoverColumn = columnCounter;

                            break;
                        }

                        columnRenderX += renderWidth;
                    }

                    columnCounter++;
                }
            }

            if (hoverColumn != d->titleHoverColumn) {
                d->titleHoverColumn = hoverColumn;

                repaint();
            }
        } else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            int hoverItemIndex = (d->renderOffset + mouseEvent->y() - d->titleHeight) / d->rowHeight;
#else
            int hoverItemIndex = (d->renderOffset + mouseEvent->position().y() - d->titleHeight) / d->rowHeight;
#endif
            // NOTE: hoverItemIndex may be less than 0, we need check index here.
            if (hoverItemIndex >= 0 && hoverItemIndex < (*d->renderItems).length()) {
                DSimpleListItem *item = (*d->renderItems)[hoverItemIndex];

                QList<int> renderWidths = getRenderWidths();

                int columnCounter = 0;
                int columnRenderX = 0;
                for (int renderWidth:renderWidths) {
                    if (renderWidth > 0) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                        auto mx = mouseEvent->x();
#else
                        auto mx = mouseEvent->position().x();
#endif
                        if (mx > columnRenderX && mx < columnRenderX + renderWidth) {
                            break;
                        }

                        columnRenderX += renderWidth;
                    }

                    columnCounter++;
                }

                if (d->drawHoverItem == NULL || !item->sameAs(d->drawHoverItem)) {
                    d->drawHoverItem = item;

                    repaint();
                }

                // Emit mouseHoverChanged signal.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                auto point = QPoint{mouseEvent->x() - columnRenderX, d->renderOffset + mouseEvent->y() - hoverItemIndex * d->rowHeight};
                mouseHoverChanged(d->mouseHoverItem, item, columnCounter,point);
#else
                auto point = QPointF{mouseEvent->position().x() - columnRenderX,
                                         d->renderOffset + mouseEvent->position().y() - hoverItemIndex * d->rowHeight};
                mouseHoverChanged(d->mouseHoverItem, item, columnCounter,point.toPoint());
#endif
                d->mouseHoverItem = item;

                if (d->lastHoverItem == NULL || !item->sameAs(d->lastHoverItem) || columnCounter != d->lastHoverColumnIndex) {
                    d->lastHoverItem = item;
                    d->lastHoverColumnIndex = columnCounter;

                    changeHoverItem(this->mapToGlobal(mouseEvent->pos()), d->lastHoverItem, columnCounter);
                }
            }
        }
    }
}

void DSimpleListView::mousePressEvent(QMouseEvent *mouseEvent)
{
    qCDebug(logListWidgets) << "Mouse press event - button:" << mouseEvent->button();
    D_D(DSimpleListView);

    setFocus();
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    bool atTitleArea = isMouseAtTitleArea(mouseEvent->y());
    bool atScrollArea = isMouseAtScrollArea(mouseEvent->x());
#else
    bool atTitleArea = isMouseAtTitleArea(mouseEvent->position().y());
    bool atScrollArea = isMouseAtScrollArea(mouseEvent->position().x());
#endif
    qCDebug(logListWidgets) << "Mouse at title area:" << atTitleArea << "at scroll area:" << atScrollArea;

    // Sort items with column's sorting algorithms when click on title area.
    if (atTitleArea) {
        qCDebug(logListWidgets) << "Mouse press in title area";
        if (mouseEvent->button() == Qt::LeftButton) {
            qCDebug(logListWidgets) << "Left button pressed in title area";
            if (d->sortingAlgorithms->count() != 0 && d->sortingAlgorithms->count() == d->columnTitles.count() && d->sortingOrderes->count() == d->columnTitles.count()) {
                // Calculate title widths;
                QList<int> renderWidths = getRenderWidths();

                int columnCounter = 0;
                int columnRenderX = 0;
                for (int renderWidth:renderWidths) {
                    if (renderWidth > 0) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                        auto mx = mouseEvent->x();
#else
                        auto mx = mouseEvent->position().x();
#endif
                        if (mx > columnRenderX && mx < columnRenderX + renderWidth) {
                            // If switch other column, default order is from top to bottom.
                            if (columnCounter != d->defaultSortingColumn) {
                                (*d->sortingOrderes)[columnCounter] = true;
                            }
                            // If user click same column, just switch reverse order.
                            else {
                                (*d->sortingOrderes)[columnCounter] = !(*d->sortingOrderes)[columnCounter];
                            }

                            d->defaultSortingColumn = columnCounter;
                            d->defaultSortingOrder = (*d->sortingOrderes)[columnCounter];

                            changeSortingStatus(d->defaultSortingColumn, d->defaultSortingOrder);

                            d->sortItemsByColumn(columnCounter, (*d->sortingOrderes)[columnCounter]);

                            if (columnCounter != d->titlePressColumn) {
                                d->titlePressColumn = columnCounter;
                            }

                            repaint();
                            break;
                        }

                        columnRenderX += renderWidth;
                    }

                    columnCounter++;
                }
            }
        } else if (mouseEvent->button() == Qt::RightButton) {
            qCDebug(logListWidgets) << "Right button pressed in title area";
            if (columnVisibles.count() == d->columnTitles.count()) {
                qCDebug(logListWidgets) << "Creating column visibility menu";
                QMenu *menu = new QMenu();
                menu->setStyle(QStyleFactory::create("dlight"));

                for (int i = 0; i < columnVisibles.count(); i++) {
                    if (i != d->alwaysVisibleColumn) {
                        QAction *action = new QAction(menu);
                        action->setText(d->columnTitles[i]);

                        action->setCheckable(true);
                        action->setChecked(columnVisibles[i]);

                        connect(action, &QAction::triggered, this, [this, i] {
                            if (i>=columnVisibles.size()) return ;

                           columnVisibles[i] = !columnVisibles[i];

                           changeColumnVisible(i, columnVisibles[i], columnVisibles);

                           repaint();
                       });

                        menu->addAction(action);
                    }
                }

                menu->exec(this->mapToGlobal(mouseEvent->pos()));
            }
        }
    }
    // Scroll when click on scrollbar area.
    else if (atScrollArea) {
        qCDebug(logListWidgets) << "Mouse press in scroll area";
        int barHeight = getScrollbarHeight();
        int barY = getScrollbarY();

        // Flag mouseDragScrollbar when click on scrollbar.
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        auto my = mouseEvent->y();
#else
        auto my = mouseEvent->position().y();
#endif
        if (my > barY && my < barY + barHeight) {
            d->mouseDragScrollbar = true;
        }
        // Scroll if click out of scrollbar area.
        else {
            d->renderOffset = adjustRenderOffset((my - barHeight / 2 - d->titleHeight) / (getScrollAreaHeight() * 1.0) * d->getItemsTotalHeight());
            repaint();
        }
    }
    // Select items.
    else {
        qCDebug(logListWidgets) << "Mouse press in item area";
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        int pressItemIndex = (d->renderOffset + mouseEvent->y() - d->titleHeight) / d->rowHeight;
#else
        int pressItemIndex = (d->renderOffset + mouseEvent->position().y() - d->titleHeight) / d->rowHeight;
#endif

        if (pressItemIndex >= d->renderItems->count()) {
            if (!d->isKeepSelectWhenClickBlank) {
                clearSelections();
            }

            repaint();
        } else {
            if (mouseEvent->button() == Qt::LeftButton) {
                if (pressItemIndex < d->renderItems->count()) {
                    // Scattered selection of items when press ctrl modifier.
                    if (!d->isSingleSelect && mouseEvent->modifiers() == Qt::ControlModifier) {
                        DSimpleListItem *item = (*d->renderItems)[pressItemIndex];

                        if (d->selectionItems->contains(item)) {
                            d->selectionItems->removeOne(item);
                        } else {
                            QList<DSimpleListItem*> items = QList<DSimpleListItem*>();
                            items << item;
                            addSelections(items);
                        }
                    }
                    // Continuous selection of items when press shift modifier.
                    else if (!d->isSingleSelect && (mouseEvent->modifiers() == Qt::ShiftModifier) && !d->selectionItems->empty()) {
                        int lastSelectionIndex = d->renderItems->indexOf(d->lastSelectItem);
                        int selectionStartIndex = std::min(pressItemIndex, lastSelectionIndex);
                        int selectionEndIndex = std::max(pressItemIndex, lastSelectionIndex);

                        shiftSelectItemsWithBound(selectionStartIndex, selectionEndIndex);
                    }
                    // Just select item under mouse if user not press any modifier.
                    else {
                        clearSelections();

                        QList<DSimpleListItem*> items = QList<DSimpleListItem*>();
                        items << (*d->renderItems)[pressItemIndex];
                        addSelections(items);
                    }

                    // Emit mousePressChanged signal.
                    QList<int> renderWidths = getRenderWidths();
                    int columnCounter = 0;
                    int columnRenderX = 0;
                    for (int renderWidth:renderWidths) {
                        if (renderWidth > 0) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                            auto mx = mouseEvent->x();
#else
                            auto mx = mouseEvent->position().x();
#endif
                            if (mx > columnRenderX && mx < columnRenderX + renderWidth) {
                                break;
                            }

                            columnRenderX += renderWidth;
                        }

                        columnCounter++;
                    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                    auto point = QPoint(mouseEvent->x() - columnRenderX, d->renderOffset + mouseEvent->y() - pressItemIndex * d->rowHeight);
#else
                    auto point = QPoint(mouseEvent->position().x() - columnRenderX,
                                            d->renderOffset + mouseEvent->position().y() - pressItemIndex * d->rowHeight);
#endif
                    mousePressChanged((*d->renderItems)[pressItemIndex], columnCounter, point);

                    repaint();
                }
            } else if (mouseEvent->button() == Qt::RightButton) {
                DSimpleListItem *pressItem = (*d->renderItems)[pressItemIndex];
                bool pressInSelectionArea = false;

                for (DSimpleListItem *item : *d->selectionItems) {
                    if (item == pressItem) {
                        pressInSelectionArea = true;

                        break;
                    }
                }

                if (!pressInSelectionArea && pressItemIndex < d->renderItems->length()) {
                    clearSelections();

                    QList<DSimpleListItem*> items = QList<DSimpleListItem*>();
                    items << (*d->renderItems)[pressItemIndex];
                    addSelections(items);

                    repaint();
                }

                if (d->selectionItems->length() > 0) {
                    rightClickItems(this->mapToGlobal(mouseEvent->pos()), *d->selectionItems);
                }
            }
        }
    }
}

void DSimpleListView::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
    qCDebug(logListWidgets) << "Mouse release event";
    D_D(DSimpleListView);

    if (d->mouseDragScrollbar) {
        qCDebug(logListWidgets) << "Ending scrollbar drag";
        // Reset mouseDragScrollbar.
        d->mouseDragScrollbar = false;

        repaint();
    } else {
        if (d->titlePressColumn != -1) {
            qCDebug(logListWidgets) << "Ending title column press";
            d->titlePressColumn = -1;
            repaint();
        }
    }

    // Emit mouseReleaseChanged signal.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    int releaseItemIndex = (d->renderOffset + mouseEvent->y() - d->titleHeight) / d->rowHeight;
#else
    int releaseItemIndex = (d->renderOffset + mouseEvent->position().y() - d->titleHeight) / d->rowHeight;
#endif

    if (releaseItemIndex >= 0 && releaseItemIndex < (*d->renderItems).length()) {
        QList<int> renderWidths = getRenderWidths();
        int columnCounter = 0;
        int columnRenderX = 0;
        for (int renderWidth:renderWidths) {
            if (renderWidth > 0) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                auto mx = mouseEvent->x();
#else
                auto mx = mouseEvent->position().x();
#endif
                if (mx > columnRenderX && mx < columnRenderX + renderWidth) {
                    break;
                }

                columnRenderX += renderWidth;
            }

            columnCounter++;
        }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        auto point = QPoint{mouseEvent->x() - columnRenderX, d->renderOffset + mouseEvent->y() - releaseItemIndex * d->rowHeight};
        mouseReleaseChanged((*d->renderItems)[releaseItemIndex], columnCounter, point);
#else
        auto point = QPointF{mouseEvent->position().x() - columnRenderX,
                                   d->renderOffset + mouseEvent->position().y() - releaseItemIndex * d->rowHeight};
        mouseReleaseChanged((*d->renderItems)[releaseItemIndex], columnCounter, point.toPoint());
#endif
    }
}

void DSimpleListView::wheelEvent(QWheelEvent *event)
{
    qCDebug(logListWidgets) << "Wheel event received";
    D_D(DSimpleListView);
    auto delta = event->angleDelta();
    if (delta.y() != 0) {
        qCDebug(logListWidgets) << "Vertical wheel scroll - delta:" << delta.y();
        // Record old render offset to control scrollbar whether display.
        d->oldRenderOffset = d->renderOffset;

        qreal scrollStep = delta.y() / 120.0;
        int oldOffset = d->renderOffset;
        d->renderOffset = adjustRenderOffset(d->renderOffset - scrollStep * d->scrollUnit);
        qCDebug(logListWidgets) << "Scroll step:" << scrollStep << "offset changed from" << oldOffset << "to" << d->renderOffset;

        repaint();
    } else {
        qCDebug(logListWidgets) << "No vertical scroll delta, ignoring wheel event";
    }

    event->accept();
}

void DSimpleListView::paintEvent(QPaintEvent *)
{
    qCDebug(logListWidgets) << "Paint event triggered";
    D_D(DSimpleListView);

    // Init.
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Calculate title widths;
    QList<int> renderWidths = getRenderWidths();

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setOpacity(0.05);

    int penWidth = 1;
    QPainterPath framePath;
    framePath.addRoundedRect(QRect(rect().x() + penWidth, rect().y() + penWidth, rect().width() - penWidth * 2, rect().height() - penWidth * 2), d->clipRadius, d->clipRadius);
    painter.setClipPath(framePath);

    // Draw title.
    if (d->titleHeight > 0) {
        QPainterPath titlePath;
        titlePath.addRect(QRectF(rect().x(), rect().y(), rect().width(), d->titleHeight));
        painter.setOpacity(titleAreaOpacity);
        painter.fillPath(titlePath, QColor(titleAreaColor));
    }

    int renderY = 0;
    int renderHeight = 0;
    if (d->titleHeight > 0) {
        int columnCounter = 0;
        int columnRenderX = 0;
        for (int renderWidth:renderWidths) {
            if (renderWidth > 0) {
                painter.setOpacity(1);

                QFont font = painter.font() ;
                font.setPointSize(titleSize);
                painter.setFont(font);

                painter.setPen(QPen(QColor(titleColor)));
                painter.drawText(QRect(columnRenderX + d->titlePadding, 0, renderWidth, d->titleHeight), Qt::AlignVCenter | Qt::AlignLeft, d->columnTitles[columnCounter]);

                columnRenderX += renderWidth;

                if (columnCounter < renderWidths.size() - 1) {
                    painter.setOpacity(0.05);
                    QPainterPath separatorPath;
                    separatorPath.addRect(QRectF(rect().x() + columnRenderX - 1, rect().y() + 4, 1, d->titleHeight - 8));
                    painter.fillPath(separatorPath, QColor(titleLineColor));
                }

                // Draw sort arrow.
                if (d->defaultSortingColumn == columnCounter) {
                    painter.setOpacity(1);
                    int arrowX = rect().x() + columnRenderX - d->titleArrowPadding - arrowUpNormalImage.width() / arrowUpNormalImage.devicePixelRatio();
                    int arrowY = rect().y() + (d->titleHeight - arrowDownNormalImage.height() / arrowUpNormalImage.devicePixelRatio()) / 2;

                    if (d->defaultSortingOrder) {
                        if (d->titlePressColumn == d->defaultSortingColumn) {
                            painter.drawPixmap(QPoint(arrowX, arrowY), arrowDownPressImage);
                        } else if (d->titleHoverColumn == d->defaultSortingColumn) {
                            painter.drawPixmap(QPoint(arrowX, arrowY), arrowDownHoverImage);
                        } else {
                            painter.drawPixmap(QPoint(arrowX, arrowY), arrowDownNormalImage);
                        }
                    } else {
                        if (d->titlePressColumn == d->defaultSortingColumn) {
                            painter.drawPixmap(QPoint(arrowX, arrowY), arrowUpPressImage);
                        } else if (d->titleHoverColumn == d->defaultSortingColumn) {
                            painter.drawPixmap(QPoint(arrowX, arrowY), arrowUpHoverImage);
                        } else {
                            painter.drawPixmap(QPoint(arrowX, arrowY), arrowUpNormalImage);
                        }
                    }
                }
            }
            columnCounter++;
        }

        renderY += d->titleHeight;
        renderHeight += d->titleHeight;
    }

    // Draw background.
    painter.setOpacity(backgroundOpacity);
    QPainterPath backgroundPath;
    backgroundPath.addRect(QRectF(rect().x(), rect().y() + d->titleHeight, rect().width(), rect().height() - d->titleHeight));
    painter.fillPath(backgroundPath, QColor(backgroundColor));

    // Draw context.
    QPainterPath scrollAreaPath;
    scrollAreaPath.addRect(QRectF(rect().x(), rect().y() + d->titleHeight, rect().width(), getScrollAreaHeight()));

    int rowCounter = 0;
    for (DSimpleListItem *item:*d->renderItems) {
        if (rowCounter >= d->renderOffset / d->rowHeight) {
            // Clip item rect.
            QPainterPath itemPath;
            itemPath.addRect(QRect(0, renderY + rowCounter * d->rowHeight - d->renderOffset, rect().width(), d->rowHeight));
            painter.setClipPath((framePath.intersected(scrollAreaPath)).intersected(itemPath));

            // Draw item backround.
            bool isSelect = d->selectionItems->contains(item);
            bool isHover = d->drawHoverItem != NULL && item->sameAs(d->drawHoverItem);
            painter.save();
            item->drawBackground(QRect(0, renderY + rowCounter * d->rowHeight - d->renderOffset, rect().width(), d->rowHeight),
                                 &painter,
                                 rowCounter,
                                 isSelect,
                                 isHover);
            painter.restore();

            // Draw item foreground.
            int columnCounter = 0;
            int columnRenderX = 0;
            for (int renderWidth:renderWidths) {
                if (renderWidth > 0) {
                    painter.save();
                    item->drawForeground(QRect(columnRenderX, renderY + rowCounter * d->rowHeight - d->renderOffset, renderWidth, d->rowHeight),
                                         &painter,
                                         columnCounter,
                                         rowCounter,
                                         isSelect,
                                         isHover);
                    painter.restore();

                    columnRenderX += renderWidth;
                }
                columnCounter++;
            }

            renderHeight += d->rowHeight;

            if (renderHeight > rect().height()) {
                break;
            }
        }

        rowCounter++;
    }

    // Keep clip area.
    painter.setClipPath(framePath);

    // Draw search tooltip.
    if (d->searchContent != "" && d->renderItems->size() == 0) {
        painter.setOpacity(1);
        painter.setPen(QPen(QColor(searchColor)));

        QFont font = painter.font() ;
        font.setPointSize(20);
        painter.setFont(font);

        painter.drawText(QRect(rect().x(), rect().y() + d->titleHeight, rect().width(), rect().height() - d->titleHeight), Qt::AlignCenter, QObject::tr("No search result"));
    }

    // Draw frame.
    if (d->drawFrame) {
        QPen framePen;
        framePen.setColor(frameColor);
        painter.setOpacity(frameOpacity);
        painter.drawPath(framePath);
    }

    // Draw scrollbar.
    if (d->mouseAtScrollArea) {
        paintScrollbar(&painter);
    } else if (d->oldRenderOffset != d->renderOffset) {
        paintScrollbar(&painter);

        startScrollbarHideTimer();
    }
}

void DSimpleListView::paintScrollbar(QPainter *painter)
{
    qCDebug(logListWidgets) << "Painting scrollbar";
    D_D(DSimpleListView);

    if (d->getItemsTotalHeight() > getScrollAreaHeight()) {
        qCDebug(logListWidgets) << "Items exceed scroll area, drawing scrollbar";
        // Init scrollbar opacity with scrollbar status.
        qreal barOpacitry = 0;
        qreal barFrameOpacitry = 0;

        // Press.
        if (d->mouseDragScrollbar) {
            qCDebug(logListWidgets) << "Scrollbar in press state";
            barOpacitry = scrollbarPressOpacity;
            barFrameOpacitry = scrollbarFramePressOpacity;
        } else {
            // Hover.
            if (d->mouseAtScrollArea) {
                qCDebug(logListWidgets) << "Scrollbar in hover state";
                barOpacitry = scrollbarHoverOpacity;
                barFrameOpacitry = scrollbarFrameHoverOpacity;
            }
            // Normal.
            else {
                qCDebug(logListWidgets) << "Scrollbar in normal state";
                barOpacitry = scrollbarNormalOpacity;
                barFrameOpacitry = scrollbarFrameNormalOpacity;
            }
        }

        int barWidth = (d->mouseAtScrollArea || d->mouseDragScrollbar) ? d->scrollbarDragWidth : d->scrollbarDefaultWidth;
        int barRadius = 4;

        int barY = getScrollbarY();
        int barHeight = getScrollbarHeight();

        painter->setOpacity(barOpacitry);
        QPainterPath path;
        path.addRoundedRect(
            QRectF(rect().x() + rect().width() - barWidth - d->scrollbarPadding,
                   barY + barRadius,
                   barWidth,
                   barHeight - barRadius * 2), barRadius, barRadius);
        painter->fillPath(path, QColor(scrollbarColor));

        QPen pen;
        pen.setColor(QColor(scrollbarColor));
        pen.setWidth(1);
        painter->setOpacity(barFrameOpacitry);
        painter->setPen(pen);
        painter->drawPath(path);
    }
}

void DSimpleListView::selectNextItemWithOffset(int scrollOffset)
{
    qCDebug(logListWidgets) << "Selecting next item with offset:" << scrollOffset;
    D_D(DSimpleListView);

    // Record old render offset to control scrollbar whether display.
    d->oldRenderOffset = d->renderOffset;

    if (d->selectionItems->empty()) {
        qCDebug(logListWidgets) << "No items selected, selecting first item";
        selectFirstItem();
    } else {
        int lastIndex = 0;
        for (DSimpleListItem *item:*d->selectionItems) {
            int index = d->renderItems->indexOf(item);
            if (index > lastIndex) {
                lastIndex = index;
            }
        }

        if (lastIndex != -1) {
            int newIndex = std::min<qsizetype>(d->renderItems->count() - 1, lastIndex + scrollOffset);
            qCDebug(logListWidgets) << "Moving selection from index" << lastIndex << "to" << newIndex;
            lastIndex = newIndex;

            clearSelections(false);

            QList<DSimpleListItem*> items = QList<DSimpleListItem*>();
            items << (*d->renderItems)[lastIndex];

            addSelections(items);

            int itemIndex = lastIndex + 1;
            int itemOffset = adjustRenderOffset(itemIndex * d->rowHeight - rect().height() + d->titleHeight);
            if (((d->renderOffset + getScrollAreaHeight()) / d->rowHeight) < itemIndex) {
                qCDebug(logListWidgets) << "Adjusting render offset for visibility";
                d->renderOffset = itemOffset;
            }

            repaint();
        } else {
            qCDebug(logListWidgets) << "Invalid last index, skipping selection";
        }
    }
}

void DSimpleListView::selectPrevItemWithOffset(int scrollOffset)
{
    qCDebug(logListWidgets) << "Selecting previous item with offset:" << scrollOffset;
    D_D(DSimpleListView);

    // Record old render offset to control scrollbar whether display.
    d->oldRenderOffset = d->renderOffset;

    if (d->selectionItems->empty()) {
        qCDebug(logListWidgets) << "No items selected, selecting first item";
        selectFirstItem();
    } else {
        int firstIndex = d->renderItems->count();
        for (DSimpleListItem *item:*d->selectionItems) {
            int index = d->renderItems->indexOf(item);
            if (index < firstIndex) {
                firstIndex = index;
            }
        }

        if (firstIndex != -1) {
            int newIndex = std::max(0, firstIndex - scrollOffset);
            qCDebug(logListWidgets) << "Moving selection from index" << firstIndex << "to" << newIndex;
            firstIndex = newIndex;

            clearSelections();

            QList<DSimpleListItem*> items = QList<DSimpleListItem*>();
            items << (*d->renderItems)[firstIndex];

            addSelections(items);

            int itemIndex = firstIndex - 1;
            int itemOffset = adjustRenderOffset(itemIndex * d->rowHeight + d->titleHeight);
            if ((d->renderOffset / d->rowHeight) > itemIndex) {
                qCDebug(logListWidgets) << "Adjusting render offset for visibility";
                d->renderOffset = itemOffset;
            }

            repaint();
        } else {
            qCDebug(logListWidgets) << "Invalid first index, skipping selection";
        }
    }
}

void DSimpleListView::shiftSelectItemsWithBound(int selectionStartIndex, int selectionEndIndex)
{
    qCDebug(logListWidgets) << "Shift selecting items from index" << selectionStartIndex << "to" << selectionEndIndex;
    D_D(DSimpleListView);

    // Note: Shift operation always selection bound from last selection index to current index.
    // So we don't need *clear* lastSelectionIndex for keep shift + button is right logic.
    clearSelections(false);
    QList<DSimpleListItem*> items = QList<DSimpleListItem*>();
    int index = 0;
    for (DSimpleListItem *item:*d->renderItems) {
        if (index >= selectionStartIndex && index <= selectionEndIndex) {
            items << item;
        }

        index++;
    }

    qCDebug(logListWidgets) << "Selected" << items.size() << "items in range";
    // Note: Shift operation always selection bound from last selection index to current index.
    // So we don't need *record* lastSelectionIndex for keep shift + button is right logic.
    addSelections(items, false);
}

void DSimpleListView::shiftSelectPrevItemWithOffset(int scrollOffset)
{
    qCDebug(logListWidgets) << "Shift selecting previous items with offset:" << scrollOffset;
    D_D(DSimpleListView);

    // Record old render offset to control scrollbar whether display.
    d->oldRenderOffset = d->renderOffset;

    if (d->selectionItems->empty()) {
        qCDebug(logListWidgets) << "No items selected, selecting first item";
        selectFirstItem();
    } else {
        int firstIndex = d->renderItems->count();
        int lastIndex = 0;
        for (DSimpleListItem *item:*d->selectionItems) {
            int index = d->renderItems->indexOf(item);

            if (index < firstIndex) {
                firstIndex = index;
            }

            if (index > lastIndex) {
                lastIndex = index;
            }
        }

        if (firstIndex != -1) {
            int lastSelectionIndex = d->renderItems->indexOf(d->lastSelectItem);
            int selectionStartIndex, selectionEndIndex;

            if (lastIndex == lastSelectionIndex) {
                selectionStartIndex = std::max(0, firstIndex - scrollOffset);
                selectionEndIndex = lastSelectionIndex;
                qCDebug(logListWidgets) << "Extending selection upward from" << selectionStartIndex << "to" << selectionEndIndex;
            } else {
                selectionStartIndex = firstIndex;
                selectionEndIndex = std::max(0, lastIndex - scrollOffset);
                qCDebug(logListWidgets) << "Contracting selection from" << selectionStartIndex << "to" << selectionEndIndex;
            }

            shiftSelectItemsWithBound(selectionStartIndex, selectionEndIndex);

            if (d->renderOffset / d->rowHeight >= selectionStartIndex) {
                qCDebug(logListWidgets) << "Adjusting render offset for visibility";
                d->renderOffset = adjustRenderOffset((selectionStartIndex - 1) * d->rowHeight + d->titleHeight);
            }

            repaint();
        } else {
            qCDebug(logListWidgets) << "Invalid first index, skipping selection";
        }
    }
}

void DSimpleListView::shiftSelectNextItemWithOffset(int scrollOffset)
{
    qCDebug(logListWidgets) << "Shift selecting next items with offset:" << scrollOffset;
    D_D(DSimpleListView);

    // Record old render offset to control scrollbar whether display.
    d->oldRenderOffset = d->renderOffset;

    if (d->selectionItems->empty()) {
        qCDebug(logListWidgets) << "No items selected, selecting first item";
        selectFirstItem();
    } else {
        int firstIndex = d->renderItems->count();
        int lastIndex = 0;
        for (DSimpleListItem *item:*d->selectionItems) {
            int index = d->renderItems->indexOf(item);

            if (index < firstIndex) {
                firstIndex = index;
            }

            if (index > lastIndex) {
                lastIndex = index;
            }
        }

        if (firstIndex != -1) {
            int lastSelectionIndex = d->renderItems->indexOf(d->lastSelectItem);
            int selectionStartIndex, selectionEndIndex;

            if (firstIndex == lastSelectionIndex) {
                selectionStartIndex = firstIndex;
                selectionEndIndex = std::min<qsizetype>(d->renderItems->count() - 1, lastIndex + scrollOffset);
                qCDebug(logListWidgets) << "Extending selection downward from" << selectionStartIndex << "to" << selectionEndIndex;
            } else {
                selectionStartIndex = std::min<qsizetype>(d->renderItems->count() - 1, firstIndex + scrollOffset);
                selectionEndIndex = lastIndex;
                qCDebug(logListWidgets) << "Contracting selection from" << selectionStartIndex << "to" << selectionEndIndex;
            }

            shiftSelectItemsWithBound(selectionStartIndex, selectionEndIndex);

            if ((d->renderOffset + rect().height()) / d->rowHeight <= selectionEndIndex + 1) {
                qCDebug(logListWidgets) << "Adjusting render offset for visibility";
                d->renderOffset = adjustRenderOffset((selectionEndIndex + 1) * d->rowHeight + d->titleHeight - rect().height());
            }


            repaint();
        } else {
            qCDebug(logListWidgets) << "Invalid first index, skipping selection";
        }
    }
}

QList<int> DSimpleListView::getRenderWidths()
{
    D_D(DSimpleListView);

    QList<int> renderWidths;
    if (d->columnWidths.length() > 0) {
        qCDebug(logListWidgets) << "Calculating render widths for" << d->columnWidths.length() << "columns";
        if (d->columnWidths.contains(-1)) {
            qCDebug(logListWidgets) << "Auto-width column detected";
            for (int i = 0; i < d->columnWidths.count(); i++) {
                if (d->columnWidths[i] != -1) {
                    if (columnVisibles.value(i)) {
                        renderWidths << d->columnWidths[i];
                    } else {
                        renderWidths << 0;
                    }
                } else {
                    if (columnVisibles.value(i)) {
                        int totalWidthOfOtherColumns = 0;

                        for (int j = 0; j < d->columnWidths.count(); j++) {
                            if (d->columnWidths[j] != -1 && columnVisibles.value(j)) {
                                totalWidthOfOtherColumns += d->columnWidths[j];
                            }
                        }

                        const auto& autoWidth = rect().width() - totalWidthOfOtherColumns;
                        qCDebug(logListWidgets) << "Auto-width column" << i << "calculated width:" << autoWidth;
                        renderWidths << autoWidth;
                    } else {
                        renderWidths << 0;
                    }
                }
            }
        } else {
            qCDebug(logListWidgets) << "Using fixed column widths";
            for (int i = 0; i < d->columnWidths.count(); i++) {
                if (columnVisibles.value(i)) {
                    renderWidths << d->columnWidths[i];
                } else {
                    renderWidths << 0;
                }
            }
        }
    }
    // Return widget width if user don't set column withs throught function 'setColumnTitleInfo'.
    // Avoid listview don't draw item's foregound cause by emptry 'columnWidths'.
    else {
        qCDebug(logListWidgets) << "No column widths set, using full widget width";
        renderWidths << rect().width();
    }

    qCDebug(logListWidgets) << "Final render widths:" << renderWidths;
    return renderWidths;
}

bool DSimpleListView::isMouseAtScrollArea(int x)
{
    D_D(DSimpleListView);

    const auto& result = (x > rect().x() + rect().width() - d->scrollbarDragWidth) && (x < rect().x() + rect().width());
    qCDebug(logListWidgets) << "Check mouse at scroll area - x:" << x << "result:" << result;
    return result;
}

bool DSimpleListView::isMouseAtTitleArea(int y)
{
    D_D(DSimpleListView);

    const auto& result = (y > rect().y() && y < rect().y() + d->titleHeight);
    qCDebug(logListWidgets) << "Check mouse at title area - y:" << y << "result:" << result;
    return result;
}

int DSimpleListView::adjustRenderOffset(int offset)
{
    const auto& result = std::max(0, std::min(offset, getBottomRenderOffset()));
    qCDebug(logListWidgets) << "Adjusting render offset from" << offset << "to" << result;
    return result;
}

int DSimpleListViewPrivate::getItemsTotalHeight()
{
    const auto& result = renderItems->count() * rowHeight;
    qCDebug(logListWidgets) << "Getting items total height:" << result << "(items:" << renderItems->count() << "row height:" << rowHeight << ")";
    return result;
}

int DSimpleListView::getScrollAreaHeight()
{
    D_D(DSimpleListView);

    const auto& result = rect().height() - d->titleHeight;
    qCDebug(logListWidgets) << "Getting scroll area height:" << result;
    return result;
}

int DSimpleListView::getScrollbarY()
{
    D_D(DSimpleListView);

    int y = static_cast<int>((d->renderOffset / (d->getItemsTotalHeight() * 1.0)) * getScrollAreaHeight() + d->titleHeight);
    const auto& result = qMin(y, rect().height() - getScrollbarHeight());
    qCDebug(logListWidgets) << "Getting scrollbar Y position:" << result << "(calculated:" << y << ")";
    return result;
}

int DSimpleListView::getScrollbarHeight()
{
    D_D(DSimpleListView);

    const auto& result = std::max(static_cast<int>(getScrollAreaHeight() / (d->getItemsTotalHeight() * 1.0) * rect().height()), d->scrollbarMinHeight);
    qCDebug(logListWidgets) << "Getting scrollbar height:" << result << "(min height:" << d->scrollbarMinHeight << ")";
    return result;
}

int DSimpleListViewPrivate::getTopRenderOffset()
{
    qCDebug(logListWidgets) << "Getting top render offset: 0";
    return 0;
}

QList<DSimpleListItem*> DSimpleListViewPrivate::getSearchItems(QList<DSimpleListItem*> items)
{
    qCDebug(logListWidgets) << "Getting search items - input count:" << items.size() << "search content:" << searchContent;
    if (searchContent == "" || searchAlgorithm == NULL) {
        qCDebug(logListWidgets) << "No search filter applied, returning all items";
        return items;
    } else {
        QList<DSimpleListItem*> *searchItems = new QList<DSimpleListItem*>();

        for (DSimpleListItem *item : items) {
            if (searchAlgorithm(item, searchContent)) {
                searchItems->append(item);
            }
        }

        qCDebug(logListWidgets) << "Search filter applied, found" << searchItems->size() << "matching items";
        return *searchItems;
    }
}

int DSimpleListView::getBottomRenderOffset()
{
    D_D(DSimpleListView);

    int itemsHeight = d->getItemsTotalHeight();
    if (itemsHeight > rect().height() - d->titleHeight) {
        const auto& result = d->getItemsTotalHeight() - rect().height() + d->titleHeight;
        qCDebug(logListWidgets) << "Getting bottom render offset:" << result;
        return result;
    } else {
        qCDebug(logListWidgets) << "Getting bottom render offset: 0 (items fit in view)";
        return 0;
    }
}

void DSimpleListViewPrivate::sortItemsByColumn(int column, bool descendingSort)
{
    qCDebug(logListWidgets) << "Sorting items by column:" << column << "descending:" << descendingSort;
    if (sortingAlgorithms->count() != 0 && sortingAlgorithms->count() == columnTitles.count() && sortingOrderes->count() == columnTitles.count()) {
        qCDebug(logListWidgets) << "Applying sort algorithm to" << renderItems->count() << "items";
        std::sort(renderItems->begin(), renderItems->end(), [&](const DSimpleListItem *item1, const DSimpleListItem *item2) {
                return (*sortingAlgorithms)[column](item1, item2, descendingSort);
            });
        qCDebug(logListWidgets) << "Sort completed";
    } else {
        qCDebug(logListWidgets) << "Sort skipped - algorithm count mismatch";
    }
}

void DSimpleListView::startScrollbarHideTimer()
{
    qCDebug(logListWidgets) << "Starting scrollbar hide timer";
    D_D(DSimpleListView);

    if (d->hideScrollbarTimer != NULL) {
        qCDebug(logListWidgets) << "Stopping existing hide timer";
        d->hideScrollbarTimer->stop();
    }

    d->hideScrollbarTimer = new QTimer();
    connect(d->hideScrollbarTimer, SIGNAL(timeout()), this, SLOT(hideScrollbar()));
    d->hideScrollbarTimer->start(d->hideScrollbarDuration);
    qCDebug(logListWidgets) << "Hide timer started with duration:" << d->hideScrollbarDuration;
}

DWIDGET_END_NAMESPACE
