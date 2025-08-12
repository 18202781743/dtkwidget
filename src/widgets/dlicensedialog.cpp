// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dlicensedialog.h"
#include "private/dabstractdialogprivate_p.h"

#include <DFontSizeManager>
#include <DIconButton>
#include <DTitlebar>
#include <DListView>
#include <DHorizontalLine>
#include <DLicenseInfo>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logDialogs, "dtk.widgets.dialogs")

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QStackedLayout>

DCORE_USE_NAMESPACE

DWIDGET_BEGIN_NAMESPACE

class DLicenseDialogPrivate : public DAbstractDialogPrivate
{
public:
    explicit DLicenseDialogPrivate(DLicenseDialog *qq);

private:
    void init();
    void addComponentItem(const DLicenseInfo::DComponentInfo *DComponentInfo);
    bool loadLicense();

    DTitlebar          *titleBar = nullptr;
    DIconButton        *backwardBtn = nullptr;
    DListView          *listView = nullptr;
    QStandardItemModel *listModel = nullptr;
    QStackedLayout     *stackedLayout = nullptr;
    QLabel             *componentNameLabel = nullptr;
    QLabel             *componentVersionLabel = nullptr;
    QLabel             *copyRightLabel = nullptr;
    QLabel             *licenseContentLabel = nullptr;
    QScrollArea        *scrollArea = nullptr;
    QByteArray         content;
    QString            path;
    DLicenseInfo       licenseInfo;
    bool isValid = false;

private:
    Q_DECLARE_PUBLIC(DLicenseDialog)
};

DLicenseDialogPrivate::DLicenseDialogPrivate(DLicenseDialog *qq)
    : DAbstractDialogPrivate(qq)
    , backwardBtn(new DIconButton(QStyle::SP_ArrowBack))
    , listView(new DListView)
    , listModel(new QStandardItemModel(listView))
    , stackedLayout(new QStackedLayout)
    , componentNameLabel(new QLabel)
    , componentVersionLabel(new QLabel)
    , copyRightLabel(new QLabel)
    , licenseContentLabel(new QLabel)
    , scrollArea(new QScrollArea)
{
    qCDebug(logDialogs) << "Creating DLicenseDialogPrivate";
}

void DLicenseDialogPrivate::init()
{
    qCDebug(logDialogs) << "Initializing DLicenseDialog";
    D_Q(DLicenseDialog);
    q->setFixedSize(900, 800);

    titleBar = new DTitlebar();
    titleBar->setAccessibleName("DLicenseDialogTitleBar");
    titleBar->setMenuVisible(false);
    titleBar->setBackgroundTransparent(true);
    titleBar->setTitle(QObject::tr("Open-Source Software"));
    titleBar->addWidget(backwardBtn, Qt::AlignLeft | Qt::AlignVCenter);

    backwardBtn->setVisible(false);

    listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    listView->setSelectionMode(QAbstractItemView::NoSelection);
    listView->setSpacing(0);
    listView->setItemSpacing(0);
    listView->setModel(listModel);
    listView->setAlternatingRowColors(true);

    DFontSizeManager *fontManager =  DFontSizeManager::instance();
    fontManager->bind(componentNameLabel, DFontSizeManager::T4, QFont::Bold);
    fontManager->bind(componentVersionLabel, DFontSizeManager::T6, QFont::DemiBold);
    fontManager->bind(copyRightLabel, DFontSizeManager::T6, QFont::DemiBold);

    licenseContentLabel->setWordWrap(true);

    QWidget *licenseWidget = new QWidget;
    QVBoxLayout *licenseLayout = new QVBoxLayout(licenseWidget);
    licenseLayout->setSpacing(0);
    licenseLayout->setContentsMargins(20, 20, 20, 20);
    licenseLayout->addWidget(componentNameLabel);
    licenseLayout->addSpacing(16);
    licenseLayout->addWidget(new DHorizontalLine);
    licenseLayout->addSpacing(16);
    licenseLayout->addWidget(componentVersionLabel);
    licenseLayout->addWidget(copyRightLabel);
    licenseLayout->addSpacing(40);
    licenseLayout->addWidget(licenseContentLabel);
    licenseLayout->addStretch(0);

    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->viewport()->setAutoFillBackground(false);
    scrollArea->setContentsMargins(QMargins(0,0,0,0));
    scrollArea->viewport()->setContentsMargins(QMargins(0,0,0,0));
    scrollArea->setWidget(licenseWidget);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidgetResizable(true);
    licenseWidget->setAutoFillBackground(false);

    stackedLayout->addWidget(listView);
    stackedLayout->addWidget(scrollArea);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(10, 0, 10, 0);
    mainLayout->addWidget(titleBar, 0, Qt::AlignTop);
    mainLayout->addLayout(stackedLayout);

    q->setLayout(mainLayout);
    q->setFocusPolicy(Qt::ClickFocus);
    q->setFocus();

    QObject::connect(stackedLayout, &QStackedLayout::currentChanged, q, [this](int index) {
        backwardBtn->setVisible(index != 0);
    });
    QObject::connect(backwardBtn, &QAbstractButton::clicked, q, [this]{
        scrollArea->horizontalScrollBar()->setValue(0);
        scrollArea->verticalScrollBar()->setValue(0);
        stackedLayout->setCurrentIndex(0);
    });
    QObject::connect(listView, &QAbstractItemView::clicked, q, [this](const QModelIndex &index) {
        const auto &components = licenseInfo.componentInfos();
        if (components.size() <= index.row() || index.row() < 0)
            return;

        auto componentInfo = components.at(index.row());

        componentNameLabel->setText(componentInfo->name());
        componentVersionLabel->setText(componentInfo->version());
        copyRightLabel->setText(componentInfo->copyRight());
        licenseContentLabel->setText(licenseInfo.licenseContent(componentInfo->licenseName()));
        stackedLayout->setCurrentIndex(1);
    });
}

