// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dprintpreviewsettinginfo.h"

#include <QLoggingCategory>

DWIDGET_BEGIN_NAMESPACE

namespace {
Q_DECLARE_LOGGING_CATEGORY(logUtils)
}

DPrintPreviewSettingInfo::DPrintPreviewSettingInfo(SettingType type)
    : t(type)
{
    qCDebug(logUtils) << "Creating print preview setting info with type:" << int(type);
}

Dtk::Widget::DPrintPreviewSettingInfo::~DPrintPreviewSettingInfo()
{
    qCDebug(logUtils) << "Destroying print preview setting info";
}

DWIDGET_END_NAMESPACE
