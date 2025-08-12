// SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DObjectPrivate>
#include <DGuiApplicationHelper>
#include <DIconTheme>

#include "dfileiconprovider.h"

#include <QLibrary>
#include <QMimeDatabase>
#include <QMimeType>
#include <QDebug>
#include <QLoggingCategory>

#ifdef USE_GTK_PLUS_2_0
#include <QUrl>
#undef signals // Collides with GTK symbols
#include <gtk/gtk.h>
#endif

DGUI_USE_NAMESPACE
DWIDGET_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(logUtilClasses)

#ifdef USE_GTK_PLUS_2_0
typedef enum {
    GNOME_ICON_LOOKUP_FLAGS_NONE = 0,
    GNOME_ICON_LOOKUP_FLAGS_EMBEDDING_TEXT = 1 << 0,
    GNOME_ICON_LOOKUP_FLAGS_SHOW_SMALL_IMAGES_AS_THEMSELVES = 1 << 1,
    GNOME_ICON_LOOKUP_FLAGS_ALLOW_SVG_AS_THEMSELVES = 1 << 2
} GnomeIconLookupFlags;

typedef enum {
    GNOME_ICON_LOOKUP_RESULT_FLAGS_NONE = 0,
    GNOME_ICON_LOOKUP_RESULT_FLAGS_THUMBNAIL = 1 << 0
} GnomeIconLookupResultFlags;

struct GnomeThumbnailFactory;
typedef gboolean(*Ptr_gnome_vfs_init)(void);
typedef char *(*Ptr_gnome_icon_lookup_sync)(
    GtkIconTheme *icon_theme,
    GnomeThumbnailFactory *,
    const char *file_uri,
    const char *custom_icon,
    GnomeIconLookupFlags flags,
    GnomeIconLookupResultFlags *result);

typedef GtkIconTheme *(*Ptr_gtk_icon_theme_get_default)(void);
#endif

class DFileIconProviderPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    DFileIconProviderPrivate(DFileIconProvider *qq);

    void init();
    QIcon getFilesystemIcon(const QFileInfo &info) const;
    QIcon fromTheme(QString iconName) const;

    D_DECLARE_PUBLIC(DFileIconProvider)

#ifdef USE_GTK_PLUS_2_0
    static Ptr_gnome_icon_lookup_sync gnome_icon_lookup_sync;
    static Ptr_gnome_vfs_init gnome_vfs_init;

    static Ptr_gtk_icon_theme_get_default gtk_icon_theme_get_default;

};

Ptr_gnome_icon_lookup_sync DFileIconProviderPrivate::gnome_icon_lookup_sync;
Ptr_gnome_vfs_init DFileIconProviderPrivate::gnome_vfs_init;
Ptr_gtk_icon_theme_get_default DFileIconProviderPrivate::gtk_icon_theme_get_default;
#else
};
#endif

DFileIconProviderPrivate::DFileIconProviderPrivate(DFileIconProvider *qq)
    : DObjectPrivate(qq)
{
    qCDebug(logUtilClasses) << "DFileIconProviderPrivate constructor called";
    init();
}

void DFileIconProviderPrivate::init()
{
    qCDebug(logUtilClasses) << "initializing DFileIconProviderPrivate";
#ifdef USE_GTK_PLUS_2_0
    gnome_icon_lookup_sync = (Ptr_gnome_icon_lookup_sync)QLibrary::resolve(QLatin1String("gnomeui-2"), 0, "gnome_icon_lookup_sync");
    gnome_vfs_init = (Ptr_gnome_vfs_init)QLibrary::resolve(QLatin1String("gnomevfs-2"), 0, "gnome_vfs_init");

    if (DGUI_NAMESPACE::DGuiApplicationHelper::instance()->isXWindowPlatform()) {
        qCDebug(logUtilClasses) << "X11 platform detected, resolving GTK icon theme";
        gtk_icon_theme_get_default = (Ptr_gtk_icon_theme_get_default)QLibrary::resolve(QLatin1String("gtk-x11-2.0"), 0, "gtk_icon_theme_get_default");
    }
#endif
}

