// SPDX-FileCopyrightText: 2022 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "private/dmessagemanager_p.h"

#include <DFloatingMessage>
#include <DDciIcon>

#include <QVBoxLayout>
#include <QEvent>
#include <QPainter>

DWIDGET_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(logMessageNotification, "dtk.widgets.message")

#define D_MESSAGE_MANAGER_CONTENT "_d_message_manager_content"
const int MARGIN = 20;
const int MESSGAE_HEIGHT = 50;
const int ANIMATION_DURATION = 400;

Q_DECLARE_METATYPE(QMargins)

class ImageLabel : public QLabel {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    explicit ImageLabel(QWidget *parent=nullptr)
        : QLabel (parent)
        , m_opacity(0)
    {
        qCDebug(logMessageNotification) << "ImageLabel constructor called";
    };
    void setOpacity(qreal opac) { 
        qCDebug(logMessageNotification) << "Setting opacity to:" << opac;
        m_opacity = opac; 
    }
    qreal opacity() { 
        qCDebug(logMessageNotification) << "Getting opacity:" << m_opacity;
        return m_opacity ;
    }

protected:
    void paintEvent(QPaintEvent *e) override
    {
        Q_UNUSED(e)
        qCDebug(logMessageNotification) << "ImageLabel paintEvent called";
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setOpacity(m_opacity);
        p.drawPixmap(rect().marginsRemoved(contentsMargins()), pixmap(Qt::ReturnByValue));
    };
private:
    qreal m_opacity;
};

// 仅仅为了接口兼容， 符号不会减少， 如果使用了这个接口，实际调用走的有namespace的
class Q_DECL_EXPORT DMessageManager: public QObject
{
    Q_OBJECT
private:
    DMessageManager();

public:
    static DMessageManager *instance();

    void sendMessage(QWidget *par, DTK_WIDGET_NAMESPACE::DFloatingMessage *floMsg);
    void sendMessage(QWidget *par, const QIcon &icon, const QString &message);
    bool setContentMargens(QWidget *par, const QMargins &margins);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

DMessageManager::DMessageManager()
{
    qCDebug(logMessageNotification) << "DMessageManager constructor called";
}

DMessageManager *DMessageManager::instance()
{
    qCDebug(logMessageNotification) << "DMessageManager instance called";
    return reinterpret_cast<DMessageManager *>(DTK_WIDGET_NAMESPACE::DMessageManager::instance());
}

void DMessageManager::sendMessage(QWidget *par, DTK_WIDGET_NAMESPACE::DFloatingMessage *floMsg)
{
    qCDebug(logMessageNotification) << "Sending floating message to parent:" << par;
    DTK_WIDGET_NAMESPACE::DMessageManager::instance()->sendMessage(par, floMsg);
}

void DMessageManager::sendMessage(QWidget *par, const QIcon &icon, const QString &message)
{
    qCDebug(logMessageNotification) << "Sending message with icon to parent:" << par << "message:" << message;
    DTK_WIDGET_NAMESPACE::DMessageManager::instance()->sendMessage(par, icon, message);
}

bool DMessageManager::setContentMargens(QWidget *par, const QMargins &margins)
{
    qCDebug(logMessageNotification) << "Setting content margins for parent:" << par << "margins:" << margins;
    return DTK_WIDGET_NAMESPACE::DMessageManager::instance()->setContentMargens(par, margins);
}

bool DMessageManager::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
    Q_UNUSED(event);
    return false;
}

