// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dprogressbar.h"

#include <QStyle>
#include <QStyleOptionProgressBar>
#include <QLoggingCategory>

DWIDGET_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(logProgressAnimation)
/*!
@~english
  @class Dtk::Widget::DProgressBar
  @brief DProgressBar is a widget for progress bar
  @details The DProgressBar Class inherits from The QProgressBar Class and provides the function of displaying progress bar.
*/

/*!
@~english
  @brief DProgressBar constructor.

  @param[in] parent is passed to QProgressBar constructor.
*/
DProgressBar::DProgressBar(QWidget *parent)
    : QProgressBar(parent)
{
    qCDebug(logProgressAnimation) << "Construct progress bar"
                                  << reinterpret_cast<const void *>(this);

}

/*!
@~english
  @brief similar to QProgressBar::sizeHint()，return the siezehint of progress bar
  @return sizehint of the progress bar
*/
QSize DProgressBar::sizeHint() const
{
    qCDebug(logProgressAnimation) << "Size hint";
    if (isTextVisible()) {
        return QProgressBar::sizeHint();
    }

    QStyleOptionProgressBar opt;
    initStyleOption(&opt);
    return style()->sizeFromContents(QStyle::CT_ProgressBar, &opt, QSize(1, 1), this);
}

/*!
@~english
  @brief similar to QProgressBar::minimumSizeHint()，return the minimum siezehint of progress bar
  @return minimum sizehint of the progress bar
*/
QSize DProgressBar::minimumSizeHint() const
{
    qCDebug(logProgressAnimation) << "Minimum size hint";
    if (isTextVisible()) {
        return QProgressBar::minimumSizeHint();
    }

    return sizeHint();
}

DWIDGET_END_NAMESPACE
