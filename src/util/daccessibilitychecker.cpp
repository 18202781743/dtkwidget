// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later


#include "daccessibilitychecker.h"

#include <DObjectPrivate>

#include <QDebug>
#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QAccessible>
#include <QAccessibleTableCellInterface>
#include <QApplication>
#include <QTimer>
#include <QLoggingCategory>

DWIDGET_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(logUtilClasses)

class DAccessibilityCheckerPrivate : public DCORE_NAMESPACE::DObjectPrivate
{
    D_DECLARE_PUBLIC(DAccessibilityChecker)
public:
    DAccessibilityCheckerPrivate(DAccessibilityChecker *qq);
    bool check();

    void checkWidgetName();
    void checkViewItemName();
    bool isIgnore(DAccessibilityChecker::Role role, const QWidget *w);

private:
    bool checkViewItemNameFromAccessibleInteface(QAbstractItemView *listview);
    bool checkViewItemNameFromAccessibleText(QAbstractItemView *listview);

    bool isDefaultIgnoreWidget(const QWidget *w) const;
    bool isDefaultIgnoreView(const QAbstractItemView *view) const;

    QString widgetOutputLog(const QWidget *w) const;
    QString widgetInfoString(const QWidget *w) const;
    QString viewItemOutputLog(int rowIndex, int columnIndex, const QAbstractItemView *absView, const QString &itemText = QString()) const;

    void formatCheckResult();
    void printSummaryResults();
    void printRoleWarningOutput(const QString &roleString, const QStringList &roleList);
    void _q_checkTimeout();

private:
    QWidgetList topLevelWidgets;
    QStringList widgetsWarningList;
    QStringList itemWariningList;
    DAccessibilityChecker::OutputFormat outputFormat;
    QTimer *checkTimer;
    int widgetIgnoredCount;
};

/*! \internal */
DAccessibilityCheckerPrivate::DAccessibilityCheckerPrivate(DAccessibilityChecker *qq)
    : DObjectPrivate(qq)
    , topLevelWidgets(qApp->topLevelWidgets())
    , widgetsWarningList()
    , itemWariningList()
    , outputFormat(DAccessibilityChecker::AssertFormat)
    , checkTimer(nullptr)
    , widgetIgnoredCount(0)
{
    qCDebug(logUtilClasses) << "Creating DAccessibilityCheckerPrivate with" << topLevelWidgets.count() << "top level widgets";
}

/*! \internal */
bool DAccessibilityCheckerPrivate::check()
{
    qCDebug(logUtilClasses) << "Starting accessibility check";
    if (topLevelWidgets.isEmpty()) {
        qCWarning(logUtilClasses) << "Found nothing about qApp top level widgets";
        return true;
    }

    checkWidgetName();
    checkViewItemName();

    if (outputFormat == DAccessibilityChecker::FullFormat) {
        qCDebug(logUtilClasses) << "Formatting check results";
        formatCheckResult();
    }

    printSummaryResults();
    const auto& hasWarnings = widgetsWarningList.isEmpty() && itemWariningList.isEmpty();
    qCDebug(logUtilClasses) << "Accessibility check completed, has warnings:" << !hasWarnings;
    if (hasWarnings)
        return true;

    return false;
}

/*! \internal */
void DAccessibilityCheckerPrivate::checkWidgetName()
{
    qCDebug(logUtilClasses) << "Checking widget names";
    D_Q(DAccessibilityChecker);

    QWidgetList childrenList(topLevelWidgets);
    for (const QWidget *topLevelWidget : topLevelWidgets)
        childrenList.append(topLevelWidget->findChildren<QWidget *>());

    for (auto child : std::as_const(childrenList)) {
        if (q->isIgnore(DAccessibilityChecker::Widget, child)) {
            widgetIgnoredCount++;
            qCDebug(logUtilClasses) << "Ignoring widget:" << child;
            continue;
        }
            continue;
        }

        const QAccessibleInterface *interface = QAccessible::queryAccessibleInterface(child);
        bool hasNoText;
        if (interface && interface->isValid()) {
            hasNoText = interface->text(QAccessible::Name).isEmpty();
        } else {
            hasNoText = child->accessibleName().isEmpty();
        }

        switch (outputFormat) {
        case DAccessibilityChecker::AssertFormat: {
            Q_ASSERT_X(hasNoText, "Widget Accessible Name Check", widgetOutputLog(child).toLocal8Bit());
        }
            break;
        case DAccessibilityChecker::FullFormat: {
            if (hasNoText)
                widgetsWarningList.append(widgetOutputLog(child));
        }
            break;
        default:
            break;
        }
    }
}