DWIDGET_BEGIN_NAMESPACE
template<typename IconType>
static void sendMessage_helper(DMessageManager *manager, QWidget *par, IconType icon, const QString &message)
{
    qCDebug(logMessageNotification) << "sendMessage_helper called with message:" << message;
    QWidget *content = par->findChild<QWidget *>(D_MESSAGE_MANAGER_CONTENT, Qt::FindDirectChildrenOnly);

    if (content) {
        int text_message_count = 0;

        for (DFloatingMessage *message : content->findChildren<DFloatingMessage *>(QString(), Qt::FindDirectChildrenOnly)) {
            if (message->messageType() == DFloatingMessage::TransientType) {
                ++text_message_count;
            }
        }

        // TransientType 类型的通知消息，最多只允许同时显示三个
        if (text_message_count >= 3) {
            qCDebug(logMessageNotification) << "Maximum transient messages reached (3), skipping";
            return;
        }
    }

    DFloatingMessage *floMsg = new DFloatingMessage(DFloatingMessage::TransientType);
    floMsg->setAttribute(Qt::WA_DeleteOnClose);
    floMsg->setIcon(icon);
    floMsg->setMessage(message);
    manager->sendMessage(par, floMsg);
}

DMessageManagerPrivate::DMessageManagerPrivate(DMessageManager *qq)
    : DObjectPrivate(qq)
    , m_aniGeometry(nullptr)
    , m_aniOpacity(nullptr)
    , m_aniGroup(nullptr)
    , m_label(nullptr)
{
    qCDebug(logMessageNotification) << "DMessageManagerPrivate constructor called";
    if (!ENABLE_ANIMATIONS || !ENABLE_ANIMATION_MESSAGE)
        return;

    m_aniGeometry = new QPropertyAnimation(qq);
    m_aniOpacity = new QPropertyAnimation(qq);
    m_aniGroup = new QParallelAnimationGroup(qq);
    m_label = new ImageLabel;

    m_aniGeometry->setPropertyName("geometry");
    m_aniGeometry->setDuration(ANIMATION_DURATION);
    m_aniGeometry->setEasingCurve(QEasingCurve::OutCubic);

    m_aniOpacity->setPropertyName("opacity");
    m_aniOpacity->setDuration(ANIMATION_DURATION);
    m_aniOpacity->setEasingCurve(QEasingCurve::OutCubic);
    m_aniOpacity->setTargetObject(m_label);
    m_aniOpacity->setStartValue(0);
    m_aniOpacity->setEndValue(1);

    m_aniGroup->addAnimation(m_aniGeometry);
    m_aniGroup->addAnimation(m_aniOpacity);
}

DMessageManager::DMessageManager()               //私有静态构造函数
    : DObject(*new DMessageManagerPrivate(this))
{
    qCDebug(logMessageNotification) << "DMessageManager constructor called";
}

/*!
@~english
 @brief DMessageManager::instance constructs a singleton object.
 @return Returns a reference to the object.
 */
DMessageManager *DMessageManager::instance()  //公有静态函数
{
    qCDebug(logMessageNotification) << "DMessageManager instance called";
    static DMessageManager instance;             //局部静态变量
    return &instance;
}

/*!
@~english
 @brief DMessageManager::sendMessage sends a popup message.
 @param[in] par The parent widget.
 @param[in] floMsg The floating message (widget).
 */
