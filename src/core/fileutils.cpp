//------------------------------------------------------------------------------
// fileutils.cpp
//------------------------------------------------------------------------------
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA
//
//------------------------------------------------------------------------------
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "fileutils.h"

#include "ntfsperm.h"
#include "log.h"
#include <QCryptographicHash>
#include <QDirIterator>
#include <QFileInfo>

class FileUtilsTr : public QObject
{
	Q_OBJECT

public:
	static QString cannotCreateDirectory()
	{
		return tr("cannot create directory");
	}

	static QString parentPathIsNotADirectory(const QString &path)
	{
		return tr("the parent path is not a directory: %1").arg(path);
	}

	static QString lackOfParentDirectoryPermissions(const QString &to)
	{
		return tr("lack of necessary permissions to the parent directory: %1")
			.arg(to);
	}

private:
	FileUtilsTr() = delete;
};

QByteArray FileUtils::md5(const QString &path)
{
	QFile f(path);
	if (f.open(QIODevice::ReadOnly))
	{
		QCryptographicHash hash(QCryptographicHash::Md5);
		QByteArray chunk = f.read(1024 * 1024);
		for (; !chunk.isEmpty(); chunk = f.read(1024 * 1024))
			hash.addData(chunk);
		f.close();
		return hash.result();
	}
	return QByteArray();
}

QString FileUtils::cdUpUntilExists(QString path)
{
	static const int SANITY_LOOP_LIMIT = 1000;
	QFileInfo fileInfo(QDir::cleanPath(path));
	for (int i = 0; i < SANITY_LOOP_LIMIT; ++i)
	{
		if (fileInfo.filePath().endsWith("/.."))
			return QString();
		if (fileInfo.exists() || fileInfo.isRoot())
			return fileInfo.filePath();
		fileInfo = QFileInfo(QDir::cleanPath(fileInfo.filePath() + "/.."));
	}
	return QString();
}

Qt::CaseSensitivity FileUtils::comparisonSensitivity()
{
	#if defined(Q_OS_WIN32)
	return Qt::CaseInsensitive;
	#else
	return Qt::CaseSensitive;
	#endif
}

bool FileUtils::containsPath(const QStringList &candidates, const QString &path)
{
	for (const QString &candidate : candidates)
	{
		if (QFileInfo(candidate) == QFileInfo(path))
			return true;
	}
	return false;
}

QDir FileUtils::dirOrDir(const QString &preferred, const QString &fallback)
{
	if (!preferred.isEmpty())
	{
		QDir dir(preferred);
		if (dir.exists())
			return dir;
	}
	return QDir(fallback);
}

DirErrno FileUtils::mkpath(const QDir &dir)
{
	// We need to reset errno to prevent false positives
	errno = 0;
	if (!dir.mkpath("."))
	{
		int errnoval = errno;
		if (errnoval != 0)
		{
			return DirErrno(dir.path(), errnoval, strerror(errnoval));
		}
		else
		{
			// Try to decipher if we have permissions.
			// First we must find the bottom-most directory that does actually exist.
			QString pathToBottomMostExisting = FileUtils::cdUpUntilExists(dir.path());
			// If we've found a bottom-most directory that exists, we can check its permissions.
			if (!pathToBottomMostExisting.isEmpty())
			{
				QFileInfo parentDir(pathToBottomMostExisting);
				if (parentDir.exists() && !parentDir.isDir())
				{
					return DirErrno(dir.path(), DirErrno::CUSTOM_ERROR,
						FileUtilsTr::parentPathIsNotADirectory(parentDir.filePath()));
				}

				// BLOCK - keep this to absolute minimum
				++qt_ntfs_permission_lookup;
				bool permissions = parentDir.isReadable()
					&& parentDir.isWritable()
					&& parentDir.isExecutable();
				--qt_ntfs_permission_lookup;
				// END OF BLOCK
				if (!permissions)
				{
					return DirErrno(dir.path(), DirErrno::CUSTOM_ERROR,
						FileUtilsTr::lackOfParentDirectoryPermissions(parentDir.filePath()));
				}
			}
			// Just give up trying to deduce a correct error.
			return DirErrno(dir.path(), DirErrno::CUSTOM_ERROR,
				FileUtilsTr::cannotCreateDirectory());
		}
	}
	return DirErrno();
}

bool FileUtils::rmAllFiles(const QString &dirPath,
	const QStringList &nameFilters)
{
	QDirIterator it(dirPath, nameFilters, QDir::Files);
	bool bAllSuccess = true;
	while (it.hasNext())
	{
		QString path = it.next();
		QFile f(path);
		if (!f.remove())
		{
			bAllSuccess = false;
			gLog << Log::tr("Failed to remove: %1").arg(path);
		}
	}
	return bAllSuccess;
}

#include "fileutils.moc"