/*! \internal */
void DAccessibilityCheckerPrivate::checkViewItemName()
{
    qCDebug(logUtilClasses) << "Checking view item names";
    D_Q(DAccessibilityChecker);

    QList<QAbstractItemView *> listViewList;
    for (const QWidget *topLevelWidget : topLevelWidgets)
        listViewList.append(topLevelWidget->findChildren<QAbstractItemView *>());

    qCDebug(logUtilClasses) << "Found" << listViewList.size() << "views to check";
    for (auto absListView : std::as_const(listViewList)) {
        if (q->isIgnore(DAccessibilityChecker::ViewItem, absListView)) {
            qCDebug(logUtilClasses) << "Ignoring view:" << absListView;
            continue;
        }

        if (!checkViewItemNameFromAccessibleInteface(absListView)) {
            qCDebug(logUtilClasses) << "Checking view item from accessible text";
            checkViewItemNameFromAccessibleText(absListView);
        }
    }
}

/*! \internal */
bool DAccessibilityCheckerPrivate::isIgnore(DAccessibilityChecker::Role role, const QWidget *w)
{
    qCDebug(logUtilClasses) << "Checking if widget should be ignored, role:" << static_cast<int>(role);
    switch (role) {
    case DAccessibilityChecker::Widget: {
        const auto& result = isDefaultIgnoreWidget(w);
        qCDebug(logUtilClasses) << "Widget ignore result:" << result;
        return result;
    }
    case DAccessibilityChecker::ViewItem: {
        if (const QAbstractItemView *view = qobject_cast<const QAbstractItemView *>(w)) {
            const auto& result = isDefaultIgnoreView(view);
            qCDebug(logUtilClasses) << "View ignore result:" << result;
            return result;
        }

        return false;
    }
    default:
        return false;
    }
}

/*! \internal */
bool DAccessibilityCheckerPrivate::isDefaultIgnoreView(const QAbstractItemView *view) const
{
    qCDebug(logUtilClasses) << "Checking default ignore view";
    static QByteArrayList defaultIgnoredView = {
        QByteArrayLiteral("QColumnView"),
        QByteArrayLiteral("QHeaderView")
    };

    const auto& result = std::any_of(defaultIgnoredView.begin(), defaultIgnoredView.end(), [view](const QByteArray &ignoreClass) -> bool { return view->inherits(ignoreClass); });
    qCDebug(logUtilClasses) << "Default ignore view result:" << result;
    return result;
}

/*!
   \internal
   \brief 从Accessible Inteface中获取Name信息。
   优先级高于 accessibleText 的方式，如果未找到对应 interface 才会找 accessible text。

   \return 成功返回true，失败返回false。
 */
bool DAccessibilityCheckerPrivate::checkViewItemNameFromAccessibleInteface(QAbstractItemView *listview)
{
    qCDebug(logUtilClasses) << "Checking view item names from accessible interface";
    auto tableAbsInterface = QAccessible::queryAccessibleInterface(listview);
    bool ret = false;

    if (!tableAbsInterface || !tableAbsInterface->isValid()) {
        qCDebug(logUtilClasses) << "No valid accessible interface found";
        return ret;
    }

    QAccessibleTableInterface *tableInterface = tableAbsInterface->tableInterface();
    if (!tableInterface) {
        qCDebug(logUtilClasses) << "No table interface found";
        return ret;
    }

    ret = true;
    qCDebug(logUtilClasses) << "Checking table with" << tableInterface->rowCount() << "rows and" << tableInterface->columnCount() << "columns";
    for (int rowIdx = 0; rowIdx < tableInterface->rowCount(); ++rowIdx) {
        for (int columnIdx = 0; columnIdx < tableInterface->columnCount(); ++columnIdx) {
            QAccessibleInterface *cellAbsInterface = tableInterface->cellAt(rowIdx, columnIdx);
            if (cellAbsInterface && cellAbsInterface->isValid()) {
                switch (outputFormat) {
                case DAccessibilityChecker::AssertFormat: {
                    Q_ASSERT_X(!cellAbsInterface->text(QAccessible::Name).isEmpty(), "Item Accessible Text Check", viewItemOutputLog(rowIdx, columnIdx, listview).toLocal8Bit());
                }
                    break;
                case DAccessibilityChecker::FullFormat: {
                    if (cellAbsInterface->text(QAccessible::Name).isEmpty()) {
                        itemWariningList.append(viewItemOutputLog(rowIdx, columnIdx, listview));
                        qCDebug(logUtilClasses) << "Found empty accessible name at row" << rowIdx << "column" << columnIdx;
                    }
                }
                    break;
                default:
                    break;
                }
            }
        }
    }

    return ret;
}

