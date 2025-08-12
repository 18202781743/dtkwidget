// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dfilechooseredit.h"
#include "private/dfilechooseredit_p.h"

#include <DStyleHelper>
#include <DSuggestButton>

#include <QScreen>
#include <QGuiApplication>
#include <private/qguiapplication_p.h>
#include <QWindow>
#include <QLoggingCategory>

DWIDGET_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(logBasicWidgets, "dtk.widgets.basic")

/*!
@~english
  @class Dtk::Widget::DFileChooserEdit
  \inmodule dtkwidget
  @brief A text editing box with a button to select a file.
  
  This control is basically the same as DLineEdit, but at the same time provides a button on the right side of the edit box, click the button will appear a select file dialog box, when the selection is completed in the dialog box click OK, the selection result will appear in the text edit box.
  There are also ways to customize the functionality of the control by setting where the dialog box appears, selecting the type of file, or setting a filename filter.
  
  @sa DLineEdit QFileDialog
 */

/*!
@~english
  @enum Dtk::Widget::DFileChooserEdit::DialogDisplayPosition
  @brief This enum holds the locations where the dialog box can appear
  
  \value FollowParentWindow
  Following the parent window
  
  \value CurrentMonitorCenter
  The center of the display where the mouse is located
 */

// =========================Signals begin=========================
/*!
@~english
  @fn void DFileChooserEdit::fileChoosed(const QString &fileName)
  @brief This signal is called after the file is selected and the Confirm button of the dialog box is clicked
  \a fileName The name of the selected file, including its absolute path.
 */

/*!
@~english
  @fn void DFileChooserEdit::dialogOpened()
  @brief This signal is called when the dialog box is about to be displayed
  @note Notice that the dialog box is not displayed
 */

/*!
@~english
  @fn void DFileChooserEdit::dialogClosed(int code)
  @brief This signal is called when the dialog box is closed, whether the dialog box is clicked to confirm or cancel, this signal is called
  \a code The return code of the dialog box, which indicates whether the dialog box was closed by clicking Cancel or Confirm
  @sa QDialog::DialogCode
 */
// =========================Signals end=========================

/*!
@~english
  @brief Gets an instance of DFileChooserEdit
  \a parent As the parent control of the instance
 */
DFileChooserEdit::DFileChooserEdit(QWidget *parent)
    : DLineEdit(*new DFileChooserEditPrivate(this), parent)
{
    qCDebug(logBasicWidgets) << "Creating DFileChooserEdit with parent:" << parent;
    D_D(DFileChooserEdit);

    d->init();
}

/*!
@~english
  @brief This property holds the location where the file selection dialog box will appear
  
  Optional values for the enumeration DFileChooserEdit: : DialogDisplayPosition of values
  
  Getter: DFileChooserEdit::dialogDisplayPosition , Setter: DFileChooserEdit::setDialogDisplayPosition
  
  @sa DFileChooserEdit::DialogDisplayPosition
 */
DFileChooserEdit::DialogDisplayPosition DFileChooserEdit::dialogDisplayPosition() const
{
    D_DC(DFileChooserEdit);
    const auto& position = d->dialogDisplayPosition;
    qCDebug(logBasicWidgets) << "Current dialog display position:" << static_cast<int>(position);
    return position;
}

/*!
@~english
  @brief Set the position of the dialog box display.

  \a dialogDisplayPosition The position of the dialog box to display.

  @sa DFileChooserEdit::dialogDisplayPosition
 */
void DFileChooserEdit::setDialogDisplayPosition(DFileChooserEdit::DialogDisplayPosition dialogDisplayPosition)
{
    qCDebug(logBasicWidgets) << "Setting dialog display position:" << static_cast<int>(dialogDisplayPosition);
    D_D(DFileChooserEdit);

    d->dialogDisplayPosition = dialogDisplayPosition;
}

void DFileChooserEdit::setFileDialog(QFileDialog *fileDialog)
{
    qCDebug(logBasicWidgets) << "Setting file dialog:" << fileDialog;
    D_D(DFileChooserEdit);

    d->dialog = fileDialog;
}

