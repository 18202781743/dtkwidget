// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtitlebarsettings.h"
#include "dtitlebar.h"
#include "private/dtitlebarsettingsimpl.h"
#include <DObjectPrivate>
#include <QLoggingCategory>

DWIDGET_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(logSettingsConfig)

class DTitlebarSettingsPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    Q_DECLARE_PUBLIC(DTitlebarSettings)

    DTitlebarSettingsPrivate(DTitlebarSettings *qq);

    DTitlebarSettingsImpl* impl;
    DTitlebar *titlebar;
};

DTitlebarSettingsPrivate::DTitlebarSettingsPrivate(DTitlebarSettings *qq)
    : DObjectPrivate(qq)
    , impl(new DTitlebarSettingsImpl())
{
}

DTitlebarSettings::DTitlebarSettings(DTitlebar *titlebar)
    : DObject( *new DTitlebarSettingsPrivate(this))
{
    qCDebug(logSettingsConfig) << "Construct titlebar settings"
                               << reinterpret_cast<const void *>(this);
    D_D(DTitlebarSettings);
    d->titlebar = titlebar;
}

bool DTitlebarSettings::initilize(QList<DTitlebarToolBaseInterface *> &tools, const QString &path)
{
    D_D(DTitlebarSettings);
    qCDebug(logSettingsConfig) << "Initialize titlebar settings"
                               << reinterpret_cast<const void *>(d->titlebar);
    d->impl->setTools(tools);
    if (!d->impl->load(path)) {
        qCDebug(logSettingsConfig) << "Load config failed";
        return false;
    }

    auto titleBarEditPanel = d->impl->toolsView();
    titleBarEditPanel->setParent(d->titlebar->parentWidget());
    titleBarEditPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->titlebar->setCustomWidget(titleBarEditPanel);
    qCDebug(logSettingsConfig) << "Initialize done";
    return true;
}

QWidget *DTitlebarSettings::toolsEditPanel() const
{
    D_DC(DTitlebarSettings);
    qCDebug(logSettingsConfig) << "Get tools panel";
    return d->impl->toolsEditPanel();
}

DTitlebarSettingsImpl *DTitlebarSettings::impl()
{
    D_D(DTitlebarSettings);
    qCDebug(logSettingsConfig) << "Get impl";
    return d->impl;
}

DWIDGET_END_NAMESPACE

#include "moc_dtitlebarsettings.cpp"