/*!
   \internal
   \brief 从函数 accessibleText 中获取Name信息.

   \return 成功返回true，失败返回false。
 */
bool DAccessibilityCheckerPrivate::checkViewItemNameFromAccessibleText(QAbstractItemView *listview)
{
    qCDebug(logUtilClasses) << "Checking view item names from accessible text";
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(listview->model());
    bool ret = false;

    if (!model) {
        qCDebug(logUtilClasses) << "No standard item model found";
        return ret;
    }

    ret = true;
    qCDebug(logUtilClasses) << "Checking model with" << model->rowCount() << "rows and" << model->columnCount() << "columns";
    for (int rowIdx = 0; rowIdx < model->rowCount(); ++rowIdx) {
        for (int columnIdx = 0; columnIdx < model->columnCount(); ++columnIdx) {
            auto standardItem = model->item(rowIdx, columnIdx);

            if (!standardItem)
                continue;

            switch (outputFormat) {
            case DAccessibilityChecker::AssertFormat: {
                Q_ASSERT_X(!standardItem->accessibleText().isEmpty(), "Item Accessible Text Check", QStringLiteral("\n%1\n").arg(viewItemOutputLog(rowIdx, columnIdx, listview, standardItem->text())).toLocal8Bit());
            }
                break;
            case DAccessibilityChecker::FullFormat: {
                if (standardItem->accessibleText().isEmpty()) {
                    itemWariningList.append(viewItemOutputLog(rowIdx, columnIdx, listview, standardItem->text()));
                    qCDebug(logUtilClasses) << "Found empty accessible text at row" << rowIdx << "column" << columnIdx;
                }
            }
                break;
            default:
                break;
            }
        }
    }

    return ret;
}

/*!
   \internal
   \brief 忽视的部分类
   Qt中部分类不做显示 应当忽视。
 */
bool DAccessibilityCheckerPrivate::isDefaultIgnoreWidget(const QWidget *w) const
{
    qCDebug(logUtilClasses) << "Checking default ignore widget";
    static QStringList defaultIgnoredWidgetObjectName = {
        QLatin1String("qt_scrollarea_hcontainer"),
        QLatin1String("qt_scrollarea_vcontainer"),
        QLatin1String("qt_scrollarea_viewport"),
        QLatin1String("qt_spinbox_lineedit"),
        QLatin1String("qt_toolbar_ext_button")
    };

    static QByteArrayList defaultIgnoredWidgetClassName = {
        QByteArrayLiteral("QScrollBar"),
        QByteArrayLiteral("QToolTip"),
        QByteArrayLiteral("QComboBoxPrivateContainer"),
        QByteArrayLiteral("QComboBoxListView"),
        QByteArrayLiteral("QComboBoxPrivateScroller"),
        QByteArrayLiteral("QColumnViewGrip")
    };

    bool ret = defaultIgnoredWidgetObjectName.contains(w->objectName());

    if (!ret) {
        ret = std::any_of(defaultIgnoredWidgetClassName.begin(), defaultIgnoredWidgetClassName.end(), [w](const QByteArray &ignoreClass) -> bool { return w->inherits(ignoreClass); });
    }

    qCDebug(logUtilClasses) << "Default ignore widget result:" << ret;
    return ret;
}

/*! \internal */
QString DAccessibilityCheckerPrivate::widgetOutputLog(const QWidget *w) const
{
    qCDebug(logUtilClasses) << "Generating widget output log";
    return QStringLiteral("[-------------]Widget [%1] has no accessible name.\nWidget Detail:\n\t%2").arg(w->metaObject()->className()).arg(widgetInfoString(w));
}

/*! \internal */
QString DAccessibilityCheckerPrivate::widgetInfoString(const QWidget *w) const
{
    qCDebug(logUtilClasses) << "Generating widget info string";
    QString splices("%1    Widget Path: %2");

    QString classDetailString(QStringLiteral("Class Name: ") + w->metaObject()->className());
    if (!w->objectName().isEmpty())
        classDetailString.append(QStringLiteral(", Object Name: ") + w->objectName());

    QString classPathString(QStringLiteral(" ——► ") + w->metaObject()->className());
    QWidget *parentWidget = w->parentWidget();
    if (!parentWidget) {
        classPathString = w->metaObject()->className();
    } else {
        while (parentWidget->parentWidget()) {
            classPathString.prepend(parentWidget->metaObject()->className()).prepend(QStringLiteral(" ——► "));
            parentWidget = parentWidget->parentWidget();
        }

        classPathString.prepend(parentWidget->metaObject()->className());
    }

    splices = splices.arg(classDetailString).arg(classPathString);
    return splices;
}

