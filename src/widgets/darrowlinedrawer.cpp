// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "darrowlinedrawer.h"
#include "dheaderline.h"
#include "diconbutton.h"
#include "private/ddrawer_p.h"

#include <QResizeEvent>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logContainers, "dtk.widgets.containers")

DWIDGET_BEGIN_NAMESPACE

namespace HeaderLine {
    class ArrowHeaderLine : public DHeaderLine
    {
        Q_OBJECT
    public:
        ArrowHeaderLine(QWidget *parent = nullptr);
        void setExpand(bool value);

    Q_SIGNALS:
        void mousePress();

    protected:
        void mousePressEvent(QMouseEvent *) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void changeEvent(QEvent *e) override;

    private:
        void reverseArrowDirection();
        bool m_isExpanded = false;
        DIconButton *m_arrowButton = nullptr;
    };

    ArrowHeaderLine::ArrowHeaderLine(QWidget *parent) :
        DHeaderLine(parent)
    {
        qCDebug(logContainers) << "Creating ArrowHeaderLine with parent:" << parent;
        m_arrowButton = new DIconButton(DStyle::SP_ReduceElement, this);
        qCDebug(logContainers) << "Created arrow button with reduce element";
        m_arrowButton->setFlat(true);
        m_arrowButton->setAccessibleName("ArrowHeaderArrowButton");
        setExpand(false);
        connect(m_arrowButton, &DIconButton::clicked, this, &ArrowHeaderLine::mousePress);
        qCDebug(logContainers) << "Connected arrow button click signal";
        setContent(m_arrowButton);
        setFixedHeight(EXPAND_HEADER_HEIGHT);
        qCDebug(logContainers) << "Initialized ArrowHeaderLine with height:" << EXPAND_HEADER_HEIGHT;
    }

    void ArrowHeaderLine::setExpand(bool value)
    {
        qCDebug(logContainers) << "Setting expand state to:" << value;
        if (value) {
            qCDebug(logContainers) << "Setting expand element icon";
            m_arrowButton->setIcon(DStyle::SP_ExpandElement);
        } else {
            qCDebug(logContainers) << "Setting reduce element icon";
            m_arrowButton->setIcon(DStyle::SP_ReduceElement);
        }
        m_isExpanded = value;
        qCDebug(logContainers) << "Expand state updated";
    }

    void ArrowHeaderLine::mousePressEvent(QMouseEvent *event)
    {
        qCDebug(logContainers) << "Mouse press event received at:" << event->pos();
        Q_EMIT mousePress();
        qCDebug(logContainers) << "Mouse press signal emitted";
    }

    void ArrowHeaderLine::mouseMoveEvent(QMouseEvent *event)
    {
        qCDebug(logContainers) << "Mouse move event blocked at:" << event->pos();
        //屏蔽掉鼠标移动事件
        event->accept();
        qCDebug(logContainers) << "Mouse move event accepted";
    }

    void ArrowHeaderLine::changeEvent(QEvent *e)
    {
        qCDebug(logContainers) << "Change event received, type:" << e->type();
        if (e->type() == QEvent::FontChange) {
            int newHeight = qMax(EXPAND_HEADER_HEIGHT, this->fontMetrics().height());
            qCDebug(logContainers) << "Font changed, adjusting height to:" << newHeight;
            setFixedHeight(newHeight);
        }

        qCDebug(logContainers) << "Forwarding change event to base class";
        return DHeaderLine::changeEvent(e);
    }

    void ArrowHeaderLine::reverseArrowDirection()
    {
        qCDebug(logContainers) << "Reversing arrow direction, current state:" << m_isExpanded;
        setExpand(!m_isExpanded);
        qCDebug(logContainers) << "Arrow direction reversed to:" << !m_isExpanded;
    }
}
using namespace HeaderLine;

