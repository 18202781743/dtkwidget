// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "darrowbutton.h"
#include "dthememanager.h"
#include <QLoggingCategory>

DWIDGET_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(logBasicWidgets)

ArrowButtonIcon::ArrowButtonIcon(QWidget *parent) :
    QLabel(parent)
{
    qCDebug(logBasicWidgets) << "Init arrow icon"
                             << reinterpret_cast<const void *>(this);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void ArrowButtonIcon::setArrowDirection(int direction)
{
    qCDebug(logBasicWidgets) << "Set arrow direction" << direction;
    m_direction = direction;
    style()->unpolish(this);
    style()->polish(this);// force a stylesheet recomputation
}

void ArrowButtonIcon::setButtonState(int state)
{
    qCDebug(logBasicWidgets) << "Set button state" << state;
    m_buttonState = state;
    style()->unpolish(this);
    style()->polish(this);// force a stylesheet recomputation
}

int ArrowButtonIcon::arrowDirection() const
{
    qCDebug(logBasicWidgets) << "Get arrow direction" << m_direction;
    return m_direction;
}

int ArrowButtonIcon::buttonState() const
{
    qCDebug(logBasicWidgets) << "Get button state" << m_buttonState;
    return m_buttonState;
}

/*!
@~english    
  \class Dtk::Widget::DArrowButton
  \inmodule dtkwidget
  @brief DArrowButton allowed you create button with arrow icon conveniently.

  \image html DArrowButton.png

  
  You can use DArrowButton to create button with arrow icon, and it also allowed you update the arrow direction by
  calling setArrowDirection() . You can also update arrow state via arrowButtonDirection and arrowButtonState property.

  \sa QLabel
*/

/*!
@~english
  @brief Construct a new DArrowButton with DArrowButton::ArrowDown direction.
  
  \a parent Parent control pointer.
 */
DArrowButton::DArrowButton(QWidget *parent)
    : QLabel(parent)
{
    qCDebug(logBasicWidgets) << "Init arrow button"
                             << reinterpret_cast<const void *>(this);
    setFixedSize(IMAGE_BUTTON_WIDTH, BUTTON_HEIGHT);

    m_normalLabel = new ArrowButtonIcon(this);
    m_normalLabel->setAccessibleName("DArrowButtonNormalLabel");
    m_normalLabel->setObjectName("ArrowButtonIconNormal");
    m_normalLabel->setFixedSize(size());
    m_normalLabel->move(0, 0);

    m_hoverLabel = new ArrowButtonIcon(this);
    m_hoverLabel->setAccessibleName("DArrowButtonHoverLabel");
    m_hoverLabel->setObjectName("ArrowButtonIconHover");
    m_hoverLabel->setFixedSize(size());
    m_hoverLabel->move(0, 0);
    m_hoverLabel->setVisible(false);

    m_pressLabel = new ArrowButtonIcon(this);
    m_pressLabel->setAccessibleName("DArrowButtonPressedLabel");
    m_pressLabel->setObjectName("ArrowButtonIconPress");
    m_pressLabel->setFixedSize(size());
    m_pressLabel->move(0, 0);
    m_pressLabel->setVisible(false);

    setArrowDirection(DArrowButton::ArrowDown);
}

/*!
@~english
  @brief Set arrow direction of the button.

  \a direction The direction of the arrow.

  \sa DArrowButton::ArrowDirection DArrowButton::arrowDirection()
 */
void DArrowButton::setArrowDirection(ArrowDirection direction)
{
    qCDebug(logBasicWidgets) << "Update arrow direction" << static_cast<int>(direction);
    m_arrowDirection = direction;
    updateIconDirection(direction);
}

/*!
@~english
  @brief Get the arrow direction of the button.

  \return Returns the direction of the arrow.
 */
int DArrowButton::arrowDirection() const
{
    qCDebug(logBasicWidgets) << "Read arrow direction" << m_arrowDirection;
    return m_arrowDirection;
}

/*!
@~english
  @brief Set the button state.

  \a state The state of the arrow button.
 */
void DArrowButton::setButtonState(ArrowButtonState state)
{
    qCDebug(logBasicWidgets) << "Update button state" << static_cast<int>(state);
    m_buttonState = state;
    updateIconState(state);
}

/*!
@~english
  @brief Get the button state.

  \return Returns the status of the button.
 */
int DArrowButton::buttonState() const
{
    qCDebug(logBasicWidgets) << "Read button state" << m_buttonState;
    return m_buttonState;
}

void DArrowButton::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    qCDebug(logBasicWidgets) << "Mouse press";
    Q_EMIT mousePress();
    m_normalLabel->setVisible(false);
    m_hoverLabel->setVisible(false);
    m_pressLabel->setVisible(true);

    setButtonState(DArrowButton::ArrowStatePress);
}