/*! \internal */
QString DAccessibilityCheckerPrivate::viewItemOutputLog(int rowIndex, int columnIndex, const QAbstractItemView *absView, const QString &itemText) const
{
    qCDebug(logUtilClasses) << "Generating view item output log";
    return QStringLiteral("[-------------]View Item has no accessible text.\n View Item Detail:\n\t%1")
        .arg(itemText.isEmpty() ? QStringLiteral("Row: %1, Column %2, Contained in ListView: [%3]").arg(rowIndex).arg(columnIndex).arg(widgetInfoString(absView))
                                : QStringLiteral("Text: %1, Row: %2, Column %3, Contained in  ListView: [%4]").arg(itemText).arg(rowIndex).arg(columnIndex).arg(widgetInfoString(absView)));
}

/*!
   \internal
   \brief 该函数只在 FullFormat 下执行。用于格式化出所有的检查内容.
 */
void DAccessibilityCheckerPrivate::formatCheckResult()
{
    qCDebug(logUtilClasses) << "Formatting check results";
    if (widgetsWarningList.isEmpty() && itemWariningList.isEmpty()) {
        qCDebug(logUtilClasses) << "No warnings to format";
        return;
    }

    qInfo().noquote() << "[=============]Found the following items missing the accessible name.";
    printRoleWarningOutput(QStringLiteral("Widgets"), widgetsWarningList);
    printRoleWarningOutput(QStringLiteral("View Items"), itemWariningList);
}

/*!
    \internal
    \brief 用于输出统计结果，汇总标记内容.
 */
void DAccessibilityCheckerPrivate::printSummaryResults()
{
    qCDebug(logUtilClasses) << "Printing summary results";
    int totalWidgetsCount = std::accumulate(this->topLevelWidgets.begin(), this->topLevelWidgets.end(), 0, [](int before, const QWidget *after) -> int { return before + after->findChildren<QWidget *>().count(); }) + this->topLevelWidgets.count();

    QString summary("[=============]Result Summary: Total Widgets Number: %1    Succeeded: %2    Failed: %3    Ignored: %4");
    qWarning().noquote() << summary.arg(totalWidgetsCount).arg(totalWidgetsCount - this->widgetsWarningList.count() - this->widgetIgnoredCount).arg(this->widgetsWarningList.count()).arg(this->widgetIgnoredCount);
}

/*! \internal */
void DAccessibilityCheckerPrivate::printRoleWarningOutput(const QString &roleString, const QStringList &roleList)
{
    qCDebug(logUtilClasses) << "Printing role warning output for" << roleString;
    if (!roleList.isEmpty()) {
        qInfo().noquote() << QStringLiteral("[*************]%1:").arg(roleString);

        for (const QString &roleResult : roleList)
            qWarning("%s\n", roleResult.toLocal8Bit().toStdString().c_str());
    }
}

void DAccessibilityCheckerPrivate::_q_checkTimeout()
{
    qCDebug(logUtilClasses) << "Check timeout triggered";
    D_Q(DAccessibilityChecker);
    this->topLevelWidgets = qApp->topLevelWidgets();
    this->widgetIgnoredCount = 0;

    if (!q->check()) {
        qCDebug(logUtilClasses) << "Check failed, aborting";
        abort();
    }
}