QIcon DFileIconProviderPrivate::getFilesystemIcon(const QFileInfo &info) const
{
    qCDebug(logUtilClasses) << "getFilesystemIcon called for file:" << info.absoluteFilePath();
#ifdef USE_GTK_PLUS_2_0
    if (gnome_vfs_init && gnome_icon_lookup_sync && gtk_icon_theme_get_default) {
        qCDebug(logUtilClasses) << "using GTK icon lookup";
        gnome_vfs_init();
        GtkIconTheme *theme = gtk_icon_theme_get_default();
        QByteArray fileurl = QUrl::fromLocalFile(info.absoluteFilePath()).toEncoded();
        char *icon_name = gnome_icon_lookup_sync(theme,
                          NULL,
                          fileurl.data(),
                          NULL,
                          GNOME_ICON_LOOKUP_FLAGS_NONE,
                          NULL);
        QString iconName = QString::fromUtf8(icon_name);
        g_free(icon_name);
        if (iconName.startsWith(QLatin1Char('/'))) {
            qCDebug(logUtilClasses) << "returning absolute path icon:" << iconName;
            return QIcon(iconName);
        }
        qCDebug(logUtilClasses) << "returning theme icon:" << iconName;
        return fromTheme(iconName);
    }
#endif

    qCDebug(logUtilClasses) << "using MIME database for icon lookup";
    const QMimeType &db = QMimeDatabase().mimeTypeForFile(info);
    const QIcon &icon = fromTheme(db.iconName());

    if (!icon.isNull()) {
        qCDebug(logUtilClasses) << "found icon from MIME type:" << db.iconName();
        return icon;
    }

    qCDebug(logUtilClasses) << "using generic icon name:" << db.genericIconName();
    return fromTheme(db.genericIconName());
}

QIcon DFileIconProviderPrivate::fromTheme(QString iconName) const
{
    qCDebug(logUtilClasses) << "fromTheme called with icon name:" << iconName;
    QIcon icon = DIconTheme::findQIcon(iconName);

    if (Q_LIKELY(!icon.isNull())) {
        qCDebug(logUtilClasses) << "found icon in theme:" << iconName;
        return icon;
    }

    qCDebug(logUtilClasses) << "icon not found, trying fallback names";
    if (iconName == "application-vnd.debian.binary-package") {
        iconName = "application-x-deb";
        qCDebug(logUtilClasses) << "fallback to deb package icon";
    } else if (iconName == "application-vnd.rar") {
        iconName = "application-zip";
        qCDebug(logUtilClasses) << "fallback to zip icon for rar";
    } else if (iconName == "application-vnd.ms-htmlhelp") {
        iconName = "chmsee";
        qCDebug(logUtilClasses) << "fallback to chmsee icon";
    } else {
        qCDebug(logUtilClasses) << "no fallback available, returning null icon";
        return icon;
    }

    icon = DIconTheme::findQIcon(iconName);
    qCDebug(logUtilClasses) << "fallback icon result:" << (icon.isNull() ? "null" : "found");

    return icon;
}

Q_GLOBAL_STATIC(DFileIconProvider, globalFIP)

DFileIconProvider::DFileIconProvider()
    : DObject(*new DFileIconProviderPrivate(this))
{
    qCDebug(logUtilClasses) << "DFileIconProvider constructor called";
}

DFileIconProvider::~DFileIconProvider()
{
    qCDebug(logUtilClasses) << "DFileIconProvider destructor called";
}

DFileIconProvider *DFileIconProvider::globalProvider()
{
    qCDebug(logUtilClasses) << "globalProvider called";
    return globalFIP;
}

QIcon DFileIconProvider::icon(const QFileInfo &info) const
{
    qCDebug(logUtilClasses) << "icon called for file:" << info.absoluteFilePath();
    Q_D(const DFileIconProvider);

    return d->getFilesystemIcon(info);
}

QIcon DFileIconProvider::icon(const QFileInfo &info, const QIcon &feedback) const
{
    qCDebug(logUtilClasses) << "icon called with feedback for file:" << info.absoluteFilePath();
    const QIcon &icon = this->icon(info);

    if (icon.isNull()) {
        qCDebug(logUtilClasses) << "icon is null, returning feedback icon";
        return feedback;
    }

    qCDebug(logUtilClasses) << "returning found icon";
    return icon;
}

DWIDGET_END_NAMESPACE
