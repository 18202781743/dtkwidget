// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dapplicationhelper.h"
#include "dpalettehelper.h"
#include <QLoggingCategory>
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
#include <QApplication>

DWIDGET_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(logWindowPlatform)

class _DApplicationHelper {
public:
    static DGuiApplicationHelper *createHelper()
    {
        qCDebug(logWindowPlatform) << "Create application helper instance";
        return new DApplicationHelper();
    }
};

__attribute__((constructor)) // This function is executed when the library is loaded
static void init_createHelper ()
{
    if (!QApplication::instance() || qobject_cast<QApplication *>(QApplication::instance())) {
        qCDebug(logWindowPlatform) << "Register application helper creator";
        DApplicationHelper::registerInstanceCreator(_DApplicationHelper::createHelper);
    } else {
        qCDebug(logWindowPlatform) << "Skip creator registration, non-QApplication env";
    }
}

class DApplicationHelperPrivate
{
public:
    DPaletteHelper *paletteHelper = DPaletteHelper::instance();
};

static DApplicationHelperPrivate *d = nullptr;

/*!
  \class Dtk::Widget::DApplicationHelper
  \inmodule dtkwidget
  \brief `dApplicationHelper` provided a modified `DGuiApplicationHelper` 类.
 */

/*!
  \brief `DApplicationHelper::instance`return `DApplicationHelper` object
 */
DApplicationHelper *DApplicationHelper::instance()
{
    qCDebug(logWindowPlatform) << "Get application helper instance";
    return qobject_cast<DApplicationHelper*>(DGuiApplicationHelper::instance());
}

/*!
  \brief `DApplicationHelper::palette` return a palette
  \a widget widget
  \a base Palette
  \return Palette
 */
DPalette DApplicationHelper::palette(const QWidget *widget, const QPalette &base) const
{
    qCDebug(logWindowPlatform) << "Get widget palette"
                               << reinterpret_cast<const void *>(widget);
    return d->paletteHelper->palette(widget, base);
}

/*!
  \brief `DApplicationHelper::setPalette` set the palette to the control
  \a widget widget
  \a palette palette
 */
void DApplicationHelper::setPalette(QWidget *widget, const DPalette &palette)
{
    qCDebug(logWindowPlatform) << "Set widget palette"
                               << reinterpret_cast<const void *>(widget);
    d->paletteHelper->setPalette(widget, palette);
}

/*!
  \brief `DApplicationHelper::resetPalette` Reset the color panel attribute of the reset control
  \a widget widget
 */
void DApplicationHelper::resetPalette(QWidget *widget)
{
    qCDebug(logWindowPlatform) << "Reset widget palette"
                               << reinterpret_cast<const void *>(widget);
    d->paletteHelper->resetPalette(widget);
}

DApplicationHelper::DApplicationHelper()
{
    qCDebug(logWindowPlatform) << "Initialize application helper";
    if (!d)
        d = new DApplicationHelperPrivate();
}

DApplicationHelper::~DApplicationHelper()
{
    qCDebug(logWindowPlatform) << "Destroy application helper";
    if (d) {
        delete d;
        d = nullptr;
    }
}

bool DApplicationHelper::eventFilter(QObject *watched, QEvent *event)
{
    qCDebug(logWindowPlatform) << "Event filter triggered"
                               << reinterpret_cast<const void *>(watched)
                               << static_cast<int>(event->type());
    Q_ASSERT_X(false, Q_FUNC_INFO, "This function should not be called.");

    return DGuiApplicationHelper::eventFilter(watched, event);
}

bool DApplicationHelper::event(QEvent *event)
{
    qCDebug(logWindowPlatform) << "Handle event"
                               << static_cast<int>(event->type());
    return DGuiApplicationHelper::event(event);
}

DWIDGET_END_NAMESPACE
#endif