QFileDialog *DFileChooserEdit::fileDialog() const
{
    D_DC(DFileChooserEdit);
    const auto& dialog = d->dialog;
    qCDebug(logBasicWidgets) << "Current file dialog:" << dialog;
    return dialog;
}

void DFileChooserEdit::initDialog()
{
    qCDebug(logBasicWidgets) << "Initializing file dialog";
    D_D(DFileChooserEdit);

    if (d->dialog) {
        qCDebug(logBasicWidgets) << "Dialog already exists, skipping initialization";
        return;
    }

    d->dialog = new QFileDialog(this);
    d->dialog->setAcceptMode(QFileDialog::AcceptOpen);
    d->dialog->setFileMode(QFileDialog::ExistingFile);
    qCDebug(logBasicWidgets) << "File dialog created with default settings";
}

/*!
@~english
  @brief Set the file selection mode
  \a mode Pattern to use
  @sa DFileChooserEdit::fileMode
 */
void DFileChooserEdit::setFileMode(QFileDialog::FileMode mode)
{
    qCDebug(logBasicWidgets) << "Setting file mode:" << static_cast<int>(mode);
    D_D(DFileChooserEdit);

    if (!d->dialog) {
        qCDebug(logBasicWidgets) << "Dialog not initialized, initializing first";
        initDialog();
    }
    d->dialog->setFileMode(mode);
}

/*!
@~english
  @brief Get dialog box to select file mode
  
  There are multiple selection modes, which means that the dialog box can have multiple displays or behaviors, 
  such as selecting a single file, selecting multiple files, or selecting a directory, etc. See QFileDialog::FileMode for details
  @return Returns the current selection mode
  @sa QFileDialog::FileMode
  @note Currently only support this control to choose a single file, even call DFileChooserEdit: : setFileMode set selection mode, 
  when there are multiple files in the dialog box is selected, take its first as a choice
 */
QFileDialog::FileMode DFileChooserEdit::fileMode() const
{
    D_DC(DFileChooserEdit);

    if (!d->dialog) {
        qCDebug(logBasicWidgets) << "Dialog not initialized, returning AnyFile mode";
        return QFileDialog::FileMode::AnyFile;
    }
    const auto& mode = d->dialog->fileMode();
    qCDebug(logBasicWidgets) << "Current file mode:" << static_cast<int>(mode);
    return mode;
}

/*!
@~english
  @brief Set the filename filter
  \a filters A list of filename filters to use
  @sa DFileChooserEdit::nameFilters
 */
void  DFileChooserEdit::setNameFilters(const QStringList &filters)
{
    qCDebug(logBasicWidgets) << "Setting name filters:" << filters;
    D_D(DFileChooserEdit);

    if (!d->dialog) {
        qCDebug(logBasicWidgets) << "Dialog not initialized, initializing first";
        initDialog();
    }
    d->dialog->setNameFilters(filters);
}

/*!
@~english
  @brief Filename filter
  
  By default, all files are selected.When the filename filter is set,
  only files with a filename matching the filename filter can be selected.
  For example, if "*.txt" is set, only files with the suffix "txt" can be selected,
  or if multiple filters are set: QStringList() << "text file (*.txt)" << "picture file (*.png);
  You will see a list of options at the bottom of the file selection dialog, 
  but note that only one option can be used at a time, which means you can't select both txt and png files at the same time
  @return Returns a list of the current filename filters
  @sa DFileChooserEdit::setNameFilters
 */
QStringList DFileChooserEdit::nameFilters() const
{
    D_DC(DFileChooserEdit);

    if (!d->dialog) {
        qCDebug(logBasicWidgets) << "Dialog not initialized, returning empty filter list";
        return QStringList();
    }
    const auto& filters = d->dialog->nameFilters();
    qCDebug(logBasicWidgets) << "Current name filters:" << filters;
    return filters;
}

void DFileChooserEdit::setDirectoryUrl(const QUrl &directory)
{
    qCDebug(logBasicWidgets) << "Setting directory URL:" << directory;
    D_D(DFileChooserEdit);

    if (!d->dialog) {
        qCDebug(logBasicWidgets) << "Dialog not initialized, initializing first";
        initDialog();
    }
    d->dialog->setDirectoryUrl(directory);
}

