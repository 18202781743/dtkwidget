// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtooltip.h"
#include "dstyle.h"

#include <DPlatformWindowHandle>

#include <QDebug>
#include <QEvent>
#include <QTimer>
#include <QToolTip>
#include <QTextLayout>
#include <QLoggingCategory>

DWIDGET_BEGIN_NAMESPACE
Q_LOGGING_CATEGORY(logUtils, "dtk.widgets.utils")
namespace DToolTipStatic {
static inline void registerDToolTipMetaType()
{
    qRegisterMetaType<DToolTip::ToolTipShowMode>();
}

Q_CONSTRUCTOR_FUNCTION(registerDToolTipMetaType);

static Qt::TextFormat textFormat = Qt::TextFormat::AutoText;
}

/*!
@~english
  @brief set the Tooltip text format
  Set the Tooltip text format inside DStyle to the specified style by setting format.
  @param[in] format text format
  @sa Qt::TextFormat
 */
void DToolTip::setToolTipTextFormat(Qt::TextFormat format)
{
    qCDebug(logUtils) << "set tooltip text format" << static_cast<int>(format);
    
    if (DToolTipStatic::textFormat == format) {
        qCDebug(logUtils) << "tooltip text format unchanged, skipping update";
        return;
    }
    
    DToolTipStatic::textFormat = format;
    qCDebug(logUtils) << "tooltip text format updated successfully";
}
/*!
@~english
  @brief get the Tooltip text format
  @return Tooltip text format used inside DStyle
  @sa Qt::TextFormat
 */
Qt::TextFormat DToolTip::toolTipTextFormat()
{
    qCDebug(logUtils) << "get tooltip text format" << static_cast<int>(DToolTipStatic::textFormat);
    return DToolTipStatic::textFormat;
}

/*!
@~english
 * @brief DToolTip::setToolTipShowMode
 * @param[in] widget widget to set ToolTip show mode
 * @param[in] mode ToolTip show mode
 */
/*!
@~english
  @enum Dtk::Widget::DToolTip::ToolTipShowMode
  @brief The ToolTipShowMode enum indicate the ToolTip show mode.
  
  \value  NotShow
  Do not display the ToolTip.
  
  \value  AlwaysShow
  Always display the ToolTip.

  \value  ShowWhenElided
  Display the ToolTip when the text is elided.

  \value  Default
  Use the default ToolTip show mode.
 */
void DToolTip::setToolTipShowMode(QWidget *widget, ToolTipShowMode mode)
{
    qCDebug(logUtils) << "setting tooltip show mode:" << mode << "for widget:" << widget;
    if (!widget) {
        qCDebug(logUtils) << "widget is null, skipping";
        return;
    }
    
    QVariant currentMode = widget->property("_d_dtk_toolTipMode");
    if (currentMode.isValid() && qvariant_cast<ToolTipShowMode>(currentMode) == mode) {
        qCDebug(logUtils) << "tooltip show mode unchanged, skipping update";
        return;
    }
    
    widget->setProperty("_d_dtk_toolTipMode", mode);
    qCDebug(logUtils) << "tooltip show mode set successfully";
}

/*!
@~english
 * @brief DToolTip::toolTipShowMode
 * @param[in] widget widget to get ToolTip show mode
 * @return ToolTip show mode
 */
DToolTip::ToolTipShowMode DToolTip::toolTipShowMode(const QWidget *widget)
{
    qCDebug(logUtils) << "getting tooltip show mode for widget:" << widget;
    if (!widget) {
        qCDebug(logUtils) << "widget is null, returning default mode";
        return ToolTipShowMode::Default;
    }
    
    QVariant vToolTipMode = widget->property("_d_dtk_toolTipMode");
    if (vToolTipMode.isValid()) {
        const auto& mode = qvariant_cast<ToolTipShowMode>(vToolTipMode);
        qCDebug(logUtils) << "returning tooltip mode:" << mode;
        return mode;
    } else {
        qCDebug(logUtils) << "no tooltip mode property found, returning default tooltip mode";
        return ToolTipShowMode::Default;
    }
}