void DMessageManager::sendMessage(QWidget *par, DFloatingMessage *floMsg)
{
    D_D(DMessageManager);
    qCDebug(logMessageNotification) << "Sending floating message to parent:" << par;
    QWidget *content = par->findChild<QWidget *>(D_MESSAGE_MANAGER_CONTENT, Qt::FindDirectChildrenOnly);

    if (!content) {
        qCDebug(logMessageNotification) << "Creating new content widget";
        content = new QWidget(par);
        content->setObjectName(D_MESSAGE_MANAGER_CONTENT);
        content->setAttribute(Qt::WA_AlwaysStackOnTop);

        QMargins magins = par->property("_d_margins").value<QMargins>();
        if (par->property("_d_margins").isValid()) {
            qCDebug(logMessageNotification) << "Using custom margins:" << magins;
            content->setContentsMargins(magins);
        } else {
            qCDebug(logMessageNotification) << "Using default margins:" << MARGIN;
            content->setContentsMargins(QMargins(MARGIN, 0, MARGIN, 0));
        }

        qCDebug(logMessageNotification) << "Installing event filters";
        content->installEventFilter(this);
        par->installEventFilter(this);
        QVBoxLayout *layout = new QVBoxLayout(content);
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setDirection(QBoxLayout::BottomToTop);
    }
    if (ENABLE_ANIMATIONS && ENABLE_ANIMATION_MESSAGE) {
        if (content->layout()->count() >= 1) {
            content->layout()->itemAt(content->layout()->count() - 1)->widget()->hide();
            delete content->layout()->takeAt(content->layout()->count() - 1);
        }
    } else {
        content->show();
    }

    static_cast<QBoxLayout*>(content->layout())->addWidget(floMsg, 0, Qt::AlignHCenter);

    if (!ENABLE_ANIMATIONS || !ENABLE_ANIMATION_MESSAGE)
        return;

    // 限制通知消息的最大宽度
    for (DFloatingMessage *message : content->findChildren<DFloatingMessage*>(QString(), Qt::FindDirectChildrenOnly)) {
        message->setMaximumWidth(par->rect().marginsRemoved(content->contentsMargins()).width());
        message->setMinimumHeight(message->sizeHint().height());
    }

    QRect geometry(QPoint(0, 0), floMsg->sizeHint() + QSize(MARGIN * 2, 0));
    geometry.moveCenter(par->rect().center());
    geometry.moveBottom(par->rect().bottom() - MESSGAE_HEIGHT);

    content->setGeometry(geometry);
    content->hide();

    if (d->m_aniGeometry->state() == QPropertyAnimation::State::Running)
        return;

    d->m_label->setParent(par);
    d->m_label->setAlignment(Qt::AlignCenter);
    d->m_label->setContentsMargins(MARGIN, 0, MARGIN, 0);

    if (floMsg && !floMsg->grab().isNull())
        d->m_label->setPixmap(floMsg->grab());

    d->m_label->setScaledContents(true);
    d->m_label->show();
    d->m_aniGeometry->setTargetObject(d->m_label);
    d->m_aniOpacity->setTargetObject(d->m_label);
    d->m_aniGeometry->setStartValue(QRect(par->rect().center().x(), par->rect().bottom(), 0, 0));
    d->m_aniGeometry->setEndValue(content->geometry());
    d->m_aniGroup->setDirection(QAbstractAnimation::Forward);
    d->m_aniGroup->start();
    connect(d->m_aniGroup, &QPropertyAnimation::finished, this, [d, this, content]() {
        content->show();
        d->m_label->hide();
        disconnect(d->m_aniGroup, &QPropertyAnimation::finished, this, nullptr);
    });

    connect(floMsg, &DFloatingMessage::messageClosed, [=]() {
        d->m_aniGeometry->setStartValue(QRect(par->rect().center().x(), par->rect().bottom(), 0, 0));
        d->m_aniGeometry->setEndValue(content->geometry());

        if (floMsg && !floMsg->grab().isNull())
            d->m_label->setPixmap(floMsg->grab());

        d->m_label->show();
        d->m_aniGroup->setDirection(QAbstractAnimation::Backward);
        d->m_aniGroup->start();
        content->hide();
    });
}

/*!
@~english
 @brief DMessageManager::sendMessage sends a popup message.
 @param[in] par The parent widget.
 @param[in] icon The message icon.
 @param[in] message The message text.
 */
void DMessageManager::sendMessage(QWidget *par, const QIcon &icon, const QString &message)
{
    qCDebug(logMessageNotification) << "Sending message with QIcon to parent:" << par << "message:" << message;
    if (!par) {
        qCDebug(logMessageNotification) << "Parent widget is null, cannot send message";
        return;
    }
    sendMessage_helper(instance(), par, icon, message);
}

void DMessageManager::sendMessage(QWidget *par, const DDciIcon &icon, const QString &message)
{
    qCDebug(logMessageNotification) << "Sending message with DDciIcon to parent:" << par << "message:" << message;
    if (!par) {
        qCDebug(logMessageNotification) << "Parent widget is null, cannot send message";
        return;
    }
    sendMessage_helper(instance(), par, icon, message);
}

/*!
@~english
 @brief DMessageManager::setContentMargins sets the margin values for the content.
 @param[in] par The parent widget.
 @param[in] margins The margin values for the four sides.
 @return Whether the setting was successful.
 */