QUrl DFileChooserEdit::directoryUrl()
{
    D_D(DFileChooserEdit);

    if (!d->dialog) {
        qCDebug(logBasicWidgets) << "Dialog not initialized, initializing first";
        initDialog();
    }
    const auto& url = d->dialog->directoryUrl();
    qCDebug(logBasicWidgets) << "Current directory URL:" << url;
    return url;
}


DFileChooserEditPrivate::DFileChooserEditPrivate(DFileChooserEdit *q)
    : DLineEditPrivate(q)
{
    qCDebug(logBasicWidgets) << "Creating DFileChooserEditPrivate";
}

void DFileChooserEditPrivate::init()
{
    qCDebug(logBasicWidgets) << "Initializing DFileChooserEdit";
    D_Q(DFileChooserEdit);

    QList<QWidget *> list;
    DSuggestButton *btn = new DSuggestButton(nullptr);
    btn->setAccessibleName("DFileChooserEditSuggestButton");
    btn->setIcon(DStyleHelper(q->style()).standardIcon(DStyle::SP_SelectElement, nullptr));

    btn->setFixedWidth(defaultButtonWidth());
    btn->setIconSize(defaultIconSize());
    QObject::connect(DGUI_NAMESPACE::DGuiApplicationHelper::instance(), &DGUI_NAMESPACE::DGuiApplicationHelper::sizeModeChanged, btn, [btn]() {
        btn->setFixedWidth(defaultButtonWidth());
        btn->setIconSize(defaultIconSize());
    });

    q->setDialogDisplayPosition(DFileChooserEdit::DialogDisplayPosition::CurrentMonitorCenter);

    list.append(btn);

    q->setRightWidgets(list);
    q->setClearButtonEnabled(true);

    q->connect(btn, SIGNAL(clicked()), q, SLOT(_q_showFileChooserDialog()));
    qCDebug(logBasicWidgets) << "DFileChooserEdit initialization completed";
}

void DFileChooserEditPrivate::_q_showFileChooserDialog()
{
    qCDebug(logBasicWidgets) << "Showing file chooser dialog";
    D_Q(DFileChooserEdit);

    if (!dialog) {
        qCDebug(logBasicWidgets) << "Dialog not initialized, initializing first";
        q->initDialog();
    }

    if (dialogDisplayPosition == DFileChooserEdit::CurrentMonitorCenter) {
        qCDebug(logBasicWidgets) << "Positioning dialog at current monitor center";
        QPoint pos = QCursor::pos();

        for (QScreen *screen : qApp->screens()) {
            if (screen->geometry().contains(pos)) {
                QRect rect = dialog->geometry();
                rect.moveCenter(screen->geometry().center());
                dialog->move(rect.topLeft());
                break;
            }
        }
    }

    q->dialogOpened();

    if (!dialog) {
        qCWarning(logBasicWidgets) << "Failed to initialize file dialog";
        return;
    }

    // 多次打开时有时会出现 filedialog 不显示的问题 exec 前确保没显示否则不调用 helper->show
    if (dialog->isVisible()) {
        qCDebug(logBasicWidgets) << "Dialog is visible, hiding it first";
        dialog->setVisible(false);
    }

    int code = dialog->exec();
    qCDebug(logBasicWidgets) << "Dialog execution completed with code:" << code;

    if (code == QDialog::Accepted && !dialog->selectedFiles().isEmpty()) {
        const QString fileName = dialog->selectedFiles().first();
        qCDebug(logBasicWidgets) << "File selected:" << fileName;

        q->setText(fileName);
        Q_EMIT q->fileChoosed(fileName);
    }

    // exec 后如果 filedialog 还阻塞了应用就要隐藏掉
    if (qApp->modalWindow() == dialog->windowHandle()) {
        qCDebug(logBasicWidgets) << "Hiding modal window";
        QGuiApplicationPrivate::hideModalWindow(dialog->windowHandle());
    }

    q->dialogClosed(code);
}

DWIDGET_END_NAMESPACE

#include "moc_dfilechooseredit.cpp"
