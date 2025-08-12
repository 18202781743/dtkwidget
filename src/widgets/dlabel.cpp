// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dlabel.h"
#include "private/dlabel_p.h"
#include "dtooltip.h"

#include <DPaletteHelper>

#include <private/qhexstring_p.h>
#include <private/qlabel_p.h>
#include <QLoggingCategory>

DWIDGET_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(logBasicWidgets)

/*!
@~english
  \class Dtk::Widget::DLabel
  \inmodule dtkwidget
  @brief DLabel a re-implementation QLabel.

  DLabel provides a function to display the DLabel at a specified location
  DLabel provides a function to change the font color
 */

/*!
@~english
  @brief Constructor for DLabel.
  \a parent The argument is sent to the QLabel constructor.
 */
DLabel::DLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
    , DObject(*new DLabelPrivate(this))
{
    qCDebug(logBasicWidgets) << "DLabel constructor called with parent:" << (parent ? "valid" : "null");
    D_D(DLabel);
    d->init();
}

/*!
@~english
  @brief Constructor for DLabel.
  \a text Text message
  \a parent Specifying the parent object.
 */
DLabel::DLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
    , DObject(*new DLabelPrivate(this))
{
    qCDebug(logBasicWidgets) << "DLabel constructor called with text:" << text << "parent:" << (parent ? "valid" : "null");
    D_D(DLabel);
    d->init();
}

DLabel::~DLabel()
{
    qCDebug(logBasicWidgets) << "DLabel destructor called";
}

/*!
@~english
  @brief DLabel::setForegroundRole The font color displayed
  \a role Font color（QPalette::ColorRole）
 */
void DLabel::setForegroundRole(QPalette::ColorRole role)
{
    qCDebug(logBasicWidgets) << "setForegroundRole called with role:" << role;
    D_D(DLabel);

    d->color = DPalette::NoType;
    QLabel::setForegroundRole(role);
}

/*!
@~english
  @brief DLabel::setForegroundRole The font color displayed
  \a color Font color
 */
void DLabel::setForegroundRole(DPalette::ColorType color)
{
    qCDebug(logBasicWidgets) << "setForegroundRole called with color:" << color;
    D_D(DLabel);
    d->color = color;
}

/*!
@~english
  @brief DLabel::setElideMode Set the mode of ellipsis display
  \a elideMode Omitted schema enumeration
 */
void DLabel::setElideMode(Qt::TextElideMode elideMode)
{
    qCDebug(logBasicWidgets) << "setElideMode called with elideMode:" << elideMode;
    D_D(DLabel);
    if (d->elideMode == elideMode) {
        qCDebug(logBasicWidgets) << "elideMode unchanged, returning early";
        return;
    }
    d->elideMode = elideMode;
    update();
}

/*!
@~english
  @brief DLabel::elideMode Gets the pattern of the ellipsis
  \return Returns the pattern of ellipses
 */
Qt::TextElideMode DLabel::elideMode() const
{
    D_DC(DLabel);
    qCDebug(logBasicWidgets) << "elideMode called, returning:" << d->elideMode;
    return d->elideMode;
}

/*!
@~english
  @brief DLabel::DLabel Constructor function
  \a dd Private class member variables
  \a parent Parent control
 */
DLabel::DLabel(DLabelPrivate &dd, QWidget *parent)
    : QLabel(parent)
    , DObject(dd)
{
    qCDebug(logBasicWidgets) << "DLabel private constructor called";
    dd.init();
}

/*!
@~english
  @brief DLabel::initPainter Initialization painter
  \a painter painter parameter
 */
void DLabel::initPainter(QPainter *painter) const
{
    qCDebug(logBasicWidgets) << "initPainter called";
    D_DC(DLabel);
    QLabel::initPainter(painter);
    if (d->color != DPalette::NoType) {
        qCDebug(logBasicWidgets) << "setting custom color:" << d->color;
        QBrush color = DPaletteHelper::instance()->palette(this).brush(d->color);
        painter->setPen(QPen(color.color()));
    }
}