void DLicenseDialogPrivate::addComponentItem(const DLicenseInfo::DComponentInfo *componentInfo)
{
    qCDebug(logDialogs) << "Adding component item:" << componentInfo->name();
    D_Q(DLicenseDialog);
    auto pItem = new DStandardItem(componentInfo->name());
    pItem->setEditable(false);
    QSize size(12, 12);
    DViewItemAction *enterAction = new DViewItemAction(Qt::AlignVCenter, size, size, true);
    enterAction->setIcon(DStyle::standardIcon(q->style(), DStyle::SP_ArrowEnter));
    pItem->setActionList(Qt::RightEdge, DViewItemActionList() << enterAction);
    listModel->appendRow(pItem);
    const auto index  = pItem->index();
    QObject::connect(enterAction, &DViewItemAction::triggered, enterAction, [this, index] {
        Q_EMIT listView->clicked(index);
    });
}

bool DLicenseDialogPrivate::loadLicense()
{
    qCDebug(logDialogs) << "Loading license, content size:" << content.size() << "path:" << path;
    if (!content.isEmpty()) {
        isValid = licenseInfo.loadContent(content);
    } else if (!path.isEmpty()) {
        isValid = licenseInfo.loadFile(path);
    }
    if (isValid) {
        listModel->clear();
        for (auto component : licenseInfo.componentInfos()) {
            addComponentItem(component);
        }
    }
    return isValid;
}

DLicenseDialog::DLicenseDialog(QWidget *parent)
    : DAbstractDialog(*new DLicenseDialogPrivate(this), parent)
{
    qCDebug(logDialogs) << "Creating DLicenseDialog";
    Q_D(DLicenseDialog);
    d->init();
}

DLicenseDialog::~DLicenseDialog()
{
    qCDebug(logDialogs) << "Destroying DLicenseDialog";
}

void DLicenseDialog::setContent(const QByteArray &content)
{
    qCDebug(logDialogs) << "Setting content, size:" << content.size();
    D_D(DLicenseDialog);
    d->content = content;
}

void DLicenseDialog::setFile(const QString &file)
{
    qCDebug(logDialogs) << "Setting file:" << file;
    D_D(DLicenseDialog);
    d->path = file;
}

void DLicenseDialog::setLicenseSearchPath(const QString &path)
{
    qCDebug(logDialogs) << "Setting license search path:" << path;
    D_D(DLicenseDialog);
    d->licenseInfo.setLicenseSearchPath(path);
}

bool DLicenseDialog::load()
{
    qCDebug(logDialogs) << "Loading license dialog";
    D_D(DLicenseDialog);
    return d->loadLicense();
}

bool DLicenseDialog::isValid() const
{
    qCDebug(logDialogs) << "Checking if valid:" << d->isValid;
    D_DC(DLicenseDialog);
    return d->isValid;
}

void DLicenseDialog::hideEvent(QHideEvent *)
{
    qCDebug(logDialogs) << "Hide event";
    D_D(DLicenseDialog);
    d->backwardBtn->setVisible(false);
    d->stackedLayout->setCurrentIndex(0);
    d->scrollArea->horizontalScrollBar()->setValue(0);
    d->scrollArea->verticalScrollBar()->setValue(0);
}
DWIDGET_END_NAMESPACE
#include "moc_dlicensedialog.cpp"
