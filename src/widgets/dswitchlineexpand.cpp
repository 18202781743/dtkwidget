// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dswitchlineexpand.h"
#include "dthememanager.h"

#include <QResizeEvent>
#include <QLoggingCategory>

DWIDGET_BEGIN_NAMESPACE

namespace {
Q_DECLARE_LOGGING_CATEGORY(logBasicWidgets)
}

/*!
  \class Dtk::Widget::DSwitchHeaderLine
  \inmodule dtkwidget
  \brief DSwitchHeaderLine 类用于提供 DSwitchLineExpand 的标题栏.
 */

/*!
  \fn void DSwitchHeaderLine::checkedChanged(bool arg)
  \brief 信号在控件开关状态发生改变时触发.
  \a arg checked状态
 */

/*!
  \brief DSwitchHeaderLine::DSwitchHeaderLine 为 DSwitchHeaderLine 的构造函数.
  \a parent 指定了控件的父控件。
  
  一般情况下，无需手动创建 DSwitchHeaderLine 对象。
 */
DSwitchHeaderLine::DSwitchHeaderLine(QWidget *parent) :
    DHeaderLine(parent)
{
    qCDebug(logBasicWidgets) << "Creating switch header line";
    m_switchButton = new DSwitchButton(this);
    m_switchButton->setAccessibleName("DSwitchHeaderLineSwitchButton");
    connect(m_switchButton, &DSwitchButton::checkedChanged, this, &DSwitchHeaderLine::checkedChanged);
    setContent(m_switchButton);
}

/*!
  \brief DSwitchHeaderLine::setExpand 用于设置控件的开关状态.
  \a value 为指定的开关状态，true 为开， false 为关。
 */
void DSwitchHeaderLine::setExpand(bool value)
{
    qCDebug(logBasicWidgets) << "Setting expand state:" << value;
    m_switchButton->setChecked(value);
}

void DSwitchHeaderLine::mousePressEvent(QMouseEvent *)
{
    const auto newState = !m_switchButton->isChecked();
    qCDebug(logBasicWidgets) << "Mouse press, toggling to:" << newState;
    m_switchButton->setChecked(newState);
//    Q_EMIT mousePress();
}

/*!
  \class Dtk::Widget::DSwitchLineExpand
  \inmodule dtkwidget
  \brief DSwitchLineExpand 类提供了一种通过开关按钮来展开/合上显示内容的控件.

  控件由标题栏和内容两部分组成，标题栏左侧显示控件标题，右侧显示一个开关控件，
  用户通过点击右侧的开关控件，来控制标题栏下面的内容部分展开还是合上。
  
  \image html switchlineexpand.png
 */

/*!
  \brief DSwitchLineExpand 类的构造函数.

  \a parent 指定了控件的父控件。
 */
DSwitchLineExpand::DSwitchLineExpand(QWidget *parent) : DDrawer(parent) {
    qCDebug(logBasicWidgets) << "Creating switch line expand widget";
    m_headerLine = new DSwitchHeaderLine(this);
    m_headerLine->setExpand(expand());
    m_headerLine->setAccessibleName("DSwitchLineExpandHeaderLine");
    connect(m_headerLine, &DSwitchHeaderLine::checkedChanged,
            [=](bool arg) { 
                qCDebug(logBasicWidgets) << "Header checked changed:" << arg;
                DDrawer::setExpand(arg); 
            });
    setHeader(m_headerLine);
}

/*!
  \brief DSwitchLineExpand::setTitle 用于设置控件的标题。
  \a title 为所要设置的标题。
 */
void DSwitchLineExpand::setTitle(const QString &title)
{
    qCDebug(logBasicWidgets) << "Setting title:" << title;
    m_headerLine->setTitle(title);
}

/*!
  \brief DSwitchLineExpand::setExpand 用于控制控件是否展开。
  \a value 为所要设置的展开状态，true 为展开，false 为合上。
  
  \note 这个函数同时会影响标题栏中开关按钮的状态。
 */
void DSwitchLineExpand::setExpand(bool value)
{
    qCDebug(logBasicWidgets) << "Setting expand state:" << value;
    //Header's arrow direction change here
    m_headerLine->setExpand(value);
}

/*!
  \brief DSwitchLineExpand::header 用来获取标题栏控件。
  \return 标题栏控件。
 */
DBaseLine *DSwitchLineExpand::header()
{
    qCDebug(logBasicWidgets) << "Getting header widget";
    return m_headerLine;
}

/*!
  \brief DSwitchLineExpand::setHeader 用于自定义标题栏的内容。
  \a header 为自定义的标题栏内容。
  
  \note 自定义的内容用于替代原来的标题显示部分，右侧的开关按钮不受影响。
 */
void DSwitchLineExpand::setHeader(QWidget *header)
{
    qCDebug(logBasicWidgets) << "Setting custom header widget";
    DDrawer::setHeader(header);
}

void DSwitchLineExpand::resizeEvent(QResizeEvent *e)
{
    qCDebug(logBasicWidgets) << "Resize event, new width:" << e->size().width();
    m_headerLine->setFixedWidth(e->size().width());

    DDrawer::resizeEvent(e);
}

DWIDGET_END_NAMESPACE