bool DMessageManager::setContentMargens(QWidget *par, const QMargins &margins)
{
    qCDebug(logMessageNotification) << "Setting content margins:" << margins << "for parent:" << par;
    
    if (!par) {
        qCDebug(logMessageNotification) << "Parent widget is null, cannot set margins";
        return false;
    }
    
    QWidget *content = par->findChild<QWidget *>(D_MESSAGE_MANAGER_CONTENT, Qt::FindDirectChildrenOnly);
    if (content) {
        qCDebug(logMessageNotification) << "Content widget found, applying margins immediately";
        QMargins currentMargins = content->contentsMargins();
        if (currentMargins == margins) {
            qCDebug(logMessageNotification) << "Margins unchanged, skipping update";
            return true;
        }
        content->setContentsMargins(margins);
        qCDebug(logMessageNotification) << "Margins applied successfully";
        return true;
    } else {
        qCDebug(logMessageNotification) << "Content widget not found, storing margins as property";
        par->setProperty("_d_margins", QVariant::fromValue(margins));
        return false;
    }
}

/*!
@~english
 @brief DMessageManager::eventFilter message filtering.
 @param[in] watched The watched object.
 @param[in] event The message event.
 @return Whether the operation was successful.
 */
bool DMessageManager::eventFilter(QObject *watched, QEvent *event)
{
    bool isOnlyResizeEvent = ENABLE_ANIMATIONS && ENABLE_ANIMATION_MESSAGE ?  event->type() == QEvent::Resize : event->type() == QEvent::LayoutRequest || event->type() == QEvent::Resize;

    if (isOnlyResizeEvent) {
        if (ENABLE_ANIMATIONS && ENABLE_ANIMATION_MESSAGE) {
            if (auto content = watched->findChild<QWidget *>(D_MESSAGE_MANAGER_CONTENT, Qt::FindDirectChildrenOnly)) {

                auto par = qobject_cast<QWidget *>(watched);

                for (DFloatingMessage *message : content->findChildren<DFloatingMessage*>(QString(), Qt::FindDirectChildrenOnly)) {
                    message->setMaximumWidth(par->rect().marginsRemoved(content->contentsMargins()).width());
                    message->setMinimumHeight(message->sizeHint().height());
                }

                QRect geometry(QPoint(0, 0), content->sizeHint());
                geometry.moveCenter(par->rect().center());
                geometry.moveBottom(par->rect().bottom() - MESSGAE_HEIGHT);
                content->setGeometry(geometry);
            }
        } else {
            if (QWidget *widget = qobject_cast<QWidget *>(watched)) {
                QWidget *content;

                if (widget->objectName() == D_MESSAGE_MANAGER_CONTENT) {
                    content = widget;
                } else {
                    content = widget->findChild<QWidget*>(D_MESSAGE_MANAGER_CONTENT, Qt::FindDirectChildrenOnly);
                }

                QWidget *par = content->parentWidget();

                // 限制通知消息的最大宽度
                for (DFloatingMessage *message : content->findChildren<DFloatingMessage*>(QString(), Qt::FindDirectChildrenOnly)) {
                    message->setMaximumWidth(par->rect().marginsRemoved(content->contentsMargins()).width());
                    message->setMinimumHeight(message->sizeHint().height());
                }

                QRect geometry(QPoint(0, 0), content->sizeHint());
                geometry.moveCenter(par->rect().center());
                geometry.moveBottom(par->rect().bottom());
                content->setGeometry(geometry);
            }
        }
    } else if (event->type() == QEvent::ChildRemoved) {
        // 如果是通知消息被删除的事件
        if (QWidget *widget = qobject_cast<QWidget*>(watched)) {
            if (widget->objectName() == D_MESSAGE_MANAGER_CONTENT
                    && widget->layout() && widget->layout()->count() == 0) {
                widget->parent()->removeEventFilter(this);
                widget->deleteLater();
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

DWIDGET_END_NAMESPACE

#include "dmessagemanager.moc"