class DArrowLineDrawerPrivate : public DDrawerPrivate
{
public:
    D_DECLARE_PUBLIC(DArrowLineDrawer)
    explicit DArrowLineDrawerPrivate(DDrawer *qq)
        : DDrawerPrivate(qq) {
        qCDebug(logContainers) << "Creating DArrowLineDrawerPrivate";
    }

    ArrowHeaderLine *headerLine = nullptr;
};

/*!
@~english
  \class Dtk::Widget::DArrowLineDrawer
  \inmodule dtkwidget
  @brief A beautiful expandable control.
  
  DArrowLineDrawer inherited from DDrawer and provide ArrowHeaderLine (a button marked with an arrow)as its fixed title control，that is to say, compared with DDrawer, the step of providing title control is omitted，you only need to provide content controls，if you need a custom title control, you should use the DDrawer class.
  \sa  DDrawer
 */

/*!
@~english
  @brief Construct a DArrowLineDrawer example
  
  \a parent is the parent control of the instance
 */
DArrowLineDrawer::DArrowLineDrawer(QWidget *parent)
    : DDrawer(*new DArrowLineDrawerPrivate(this), parent)
{
    qCDebug(logContainers) << "Creating DArrowLineDrawer with parent:" << parent;
    D_D(DArrowLineDrawer);
    d->headerLine = new ArrowHeaderLine(this);
    qCDebug(logContainers) << "Created header line";
    d->headerLine->setExpand(expand());
    d->headerLine->setAccessibleName("DArrowLineDrawerHeaderLine");
    qCDebug(logContainers) << "Set header line properties";
    connect(d->headerLine, &ArrowHeaderLine::mousePress, [=]{
        qCDebug(logContainers) << "Header line mouse press, toggling expand state";
        setExpand(!expand());
    });
    setHeader(d->headerLine);
    qCDebug(logContainers) << "DArrowLineDrawer initialization completed";
}

/*!
@~english
  @brief Set the text for the title to display
  
  \a title Title content
 */
void DArrowLineDrawer::setTitle(const QString &title)
{
    qCDebug(logContainers) << "Setting title to:" << title;
    D_D(DArrowLineDrawer);
    d->headerLine->setTitle(title);
    qCDebug(logContainers) << "Title set successfully";
}

/*!
@~english
  @brief Sets whether to expand to display the content control
  
  \a value If it is true, it will be displayed, and vice versa.
 */
void DArrowLineDrawer::setExpand(bool value)
{
    qCDebug(logContainers) << "Setting expand state to:" << value;
    D_D(DArrowLineDrawer);
    //Header's arrow direction change here
    qCDebug(logContainers) << "Updating header line expand state";
    d->headerLine->setExpand(value);
    qCDebug(logContainers) << "Updating base drawer expand state";
    DDrawer::setExpand(value);
    qCDebug(logContainers) << "Expand state updated successfully";
}

/*!
@~english
  @brief Get the title control
  \return Title control
  \sa DHeaderLine DBaseLine
 */
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
DBaseLine *DArrowLineDrawer::headerLine()
{
    qCDebug(logContainers) << "Getting header line";
    D_D(DArrowLineDrawer);
    qCDebug(logContainers) << "Returning header line:" << d->headerLine;
    return d->headerLine;
}
#endif

void DArrowLineDrawer::setHeader(QWidget *header)
{
    qCDebug(logContainers) << "Setting header widget:" << header;
    DDrawer::setHeader(header);
    qCDebug(logContainers) << "Header widget set successfully";
}

void DArrowLineDrawer::resizeEvent(QResizeEvent *e)
{
    qCDebug(logContainers) << "Resize event received, new size:" << e->size();
    D_D(DArrowLineDrawer);
    qCDebug(logContainers) << "Updating header line width to:" << e->size().width();
    d->headerLine->setFixedWidth(e->size().width());

    qCDebug(logContainers) << "Forwarding resize event to base class";
    DDrawer::resizeEvent(e);
    qCDebug(logContainers) << "Resize event handled";
}

DWIDGET_END_NAMESPACE

#include "darrowlinedrawer.moc"