/*!
  \class Dtk::Widget::DAccessibilityChecker
  \inmodule dtkwidget

  \brief 该类用于检测自动化标记是否存在.

  这是一个用于检测控件自动化标记是否完整添加的类，推荐该类在Debug模式下工作。可以使
  用 QT_DEBUG 或 QT_NO_DEBUG 宏指定当前是否为debug模式。断言输出模式下，程序在遇到控
  件不存在自动化标记名称时断言退出，并提示出具体控件和路径；全输出模式下，程序会输出
  全部的日志信息，且程序不会退出。除此之外，start() 函数会每隔3秒执行 check() 函数，
  如发现有控件不存在自动化标记名称，程序会直接退出并提示对应控件信息。一般的使用方法
  如下代码所示：
  \code
  MainWindow w;
  #ifdef QT_DEBUG
  DAccessibilityChecker checker;
  checker.start();
  #endif
  w.show();
  \endcode
  或
  \code
  MainWindow w;
  #ifdef QT_DEBUG
  DAccessibilityChecker checker;
  checker.setOutputFormat(DAccessibilityChecker::FullFormat);
  checker.check();
  #endif
  w.show();
  \endcode
  如果检测出某一类不存在自动化标记，会存在类似如下的提示
  \code
  Check Widget [QWidget] has no accessible name.
  Widget Detail:
      Class Name: QWidget, Object Name: CentralWidget	Widget Path: MainWindow ——► QWidget
  \endcode
  其中，可以从控件信息中获取到类名（ClassName）、对象名(ObjectName，如果存在)、对象路径（到顶层窗口的完整的父子关系）。

 \warning 目前该类只用于检测自动化标记的名称。
 \note 对象名（setObjectName）设置过的类，能够更容易找到是否添加了自动化标记名称
 */

/*!
   \brief 默认构造.

   默认构造

   \a parent 父类指针
 */
DAccessibilityChecker::DAccessibilityChecker(QObject *parent)
    : QObject(parent)
    , DObject(*new DAccessibilityCheckerPrivate(this))
{
    qCDebug(logUtilClasses) << "Constructing DAccessibilityChecker";
}

void DAccessibilityChecker::setOutputFormat(DAccessibilityChecker::OutputFormat format)
{
    qCDebug(logUtilClasses) << "Setting output format:" << static_cast<int>(format);
    D_D(DAccessibilityChecker);
    d->outputFormat = format;
}

/*!
   \enum DAccessibilityChecker::OutputFormat

   \brief 日志格式枚举.

   \var AssertFormat
   断言日志，如检查到缺失，直接断言。
   \var FullFormat
   全日志，全日志将所有的缺失信息显示到控制台中。
   \sa DAccessibilityChecker::outputFormat
 */

/*!
  \enum DAccessibilityChecker::Role

  \brief 检测的角色.

  检测角色目前分为两种。
  \var Widget
  所有继承自 QWidget 的小控件
  \var ViewItem
  所有继承自 QStandardItem 的View Item项
 */

/*!
   \property DAccessibilityChecker::outputFormat

   \brief 日志输出格式.

   可以通过设置日志输出格式来控制输出的检查日志是断言日志还是全日志。
   默认情况下，这个属性是断言日志。
   \sa DAccessibilityChecker::OutputFormat
 */
DAccessibilityChecker::OutputFormat DAccessibilityChecker::outputFormat() const
{
    qCDebug(logUtilClasses) << "Getting output format";
    D_DC(DAccessibilityChecker);
    return d->outputFormat;
}

/*!
  \brief 检测自动化标记名称.

  该函数用于检测自动化检测的名称，目前支持检测的类型为 QWidget QStandardItem。

  \return 所有控件都存在自动化标记名称，返回true，否则返回false。
  \note 建议该函数在debug模式下执行，release模式下会影响程序运行
  \sa start()
 */
bool DAccessibilityChecker::check()
{
    qCDebug(logUtilClasses) << "Starting accessibility check";
    D_D(DAccessibilityChecker);
    return d->check();
}

/*!
  \brief 定时检测控件的标记名称.

  调用此函数会定时执行自动化标记检测，如果发现某控件的自动化标记存在缺失，则程序退出并提示出控件的相关信息。
  \a msec 定时开启的时间，默认为3秒.

  \sa check()
 */
void DAccessibilityChecker::start(int msec)
{
    qCDebug(logUtilClasses) << "Starting timer with interval:" << msec;
    D_D(DAccessibilityChecker);

    if (!d->checkTimer) {
        d->checkTimer = new QTimer(this);
        QObject::connect(d->checkTimer,  SIGNAL(timeout()), this, SLOT(_q_checkTimeout()));
        qCDebug(logUtilClasses) << "Created new timer";
    }

    // 先立即执行 再开始定时器执行。
    d->_q_checkTimeout();
    d->checkTimer->start(msec);
}

bool DAccessibilityChecker::isIgnore(DAccessibilityChecker::Role role, const QWidget *w)
{
    qCDebug(logUtilClasses) << "Checking if widget is ignored";
    D_D(DAccessibilityChecker);

    return d->isIgnore(role, w);
}

DWIDGET_END_NAMESPACE
#include "moc_daccessibilitychecker.cpp"
