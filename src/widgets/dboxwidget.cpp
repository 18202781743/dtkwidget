// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QBoxLayout>
#include <QEvent>
#include <QDebug>
#include <QResizeEvent>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logBasicWidgets, "dtk.widgets.basic")

#include "dthememanager.h"
#include "dboxwidget.h"
#include "private/dboxwidget_p.h"

DWIDGET_BEGIN_NAMESPACE


DBoxWidgetPrivate::DBoxWidgetPrivate(DBoxWidget *qq):
    DObjectPrivate(qq),
    layout(new QBoxLayout(QBoxLayout::TopToBottom))
{
    qCDebug(logBasicWidgets) << "DBoxWidgetPrivate created";
}

void DBoxWidgetPrivate::init()
{
    Q_Q(DBoxWidget);
    qCDebug(logBasicWidgets) << "Initializing DBoxWidget";

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    q->setLayout(layout);
    q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

/*!
@~english
  @class Dtk::Widget::DBoxWidget
  \inmodule dtkwidget

  @brief The DBoxWidget class provides widget born with QBoxLayout set.

  Since an widget with QBoxLayout set is very common use case in Qt
  programming, yet very tedious, DBoxWidget is built to ease that pain.
  
  Also, DBoxWidget will calculate the size it needs automatically, so you don't
  need to set width for DHBoxWidget or height for DVBoxLayout.
  
  @sa DHBoxWidget and DVBoxWidget
 */

/*!
@~english
  @brief DBoxWidget::DBoxWidget constructs an instance of DBoxWidget.

  \a direction is the direction used by the internal QBoxLayout.
  \a parent is passed to QFrame constructor.
 */
DBoxWidget::DBoxWidget(QBoxLayout::Direction direction, QWidget *parent):
    QFrame(parent),
    DObject(*new DBoxWidgetPrivate(this))
{
    qCDebug(logBasicWidgets) << "DBoxWidget created with direction:" << direction;
    d_func()->layout->setDirection(direction);
    d_func()->init();
}

/*!
@~english
  @property DBoxWidget::direction

  @brief This property holds the direction of the internal QBoxLayout.

  @return QBoxLayout::Direction Current direction
 */
QBoxLayout::Direction DBoxWidget::direction() const
{
    Q_D(const DBoxWidget);
    qCDebug(logBasicWidgets) << "Getting box widget direction";
    return d->layout->direction();
}

/*!
@~english
  @brief This property holds the internal layout object.
  
  This property can be used to get the internal layout, so you can set
  some extra properties on the layout to match the custom needs.

  @return QBoxLayout*
 */
QBoxLayout *DBoxWidget::layout() const
{
    Q_D(const DBoxWidget);
    qCDebug(logBasicWidgets) << "Getting box widget layout";
    return d->layout;
}

/*!
@~english
  @brief DBoxWidget::addWidget adds widget to the internal layout.

  \a widget is the widget to be added.
 */
void DBoxWidget::addWidget(QWidget *widget)
{
    qCDebug(logBasicWidgets) << "Adding widget to box widget";
    layout()->addWidget(widget);
}

/*!
@~english
  @brief Sets the current direction of QBoxLayout
  
  \a direction
 */
void DBoxWidget::setDirection(QBoxLayout::Direction direction)
{
    Q_D(DBoxWidget);
    qCDebug(logBasicWidgets) << "Setting box widget direction:" << direction;

    if (d->layout->direction() == direction) {
        qCDebug(logBasicWidgets) << "Direction unchanged, skipping update";
        return;
    }

    d->layout->setDirection(direction);
    Q_EMIT directionChanged(direction);
}

/*!
@~english
  @brief When the direction is QBoxLayout::TopToBottom or QBoxLayout::BottomToTop, 
  the fixed height will use the passed height and set the minimum width to the passed width.
  Otherwise, the passed width is used as the fixed width and the height is the minimum height.
  \a size
 */
void DBoxWidget::updateSize(const QSize &size)
{
    qCDebug(logBasicWidgets) << "Updating box widget size:" << size;
    if(direction() == QBoxLayout::TopToBottom || direction() == QBoxLayout::BottomToTop) {
        qCDebug(logBasicWidgets) << "Vertical direction, setting fixed height";
        setFixedHeight(size.height());

        if(size.width() > minimumWidth()) {
            qCDebug(logBasicWidgets) << "Adjusting minimum width";
            setMinimumWidth(qMin(size.width(), maximumWidth()));
        }
    } else {
        qCDebug(logBasicWidgets) << "Horizontal direction, setting fixed width";
        setFixedWidth(size.width());

        if(size.height() > minimumHeight()) {
            qCDebug(logBasicWidgets) << "Adjusting minimum height";
            setMinimumHeight(qMin(size.height(), maximumHeight()));
        }
    }
}

/*!\reimp */
bool DBoxWidget::event(QEvent *ee)
{
    Q_D(const DBoxWidget);
    qCDebug(logBasicWidgets) << "DBoxWidget event:" << ee->type();

    if(ee->type() == QEvent::LayoutRequest) {
        if(size() != d->layout->sizeHint()) {
            qCDebug(logBasicWidgets) << "Layout request, updating size";
            updateSize(d->layout->sizeHint());
            updateGeometry();
        }
    } else if(ee->type() == QEvent::Resize) {
        qCDebug(logBasicWidgets) << "Resize event, emitting size changed";
        Q_EMIT sizeChanged(size());
    } else if(ee->type() == QEvent::ChildAdded) {
        qCDebug(logBasicWidgets) << "Child added, updating size";
        updateSize(d->layout->sizeHint());
    } else if(ee->type() == QEvent::ChildRemoved) {
        qCDebug(logBasicWidgets) << "Child removed, updating size";
        updateSize(d->layout->sizeHint());
    } else if(ee->type() == QEvent::Show) {
        qCDebug(logBasicWidgets) << "Show event, updating size";
        updateSize(d->layout->sizeHint());
    }

    return QWidget::event(ee);
}

/*!
@~english
  \reimp
  @brief DBoxWidget::sizeHint reimplemented from QWidget::sizeHint().
  @return the recommended size of this widget.
 */
QSize DBoxWidget::sizeHint() const
{
    Q_D(const DBoxWidget);
    qCDebug(logBasicWidgets) << "Getting box widget size hint";
    return d->layout->sizeHint();
}

/*!
@~english
  @class Dtk::Widget::DHBoxWidget
  \inmodule dtkwidget

  @brief The DHBoxWidget class is DBoxWidget with DBoxWidget::direction set to
   QBoxLayout::LeftToRight.
 */

/*!
@~english
  @brief DHBoxWidget::DHBoxWidget constructs an instance of DHBoxWidget.

  \a parent is passed to DBoxWidget constructor.
 */
DHBoxWidget::DHBoxWidget(QWidget *parent):
    DBoxWidget(QBoxLayout::LeftToRight, parent)
{

}

/*!
@~english
  @class Dtk::Widget::DVBoxWidget
  \inmodule dtkwidget

  @brief The DVBoxWidget class is DBoxWidget with DBoxWidget::direction set to
   QBoxLayout::TopToBottom.
 */

/*!
@~english
  @brief DVBoxWidget::DVBoxWidget constructs an instance of DVBoxWidget.

  \a parent is passed to DBoxWidget constructor.
 */
DVBoxWidget::DVBoxWidget(QWidget *parent):
    DBoxWidget(QBoxLayout::TopToBottom, parent)
{

}

DWIDGET_END_NAMESPACE
