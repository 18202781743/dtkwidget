// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dfiledialog.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QLoggingCategory>

#ifndef foreach
#define foreach Q_FOREACH
#endif
#include <private/qfiledialog_p.h>
#include <qpa/qplatformdialoghelper.h>

DWIDGET_BEGIN_NAMESPACE

namespace {
Q_DECLARE_LOGGING_CATEGORY(logDialogs)
}

/*!
@~english
@class Dtk::Widget::DFileDialog

  @brief The DFileDialog class provides a dialog that allow users to select
  files or directories.
  You can also add extra ComboBox and LineEdit widget via addComboBox() and
  addLineEdit() to allowed user fill more field when needed. Values of these
  extra fields can be accessed via getComboBoxValue() and getLineEditValue() .

@image html DFileDialog.png
*/

DFileDialog::DFileDialog(QWidget *parent, Qt::WindowFlags f)
    : QFileDialog(parent, f)
{
    qCDebug(logDialogs) << "Creating file dialog with flags";
}

DFileDialog::DFileDialog(QWidget *parent, const QString &caption,
                         const QString &directory, const QString &filter)
    : QFileDialog(parent, caption, directory, filter)
{
    qCDebug(logDialogs) << "Creating file dialog with caption:" << caption
                        << "directory:" << directory;
}

/*!
@~english
  @brief Add an extra ComboBox widget to the DFileDialog
   text ComboBox description text (as key for getting value).
   data ComboBox options in a string list
 */
void DFileDialog::addComboBox(const QString &text, const QStringList &data)
{
    qCDebug(logDialogs) << "Adding combobox:" << text << "with" << data.size() << "items";
    if (data.isEmpty()) {
        qCWarning(logDialogs) << "Empty data provided for combobox:" << text;
        return;
    }

    addComboBox(text, DComboBoxOptions {
                    false, data, data.first()
                });
}

/*!
@~english
  @brief Add an extra ComboBox widget to the DFileDialog
   text ComboBox description text (as key for getting value).
   options ComboBox data
 */
void DFileDialog::addComboBox(const QString &text, const DFileDialog::DComboBoxOptions &options)
{
    qCDebug(logDialogs) << "Adding combobox with options:" << text
                        << "editable:" << options.editable;
    QJsonObject json;

    json["text"] = text;
    json["editable"] = options.editable;
    json["data"] = QJsonArray::fromStringList(options.data);
    json["defaultValue"] = options.defaultValue;

    QStringList list = property("_dtk_widget_custom_combobox_list").toStringList();

    list << QJsonDocument(json).toJson(QJsonDocument::Compact);
    setProperty("_dtk_widget_custom_combobox_list", list);
}

/*!
@~english
  @brief Add an extra LineEdit widget to the DFileDialog
   text LineEdit description text (as key for getting value).
 */
void DFileDialog::addLineEdit(const QString &text)
{
    qCDebug(logDialogs) << "Adding simple lineedit:" << text;
    addLineEdit(text, DLineEditOptions {
                    -1, QLineEdit::Normal, QString(), QString(), QString()
                });
}

/*!
@~english
  @brief Add an extra LineEdit widget to the DFileDialog
   text LineEdit description text (as key for getting value).
   options LineEdit data
 */
void DFileDialog::addLineEdit(const QString &text, const DFileDialog::DLineEditOptions &options)
{
    qCDebug(logDialogs) << "Adding lineedit with options:" << text
                        << "maxLength:" << options.maxLength;
    QJsonObject json;

    json["text"] = text;
    json["maxLength"] = options.maxLength;
    json["echoMode"] = options.echoMode;
    json["defaultValue"] = options.defaultValue;
    json["inputMask"] = options.inputMask;
    json["placeholderText"] = options.placeholderText;

    QStringList list = property("_dtk_widget_custom_lineedit_list").toStringList();

    list << QJsonDocument(json).toJson(QJsonDocument::Compact);
    setProperty("_dtk_widget_custom_lineedit_list", list);
}

/*!
@~english
  @brief Allow mixed selection
  @details
  Allow user choose files and folders at the same time when selecting multiple files. By
  default user can only select files (folder not included) when selecting multiple files.
  Notice that this option only works when file mode is QFileDialog::ExistingFiles
   on enable this feature or not.
 */
void DFileDialog::setAllowMixedSelection(bool on)
{
    qCDebug(logDialogs) << "Setting mixed selection:" << on;
    setProperty("_dtk_widget_filedialog_mixed_selection", on);
}

/*!
@~english
  @brief Get the added extra ComboBox value
   text The description (key) of the ComboBox.
  @sa addComboBox()
 */
QString DFileDialog::getComboBoxValue(const QString &text) const
{
    const auto& value = property(QString("_dtk_widget_combobox_%1_value").arg(text).toUtf8()).toString();
    qCDebug(logDialogs) << "Getting combobox value for:" << text << "result:" << value;
    return value;
}

/*!
@~english
  @brief Get the added extra LineEdit value
   text The description (key) of the ComboBox.
  @sa addLineEdit()
 */
QString DFileDialog::getLineEditValue(const QString &text) const
{
    const auto& value = property(QString("_dtk_widget_lineedit_%1_value").arg(text).toUtf8()).toString();
    qCDebug(logDialogs) << "Getting lineedit value for:" << text << "result:" << value;
    return value;
}

void DFileDialog::setVisible(bool visible)
{
    qCDebug(logDialogs) << "Setting dialog visibility:" << visible;
    QFileDialogPrivate *d = static_cast<QFileDialogPrivate*>(d_ptr.data());

    if (d->canBeNativeDialog()) {
        qCDebug(logDialogs) << "Using native dialog";
        QPlatformDialogHelper *helper = d->platformHelper();

        if (helper) {
            helper->setProperty("_dtk_widget_QFileDialog", reinterpret_cast<quintptr>(this));
        } else {
            qCWarning(logDialogs) << "Platform helper not available";
        }
    }

    QFileDialog::setVisible(visible);
}

DWIDGET_END_NAMESPACE
