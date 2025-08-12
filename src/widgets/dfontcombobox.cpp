// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dfontcombobox.h"

#include "private/dcombobox_p.h"
#include <DObjectPrivate>
#include <QScreen>
#include <QWindow>
#include <QListView>
#include <QTableView>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QDesktopWidget>
#endif
#include <QEvent>
#include <QApplication>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logBasicWidgets, "dtk.widgets.basic")

DWIDGET_BEGIN_NAMESPACE

class DFontComboBoxPrivate : public DComboBoxPrivate
{
    D_DECLARE_PUBLIC(DFontComboBox)
public:
    explicit DFontComboBoxPrivate(DFontComboBox* q)
      : DComboBoxPrivate(q)
      , impl(new QFontComboBox())
    {
        qCDebug(logBasicWidgets) << "Creating DFontComboBoxPrivate";
    }

    virtual ~DFontComboBoxPrivate() override;

    // The specific implementation of DFontComboBox adopts the implementation of QFontComboBox
    QFontComboBox* impl;
};

DFontComboBoxPrivate::~DFontComboBoxPrivate()
{
    qCDebug(logBasicWidgets) << "Destroying DFontComboBoxPrivate";
    impl->deleteLater();
}

/*!
@~english
  @class DFontComboBox
  @brief The DFontComboBox class provides combobox for selecting font

  The DFontComboBox class provides functions to select system font

  @code 
  #code example
  DFontComboBox *fontComboBox = new DFontComboBox();
  @endcode
  
  @image html DFontComboBox.png
/*!
@~english
  @brief DFontComboBox constructor.
  @param[in] parent is passed to DComboBox constructor
 */
DFontComboBox::DFontComboBox(QWidget *parent)
    : DComboBox(*new DFontComboBoxPrivate(this), parent)
{
    qCDebug(logBasicWidgets) << "Creating DFontComboBox";
    setModel(d_func()->impl->model());
    setItemDelegate(d_func()->impl->itemDelegate());

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(this, SIGNAL(currentIndexChanged(QString)),
            d_func()->impl, SIGNAL(currentIndexChanged(QString)));
#else
    connect(this, SIGNAL(currentIndexChanged(int)),
            d_func()->impl, SIGNAL(currentIndexChanged(int)));
#endif
    connect(d_func()->impl, SIGNAL(currentFontChanged(const QFont &)),
            this, SIGNAL(currentFontChanged(const QFont &)));
}

DFontComboBox::~DFontComboBox()
{
    qCDebug(logBasicWidgets) << "Destroying DFontComboBox";
}

/*!
@~english
  @brief same as QFontComboBox::setWritingSystem
 */
void DFontComboBox::setWritingSystem(QFontDatabase::WritingSystem script)
{
    qCDebug(logBasicWidgets) << "Setting writing system to:" << script;
    D_D(DFontComboBox);
    d->impl->setWritingSystem(script);
}

/*!
@~english
  @brief same as QFontComboBox::writingSystem
 */
QFontDatabase::WritingSystem DFontComboBox::writingSystem() const
{
    qCDebug(logBasicWidgets) << "Getting writing system:" << d->impl->writingSystem();
    D_DC(DFontComboBox);
    return d->impl->writingSystem();
}

/*!
@~english
  @brief same as QFontComboBox::setFontFilters
 */
void DFontComboBox::setFontFilters(QFontComboBox::FontFilters filters)
{
    qCDebug(logBasicWidgets) << "Setting font filters to:" << filters;
    D_DC(DFontComboBox);
    return d->impl->setFontFilters(filters);
}

/*!
@~english
  @brief same as QFontComboBox::fontFilters
 */
QFontComboBox::FontFilters DFontComboBox::fontFilters() const
{
    qCDebug(logBasicWidgets) << "Getting font filters:" << d->impl->fontFilters();
    D_DC(DFontComboBox);
    return d->impl->fontFilters();
}

/*!
@~english
  @brief same as QFontComboBox::currentFont
 */
QFont DFontComboBox::currentFont() const
{
    qCDebug(logBasicWidgets) << "Getting current font:" << d->impl->currentFont().family();
    D_DC(DFontComboBox);
    return d->impl->currentFont();
}

/*!
@~english
  @brief same as QFontComboBox::sizeHint
 */
QSize DFontComboBox::sizeHint() const
{
    qCDebug(logBasicWidgets) << "Getting size hint:" << d->impl->sizeHint();
    D_DC(DFontComboBox);
    return d->impl->sizeHint();
}

/*!
@~english
  @brief same as QFontComboBox::setCurrentFont
 */
void DFontComboBox::setCurrentFont(const QFont &f)
{
    qCDebug(logBasicWidgets) << "Setting current font to:" << f.family();
    D_DC(DFontComboBox);
    d->impl->setCurrentFont(f);
}

/*!
@~english
  @brief same as QFontComboBox::event
 */
bool DFontComboBox::event(QEvent *e)
{
    qCDebug(logBasicWidgets) << "Font combo box event type:" << e->type();
    if (e->type() == QEvent::Resize) {
        qCDebug(logBasicWidgets) << "Handling resize event";
        QListView *lview = qobject_cast<QListView*>(view());
        if (lview) {
            qCDebug(logBasicWidgets) << "Found list view, adjusting window width";
            lview->winId();
            auto window = lview->window();
            int newWidth = qMin(width() * 5 / 3,
                               window->windowHandle()->screen()->availableGeometry().width());
            qCDebug(logBasicWidgets) << "Setting window width to:" << newWidth;
            window->setFixedWidth(newWidth);
        } else {
            qCDebug(logBasicWidgets) << "No list view found";
        }
    }
    return DComboBox::event(e);
}

DWIDGET_END_NAMESPACE
