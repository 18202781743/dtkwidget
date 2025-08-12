// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "combobox.h"

#include <QLoggingCategory>

namespace {
Q_DECLARE_LOGGING_CATEGORY(logListWidgets)
}

ComboBox::ComboBox(QWidget *parent) :
    QComboBox(parent)
{
    qCDebug(logListWidgets) << "Creating settings combobox";
}

void ComboBox::wheelEvent(QWheelEvent *e)
{
    if (hasFocus()) {
        qCDebug(logListWidgets) << "Processing wheel event with focus";
        QComboBox::wheelEvent(e);
    } else {
        qCDebug(logListWidgets) << "Ignoring wheel event without focus";
        return QWidget::wheelEvent(e);
    }
}