QString DToolTip::wrapToolTipText(QString text, QTextOption option)
{
    qCDebug(logUtils) << "wrapping tooltip text, length:" << text.length();
    if (text.isEmpty()) {
        qCDebug(logUtils) << "empty text, returning empty string";
        return "";
    }
    const auto MaxPixelsPerRow = DStyle::pixelMetric(nullptr, DStyle::PixelMetric::PM_ToolTipLabelWidth);
    QStringList paragraphs = text.split('\n');
    qCDebug(logUtils) << "text split into" << paragraphs.size() << "paragraphs";
    const QFont &toolTipFont = QToolTip::font();
    QString toolTip{""};
    for (const QString &paragraph : std::as_const(paragraphs))
    {
        if (paragraph.isEmpty())
            continue;
        QTextLayout toolTipLayout(paragraph, toolTipFont);
        toolTipLayout.setTextOption(option);
        qreal height = 0;
        toolTipLayout.beginLayout();
        QTextLine line = toolTipLayout.createLine();
        while(line.isValid()) {
            line.setLineWidth(MaxPixelsPerRow);
            line.setPosition({0, height});
            height += line.height();
            line = toolTipLayout.createLine();
        }
        toolTipLayout.endLayout();
        for (int i = 0; i < toolTipLayout.lineCount(); ++i) {
            const auto &currentLine = toolTipLayout.lineAt(i);
            const auto &toolTipText = toolTipLayout.text();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            toolTip.append(toolTipText.midRef(currentLine.textStart(), currentLine.textLength()));
#else
            toolTip.append(QStringView{toolTipText}.mid(currentLine.textStart(), currentLine.textLength()));
#endif
            toolTip.append('\n');
        }
    }
    toolTip.chop(1);
    qCDebug(logUtils) << "wrapped tooltip text length:" << toolTip.length();
    return toolTip;
}

bool DToolTip::needUpdateToolTip(const QWidget *widget, bool showToolTip)
{
    qCDebug(logUtils) << "checking if tooltip needs update, showToolTip:" << showToolTip;
    QVariant vShowToolTip = widget->property("_d_dtk_showToolTip");
    bool needUpdate = false;
    if (vShowToolTip.isValid()) {
        bool oldShowStatus = vShowToolTip.toBool();
        if (showToolTip != oldShowStatus) {
            qCDebug(logUtils) << "tooltip status changed, needs update";
            needUpdate = true;
        } else {
            qCDebug(logUtils) << "tooltip status unchanged";
        }
    } else {
        qCDebug(logUtils) << "no previous tooltip status, needs update";
        needUpdate = true;
    }
    return needUpdate;
}

void DToolTip::setShowToolTip(QWidget *widget, bool showToolTip)
{
    qCDebug(logUtils) << "setting show tooltip:" << showToolTip << "for widget:" << widget;
    if (!widget) {
        qCDebug(logUtils) << "widget is null, skipping";
        return;
    }
    
    QVariant currentValue = widget->property("_d_dtk_showToolTip");
    if (currentValue.isValid() && currentValue.toBool() == showToolTip) {
        qCDebug(logUtils) << "show tooltip value unchanged, skipping update";
        return;
    }
    
    widget->setProperty("_d_dtk_showToolTip", showToolTip);
    qCDebug(logUtils) << "show tooltip property set successfully";
}

/*!
@~english
  @class Dtk::Widget::DToolTip
  @inmodule dtkwidget
  @brief like QToolTip
  @details DToolTip provides a function that displays DTooltip in the specified position.
 */

/*!
@~english
  @brief DToolTip constructor
  @param[in] text text message
  @param[in] completionClose Prompt whether the tip is destroyed after disappearing.
 */
DToolTip::DToolTip(const QString &text, bool completionClose)
    : DTipLabel(text)
{
    qCDebug(logUtils) << "creating tooltip with text:" << text << "completion close:" << completionClose;
    if (completionClose) {
        setAttribute(Qt::WA_DeleteOnClose);
    }
}

/*!
@~english
  @brief DToolTip::sizeHint

  @sa QLabel::sizeHint()
 */
QSize DToolTip::sizeHint() const
{
    qCDebug(logUtils) << "calculating tooltip size hint";
    int radius = DStyleHelper(style()).pixelMetric(DStyle::PM_FrameRadius);
    QSize fontSize = fontMetrics().size({}, text());

    fontSize.setWidth(fontSize.width() + radius);
    qCDebug(logUtils) << "tooltip size hint:" << fontSize;
    return fontSize;
}

/*!
@~english
  @brief DToolTip::show set the location where the tip appears

  @param[in] pos location of tip
  @param[in] duration tip existence time, unit in millisecond
 */
void DToolTip::show(const QPoint &pos, int duration)
{
    qCDebug(logUtils) << "showing tooltip at position:" << pos << "duration:" << duration;
    QTimer::singleShot(duration, this, &DTipLabel::close);

    DTipLabel::show(pos);
}

DWIDGET_END_NAMESPACE
