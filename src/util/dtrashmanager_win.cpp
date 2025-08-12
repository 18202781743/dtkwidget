// SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtrashmanager.h"

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)

#include <DObjectPrivate>

#include <QDirIterator>
#include <QStorageInfo>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>

DWIDGET_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(logUtilClasses)

class DTrashManager_ : public DTrashManager {};
Q_GLOBAL_STATIC(DTrashManager_, globalTrashManager)

static QString getNotExistsFileName(const QString &fileName, const QString &targetPath)
{
    qCDebug(logUtilClasses) << "Getting non-existing file name for:" << fileName;
    QByteArray name = fileName.toUtf8();

    int index = name.lastIndexOf('.');
    QByteArray suffix;

    if (index >= 0)
    {
        suffix = name.mid(index);
    }

    if (suffix.size() > 200)
    {
        suffix = suffix.left(200);
    }

    name.chop(suffix.size());
    name = name.left(200 - suffix.size());

    while (QFile::exists(targetPath + "/" + name + suffix))
    {
        name = QCryptographicHash::hash(name, QCryptographicHash::Md5).toHex();
    }

    return QString::fromUtf8(name + suffix);
}

static bool renameFile(const QFileInfo &fileInfo, const QString &target, QString *errorString = NULL)
{
    qCDebug(logUtilClasses) << "Renaming file to:" << target;
    if (fileInfo.isFile() || fileInfo.isSymLink())
    {
        QFile file(fileInfo.filePath());

        if (!file.rename(target))
        {
            qCWarning(logUtilClasses) << "Failed to rename file:" << file.errorString();
            if (errorString)
            {
                *errorString = file.errorString();
            }

            return false;
        }

        return true;
    }
    else
    {
        QDirIterator iterator(fileInfo.filePath(),
                              QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);

        while (iterator.hasNext())
        {
            iterator.next();

            const QString newFile = iterator.filePath().replace(0, fileInfo.filePath().length(), target);

            if (!QDir().mkpath(QFileInfo(newFile).path()))
            {
                qCWarning(logUtilClasses) << "Failed to make path:" << QFileInfo(newFile).path();
                if (errorString)
                {
                    *errorString = QString("Make the %1 path is failed").arg(QFileInfo(newFile).path());
                }

                return false;
            }

            if (!renameFile(iterator.fileInfo(), newFile, errorString))
            {
                return false;
            }
        }

        if (!QDir().rmdir(fileInfo.filePath()))
        {
            qCWarning(logUtilClasses) << "Failed to remove directory:" << fileInfo.filePath();
            if (errorString)
            {
                *errorString = QString("Cannot remove the %1 dir").arg(fileInfo.filePath());
            }

            return false;
        }
    }

    return true;
}

class DTrashManagerPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    DTrashManagerPrivate(DTrashManager *q_ptr)
        : DObjectPrivate(q_ptr) {}

    D_DECLARE_PUBLIC(DTrashManager)
};

DTrashManager *DTrashManager::instance()
{
    qCDebug(logUtilClasses) << "Getting DTrashManager instance";
    return globalTrashManager;
}

bool DTrashManager::trashIsEmpty() const
{
    qCDebug(logUtilClasses) << "Checking if trash is empty (Windows implementation)";
    return false;
}

bool DTrashManager::cleanTrash()
{
    qCDebug(logUtilClasses) << "Cleaning trash (Windows implementation)";
    return false;
}

bool DTrashManager::moveToTrash(const QString &filePath, bool followSymlink)
{
    qCDebug(logUtilClasses) << "Moving file to trash (Windows implementation):" << filePath;
    return false;
}

DTrashManager::DTrashManager()
    : QObject()
    , DObject(*new DTrashManagerPrivate(this))
{
    qCDebug(logUtilClasses) << "DTrashManager created (Windows implementation)";
}

DWIDGET_END_NAMESPACE

#endif