/*!
@~english
  @brief DLabel::paintEvent
  \a event Message event
  \sa QLabel::paintEvent()
 */
void DLabel::paintEvent(QPaintEvent *event)
{
    qCDebug(logBasicWidgets) << "paintEvent called";
    Q_UNUSED(event)
    QLabelPrivate *d = static_cast<QLabelPrivate*>(d_ptr.data());
    QStyle *style = QWidget::style();
    QPainter painter(this);
    drawFrame(&painter);
    QRect cr = contentsRect();
    cr.adjust(d->margin, d->margin, -d->margin, -d->margin);
    int align = QStyle::visualAlignment(d->isTextLabel ? DLabelPrivate::textDirection(d)
                                                       : layoutDirection(), QFlag(d->align));

#if QT_CONFIG(movie)
    if (d->movie && !d->movie->currentPixmap().isNull()) {
        if (d->scaledcontents)
            style->drawItemPixmap(&painter, cr, align, d->movie->currentPixmap().scaled(cr.size()));
        else
            style->drawItemPixmap(&painter, cr, align, d->movie->currentPixmap());
    }
    else
#endif
    if (d->isTextLabel) {
        QRectF lr = DLabelPrivate::layoutRect(d).toAlignedRect();
        QStyleOption opt;
        opt.initFrom(this);

        if (d->control) {
#ifndef QT_NO_SHORTCUT
            const bool underline = (bool)style->styleHint(QStyle::SH_UnderlineShortcut, 0, this, 0);
            if (d->shortcutId != 0
                && underline != d->shortcutCursor.charFormat().fontUnderline()) {
                QTextCharFormat fmt;
                fmt.setFontUnderline(underline);
                d->shortcutCursor.mergeCharFormat(fmt);
            }
#endif
            DLabelPrivate::ensureTextLayouted(d);
            QAbstractTextDocumentLayout::PaintContext context;
            // Adjust the palette
            context.palette = opt.palette;

            if (d_func()->color != DPalette::NoType) {
                context.palette.setBrush(QPalette::Text, DPaletteHelper::instance()->palette(this).brush(d_func()->color));
            } else if (foregroundRole() != QPalette::Text && isEnabled()) {
                context.palette.setColor(QPalette::Text, context.palette.color(foregroundRole()));
            }

            painter.save();
            painter.translate(lr.topLeft());
            painter.setClipRect(lr.translated(-lr.x(), -lr.y()));
            d->control->setPalette(context.palette);
            d->control->drawContents(&painter, QRectF(), this);
            painter.restore();
        } else {
            int flags = align | (DLabelPrivate::textDirection(d) == Qt::LeftToRight ? Qt::TextForceLeftToRight
                                                                                    : Qt::TextForceRightToLeft);
            if (d->hasShortcut) {
                flags |= Qt::TextShowMnemonic;
                if (!style->styleHint(QStyle::SH_UnderlineShortcut, &opt, this))
                    flags |= Qt::TextHideMnemonic;
            }

            QPalette palette = opt.palette;

            if (d_func()->color != DPalette::NoType) {
                palette.setBrush(foregroundRole(), DPaletteHelper::instance()->palette(this).brush(d_func()->color));
            }

            QString text = d->text;
            if (elideMode() != Qt::ElideNone) {
                const QFontMetrics fm(fontMetrics());
                text = fm.elidedText(text, elideMode(), width(), flags);
            }
            const DToolTip::ToolTipShowMode &toolTipShowMode = DToolTip::toolTipShowMode(this);
            if (toolTipShowMode != DToolTip::Default) {
                const bool showToolTip = (toolTipShowMode == DToolTip::AlwaysShow)
                        || ((toolTipShowMode == DToolTip::ShowWhenElided) && (d->text != text));
                if (DToolTip::needUpdateToolTip(this, showToolTip)) {
                    QString toolTip;
                    if (showToolTip) {
                        QTextOption textOption;
                        textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
                        textOption.setTextDirection(opt.direction);
                        textOption.setAlignment(Qt::Alignment(align));
                        toolTip = DToolTip::wrapToolTipText(d->text, textOption);
                    }
                    this->setToolTip(toolTip);
                    DToolTip::setShowToolTip(this, showToolTip);
                }
            }
            style->drawItemText(&painter, lr.toRect(), flags, palette, isEnabled(), text, foregroundRole());
        }
    } else
#ifndef QT_NO_PICTURE
    if (d->picture) {
        QRect br = d->picture->boundingRect();
        int rw = br.width();
        int rh = br.height();
        if (d->scaledcontents) {
            painter.save();
            painter.translate(cr.x(), cr.y());
            painter.scale((double)cr.width()/rw, (double)cr.height()/rh);
            painter.drawPicture(-br.x(), -br.y(), *d->picture);
            painter.restore();
        } else {
            int xo = 0;
            int yo = 0;
            if (align & Qt::AlignVCenter)
                yo = (cr.height()-rh)/2;
            else if (align & Qt::AlignBottom)
                yo = cr.height()-rh;
            if (align & Qt::AlignRight)
                xo = cr.width()-rw;
            else if (align & Qt::AlignHCenter)
                xo = (cr.width()-rw)/2;
            painter.drawPicture(cr.x()+xo-br.x(), cr.y()+yo-br.y(), *d->picture);
        }
    } else
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
        if (d->pixmap && !d->pixmap->isNull()) {
#else
        if (d->icon && !d->icon->isNull()) {
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
      QPixmap pix;
      if (d->scaledcontents) {
        QSize scaledSize = cr.size() * devicePixelRatioF();
        if (!d->scaledpixmap || d->scaledpixmap->size() != scaledSize) {
#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
                if (!d->cachedimage)
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 2)
                    d->cachedimage = new QImage(d->pixmap->toImage());
                delete d->scaledpixmap;
#else
#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
                    d->cachedimage = QImage(d->pixmap->toImage());
#endif
                d->scaledpixmap.reset();
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
                QImage scaledImage =
                    d->cachedimage->scaled(scaledSize,
                                           Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
#else
                    d->scaledpixmap = d->pixmap->scaled(scaledSize,
                                            Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 2)
                d->scaledpixmap = new QPixmap(QPixmap::fromImage(scaledImage));
#else
#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
                d->scaledpixmap = QPixmap(QPixmap::fromImage(scaledImage));
#endif
#endif
                d->scaledpixmap->setDevicePixelRatio(devicePixelRatioF());
        }
            pix = *d->scaledpixmap;
      } else
        pix = *d->pixmap;
      QStyleOption opt;
      opt.initFrom(this);
      if (!isEnabled())
        pix = style->generatedIconPixmap(QIcon::Disabled, pix, &opt);
      style->drawItemPixmap(&painter, cr, align, pix);
#else
      // pick up from
      // https://github.com/qt/qtbase/blob/25986746947798e1a22d0830d3bcb11a55fcd3ae/src/widgets/widgets/qlabel.cpp#L1052
      const qreal dpr = devicePixelRatio();
      const QSize size = d->scaledcontents ? cr.size() : d->pixmapSize;
      const auto mode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
      QPixmap pix = d->icon->pixmap(size, dpr, mode);
      if (d->scaledcontents && pix.size() != size * dpr) {
        using namespace Qt::StringLiterals;
        const QString key =
            "qt_label_"_L1 % HexString<quint64>(pix.cacheKey()) %
            HexString<quint8>(mode) % HexString<uint>(size.width()) %
            HexString<uint>(size.height()) %
            HexString<quint16>(qRound(dpr * 1000));
        if (!QPixmapCache::find(key, &pix)) {
          pix = pix.scaled(size * dpr, Qt::IgnoreAspectRatio,
                           Qt::SmoothTransformation);
          pix.setDevicePixelRatio(dpr);
          // using QIcon to cache the newly create pixmap is not possible
          // because QIcon does not clear this cache (so we grow indefinitely)
          // and also uses the newly added pixmap as starting point for new
          // scaled pixmap which makes it very blurry.
          // Therefore use QPixmapCache here.
          QPixmapCache::insert(key, pix);
        }
      }
      QStyleOption opt;
      opt.initFrom(this);
      style->drawItemPixmap(&painter, cr, align, pix);
#endif
    }
}

DLabelPrivate::DLabelPrivate(DLabel *q)
    : DObjectPrivate(q)
{
}

void DLabelPrivate::init()
{
    qCDebug(logBasicWidgets) << "DLabelPrivate::init called";
}

Qt::LayoutDirection DLabelPrivate::textDirection(QLabelPrivate *d)
{
    qCDebug(logBasicWidgets) << "Getting text direction";
    if (d->control) {
        qCDebug(logBasicWidgets) << "Getting direction from text control";
        QTextOption opt = d->control->document()->defaultTextOption();
        const auto& direction = opt.textDirection();
        qCDebug(logBasicWidgets) << "Text control direction:" << direction;
        return direction;
    }

    const auto& direction = d->text.isRightToLeft() ? Qt::RightToLeft : Qt::LeftToRight;
    qCDebug(logBasicWidgets) << "Text direction from string:" << direction;
    return direction;
}

QRectF DLabelPrivate::documentRect(QLabelPrivate *d)
{
    qCDebug(logBasicWidgets) << "Calculating document rect";
    QLabel *q = qobject_cast<QLabel*>(d->q_ptr);
    Q_ASSERT_X(d->isTextLabel, "documentRect", "document rect called for label that is not a text label!");
    QRect cr = q->contentsRect();
    qCDebug(logBasicWidgets) << "Initial contents rect:" << cr;

    cr.adjust(d->margin, d->margin, -d->margin, -d->margin);
    qCDebug(logBasicWidgets) << "Adjusted contents rect with margins:" << cr;

    const int align = QStyle::visualAlignment(d->isTextLabel ? textDirection(d)
                                                             : q->layoutDirection(), QFlag(d->align));
    qCDebug(logBasicWidgets) << "Visual alignment:" << align;

    int m = d->indent;
    if (m < 0 && q->frameWidth()) { // no indent, but we do have a frame
        m = q->fontMetrics().horizontalAdvance(QLatin1Char('x')) / 2 - d->margin;
        qCDebug(logBasicWidgets) << "Calculated indent from frame width:" << m;
    }

    if (m > 0) {
        qCDebug(logBasicWidgets) << "Applying indent:" << m;
        if (align & Qt::AlignLeft) {
            cr.setLeft(cr.left() + m);
            qCDebug(logBasicWidgets) << "Adjusted left for indent";
        }
        if (align & Qt::AlignRight) {
            cr.setRight(cr.right() - m);
            qCDebug(logBasicWidgets) << "Adjusted right for indent";
        }
        if (align & Qt::AlignTop) {
            cr.setTop(cr.top() + m);
            qCDebug(logBasicWidgets) << "Adjusted top for indent";
        }
        if (align & Qt::AlignBottom) {
            cr.setBottom(cr.bottom() - m);
            qCDebug(logBasicWidgets) << "Adjusted bottom for indent";
        }
    }

    qCDebug(logBasicWidgets) << "Final document rect:" << cr;
    return cr;
}

QRectF DLabelPrivate::layoutRect(QLabelPrivate *d)
{
    qCDebug(logBasicWidgets) << "Calculating layout rect";
    QRectF cr = documentRect(d);
    qCDebug(logBasicWidgets) << "Document rect:" << cr;

    if (!d->control) {
        qCDebug(logBasicWidgets) << "No text control, returning document rect";
        return cr;
    }

    ensureTextLayouted(d);
    // Calculate y position manually
    qreal rh = d->control->document()->documentLayout()->documentSize().height();
    qCDebug(logBasicWidgets) << "Document height:" << rh;

    qreal yo = 0;
    if (d->align & Qt::AlignVCenter) {
        yo = qMax((cr.height()-rh)/2, qreal(0));
        qCDebug(logBasicWidgets) << "Vertical center alignment, y offset:" << yo;
    } else if (d->align & Qt::AlignBottom) {
        yo = qMax(cr.height()-rh, qreal(0));
        qCDebug(logBasicWidgets) << "Bottom alignment, y offset:" << yo;
    }

    const auto& result = QRectF(cr.x(), yo + cr.y(), cr.width(), cr.height());
    qCDebug(logBasicWidgets) << "Final layout rect:" << result;
    return result;
}

void DLabelPrivate::ensureTextLayouted(QLabelPrivate *d)
{
    qCDebug(logBasicWidgets) << "Ensuring text is properly laid out";
    if (d->textLayoutDirty) {
        qCDebug(logBasicWidgets) << "Text layout is dirty";
        if (d->textDirty) {
            qCDebug(logBasicWidgets) << "Text content is dirty";
            if (d->control) {
                qCDebug(logBasicWidgets) << "Updating text control content";
                QTextDocument *doc = d->control->document();
                if (d->textDirty) {
        #ifndef QT_NO_TEXTHTMLPARSER
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
                    if (d->textformat == Qt::TextFormat::RichText) {
                        qCDebug(logBasicWidgets) << "Setting HTML text";
                        doc->setHtml(d->text);
                    } else {
                        qCDebug(logBasicWidgets) << "Setting plain text";
                        doc->setPlainText(d->text);
                    }
        #else
                    if (d->isRichText) {
                        qCDebug(logBasicWidgets) << "Setting HTML text";
                        doc->setHtml(d->text);
                    } else {
                        qCDebug(logBasicWidgets) << "Setting plain text";
                        doc->setPlainText(d->text);
                    }
        #endif
        #else
                    qCDebug(logBasicWidgets) << "Setting plain text (no HTML support)";
                    doc->setPlainText(d->text);
        #endif
                    doc->setUndoRedoEnabled(false);

        #ifndef QT_NO_SHORTCUT
                    if (d->hasShortcut) {
                        qCDebug(logBasicWidgets) << "Processing shortcut underlines";
                        // Underline the first character that follows an ampersand (and remove the others ampersands)
                        int from = 0;
                        bool found = false;
                        QTextCursor cursor;
                        while (!(cursor = d->control->document()->find((QLatin1String("&")), from)).isNull()) {
                            qCDebug(logBasicWidgets) << "Found ampersand at position:" << cursor.position();
                            cursor.deleteChar(); // remove the ampersand
                            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
                            from = cursor.position();
                            if (!found && cursor.selectedText() != QLatin1String("&")) { //not a second &
                                qCDebug(logBasicWidgets) << "Found shortcut character:" << cursor.selectedText();
                                found = true;
                                d->shortcutCursor = cursor;
                            }
                        }
                    }
        #endif
                }
            }
            qCDebug(logBasicWidgets) << "Text content updated";
            d->textDirty = false;
        }

        if (d->control) {
            qCDebug(logBasicWidgets) << "Updating text layout options";
            QTextDocument *doc = d->control->document();
            QTextOption opt = doc->defaultTextOption();

            opt.setAlignment(QFlag(d->align));
            qCDebug(logBasicWidgets) << "Setting text alignment:" << d->align;

            if (d->align & Qt::TextWordWrap) {
                qCDebug(logBasicWidgets) << "Setting word wrap mode";
                opt.setWrapMode(QTextOption::WordWrap);
            } else {
                qCDebug(logBasicWidgets) << "Setting manual wrap mode";
                opt.setWrapMode(QTextOption::ManualWrap);
            }

            doc->setDefaultTextOption(opt);

            QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
            fmt.setMargin(0);
            qCDebug(logBasicWidgets) << "Setting root frame margin to 0";
            doc->rootFrame()->setFrameFormat(fmt);

            const auto& width = DLabelPrivate::documentRect(d).width();
            qCDebug(logBasicWidgets) << "Setting text width to:" << width;
            doc->setTextWidth(width);
        }
        qCDebug(logBasicWidgets) << "Text layout updated";
        d->textLayoutDirty = false;
    }
}

DWIDGET_END_NAMESPACE