void DArrowButton::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    qCDebug(logBasicWidgets) << "Mouse release";
    Q_EMIT mouseRelease();
    m_normalLabel->setVisible(true);
    m_hoverLabel->setVisible(false);
    m_pressLabel->setVisible(false);

    setButtonState(DArrowButton::ArrowStateNormal);
}

/* \reimp */
void DArrowButton::enterEvent(QEvent *)
{
    qCDebug(logBasicWidgets) << "Mouse enter";
    Q_EMIT mouseEnter();
}

/* \reimp */
void DArrowButton::leaveEvent(QEvent *)
{
    qCDebug(logBasicWidgets) << "Mouse leave";
    Q_EMIT mouseLeave();
}

void DArrowButton::initButtonState()
{
//    QStateMachine * machine = new QStateMachine(this);
//    QState * hoverState = new QState(machine);
//    hoverState->assignProperty(this,"pos",QPoint(0,0));
//    QState * hideState = new QState(machine);
//    hideState->assignProperty(this,"pos",QPoint(0,height()));

//    machine->setInitialState(hoverState);

//    QPropertyAnimation *sa = new QPropertyAnimation(this, "pos");
//    sa->setDuration(200);
//    sa->setEasingCurve(QEasingCurve::InSine);
//    connect(sa,&QPropertyAnimation::finished,this,&Panel::hasShown);

//    QPropertyAnimation *ha = new QPropertyAnimation(this, "pos");
//    ha->setDuration(200);
//    ha->setEasingCurve(QEasingCurve::InSine);
//    connect(ha,&QPropertyAnimation::finished,this,&Panel::hasHidden);

//    QSignalTransition *ts1 = hoverState->addTransition(this,SIGNAL(startHide()), hideState);
//    ts1->addAnimation(ha);
//    connect(ts1,&QSignalTransition::triggered,[=](int value = 2){
//        m_HSManager->SetState(value);
//    });
//    QSignalTransition *ts2 = hideState->addTransition(this,SIGNAL(startShow()),hoverState);
//    ts2->addAnimation(sa);
//    connect(ts2,&QSignalTransition::triggered,[=](int value = 0){
//        m_HSManager->SetState(value);
//    });

//    machine->start();
}

/*!
@~english
  \enum Dtk::Widget::DArrowButton::ArrowDirection

  @brief The ArrowDirection enum indicate the direction of the arrow icon in the arrown button.
  
  \value ArrowUp
  Up direction
  
  \value ArrowDown
  Down direction
  
  \value ArrowLeft
  Left direction
  
  \value ArrowRight
  Right direction
*/

/*!
@~english
  \enum Dtk::Widget::DArrowButton::ArrowButtonState
  
  @brief The ArrowDirection enum indicate the direction of the arrow icon in the arrown button.
  
  \value  ArrowStateNormal
  Normal state
  
  \value  ArrowStateHover
  Mouse hover state
  
  \value  ArrowStatePress
  Button got pressed state
 */

/*!
@~english
  @brief Update arrow direction of the button.
 */
void DArrowButton::updateIconDirection(ArrowDirection direction)
{
    qCDebug(logBasicWidgets) << "Update icon direction" << static_cast<int>(direction);
    m_normalLabel->setArrowDirection(direction);
    m_hoverLabel->setArrowDirection(direction);
    m_pressLabel->setArrowDirection(direction);
}

/*!
@~english
  @brief Update icon state of the button.
 */
void DArrowButton::updateIconState(ArrowButtonState state)
{
    qCDebug(logBasicWidgets) << "Update icon state" << static_cast<int>(state);
    m_normalLabel->setButtonState(state);
    m_hoverLabel->setButtonState(state);
    m_pressLabel->setButtonState(state);
}

DWIDGET_END_NAMESPACE
