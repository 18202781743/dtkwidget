// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DObjectPrivate>
#include <DGuiApplicationHelper>

#include "dpalettehelper.h"
#include "dstyleoption.h"
#include "private/dpalettehelper_p.h"
#include <QLoggingCategory>

DWIDGET_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(logStyleTheme)

static DPaletteHelper *g_instance = nullptr;

DPaletteHelperPrivate::DPaletteHelperPrivate(DPaletteHelper *qq)
    : DTK_CORE_NAMESPACE::DObjectPrivate(qq)
{
}

DPaletteHelper::DPaletteHelper(QObject *parent)
    : QObject(parent)
    , DTK_CORE_NAMESPACE::DObject(*new DPaletteHelperPrivate(this))
{
    qCDebug(logStyleTheme) << "Construct palette helper"
                           << reinterpret_cast<const void *>(this);
}

DPaletteHelper::~DPaletteHelper()
{
    qCDebug(logStyleTheme) << "Destruct palette helper";
    if (g_instance) {
        g_instance->deleteLater();
        g_instance = nullptr;
    }
}

DPaletteHelper *DPaletteHelper::instance()
{
    qCDebug(logStyleTheme) << "Get palette helper singleton";
    if (!g_instance) {
        qCDebug(logStyleTheme) << "Create palette helper singleton";
        g_instance = new DPaletteHelper;
    }

    return g_instance;
}

/*!
  \brief DPaletteHelper::palette返回调色板
  \a widget 控件
  \a base 调色板
  \return 调色板
 */
DPalette DPaletteHelper::palette(const QWidget *widget, const QPalette &base) const
{
    D_DC(DPaletteHelper);

    DPalette palette;

    if (!widget) {
        qCDebug(logStyleTheme) << "No widget, return application palette";
        return DGuiApplicationHelper::instance()->applicationPalette();
    }

    do {
        // 先从缓存中取数据
        if (d->paletteCache.contains(widget)) {
            qCDebug(logStyleTheme) << "Palette cache hit"
                                   << reinterpret_cast<const void *>(widget);
            palette = d->paletteCache.value(widget);
            break;
        }

        if (QWidget *parent = widget->parentWidget()) {
            qCDebug(logStyleTheme) << "Inherit palette from parent"
                                   << reinterpret_cast<const void *>(parent);
            palette = this->palette(parent, base);
        } else {
            qCDebug(logStyleTheme) << "Use application palette";
            palette = DGuiApplicationHelper::instance()->applicationPalette();
        }

        // 判断widget对象有没有被设置过palette
        if (widget->testAttribute(Qt::WA_SetPalette)) {
            // 存在自定义palette时应该根据其自定义的palette获取对应色调的DPalette
            const QPalette &wp = widget->palette();

            // 判断控件自己的palette色调是否和要继承调色板色调一致
            if (DGuiApplicationHelper::instance()->toColorType(palette) != DGuiApplicationHelper::instance()->toColorType(wp)) {
                // 不一致时则fallback到标准的palette
                qCDebug(logStyleTheme) << "Tone mismatch, fallback to standard palette";
                palette = DGuiApplicationHelper::instance()->standardPalette(DGuiApplicationHelper::instance()->toColorType(wp));
            }
        }

        // 缓存数据
        const_cast<DPaletteHelperPrivate *>(d)->paletteCache.insert(widget, palette);
        // 关注控件palette改变的事件
        const_cast<QWidget *>(widget)->installEventFilter(const_cast<DPaletteHelper *>(this));
    } while (false);
    palette.QPalette::operator=(
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    base.resolve()
#else
    base.resolveMask()
#endif
     ? base : widget->palette());
    qCDebug(logStyleTheme) << "Palette generated"
                           << reinterpret_cast<const void *>(widget);
    return palette;
}

/*!
  \brief DPaletteHelper::setPalette将调色板设置到控件
  \a widget 控件
  \a palette 调色板
 */
void DPaletteHelper::setPalette(QWidget *widget, const DPalette &palette)
{
    D_D(DPaletteHelper);

    qCDebug(logStyleTheme) << "Set widget palette"
                           << reinterpret_cast<const void *>(widget);
    d->paletteCache.insert(widget, palette);
    widget->installEventFilter(const_cast<DPaletteHelper *>(this));
    // 记录此控件被设置过palette
    widget->setProperty("_d_set_palette", true);
    widget->setPalette(palette);
}

/*!
  \brief DPaletteHelper::resetPalette重置控件的调色板属性
  \a widget 控件
 */
void DPaletteHelper::resetPalette(QWidget *widget)
{
    D_D(DPaletteHelper);

    // 清理数据
    qCDebug(logStyleTheme) << "Reset widget palette"
                           << reinterpret_cast<const void *>(widget);
    d->paletteCache.remove(widget);
    widget->setProperty("_d_set_palette", QVariant());
    widget->setAttribute(Qt::WA_SetPalette, false);
}

bool DPaletteHelper::eventFilter(QObject *watched, QEvent *event)
{
    D_D(DPaletteHelper);

    qCDebug(logStyleTheme) << "Filter event"
                           << reinterpret_cast<const void *>(watched)
                           << static_cast<int>(event->type());
    if (Q_UNLIKELY(event->type() == QEvent::PaletteChange)) {
        if (QWidget *widget = qobject_cast<QWidget *>(watched)) {
            if (!widget->property("_d_set_palette").toBool()) {
                // 清理缓存
                qCDebug(logStyleTheme) << "Palette changed, clear cache"
                                       << reinterpret_cast<const void *>(widget);
                d->paletteCache.remove(widget);
            }
        }
    } else if (Q_UNLIKELY(event->type() == QEvent::Destroy)) {
        if (QWidget *widget = qobject_cast<QWidget *>(watched)) {
            if (d->paletteCache.contains(widget)) {
                // 清理缓存
                qCDebug(logStyleTheme) << "Widget destroyed, clear cache"
                                       << reinterpret_cast<const void *>(widget);
                d->paletteCache.remove(widget);
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

DWIDGET_END_NAMESPACE
