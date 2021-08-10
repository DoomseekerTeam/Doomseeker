//------------------------------------------------------------------------------
// wadinstaller.cpp
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
// Copyright (C) 2011 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "wadinstaller.h"

#include "entities/waddownloadinfo.h"
#include "filefind.h"
#include "ioutils.h"
#include "zip/unarchive.h"

#include <QDebug>
#include <QDir>
#include <QFile>

WadInstaller::WadInstaller(const QString &installPath)
{
	d.installPath = installPath;
}

WadInstaller::WadInstallerResult WadInstaller::installArchive(UnArchive &archive, const QList<WadDownloadInfo *> &requestedWads)
{
	QDir installDir(d.installPath);
	WadInstallerResult dirResult = makeSureDirPathExists(installDir);
	if (dirResult.isError())
	{
		return dirResult;
	}

	WadInstallerResult result;
	#ifndef NDEBUG
	for (const QString &file : archive.files())
	{
		qDebug() << "archive file entry " << file;
	}
	#endif
	FileFind fileFinder(archive.files());

	// We will try to find all requested WADs in the single archive.
	for (const WadDownloadInfo *pWadInfo : requestedWads)
	{
		const QString &name = pWadInfo->name();
		#ifndef NDEBUG
		qDebug() << "requestedWad " << name;
		#endif

		int entryIndex = fileFinder.findFilename(name);
		#ifndef NDEBUG
		qDebug() << "requestedWad index in the archive " << entryIndex;
		#endif
		if (entryIndex >= 0)
		{
			// File was found in the archive. Attempt extraction.
			QString filePath = installDir.absoluteFilePath(name);
			#ifndef NDEBUG
			qDebug() << "extracting requestedWad to path " << filePath;
			#endif
			if (archive.extract(entryIndex, filePath))
			{
				result.installedWads << QFileInfo(filePath);
			}
		}
	}

	return result;
}

WadInstaller::WadInstallerResult WadInstaller::installFile(const QString &fileName, QIODevice *stream)
{
	// Check for problems.
	QDir installDir(d.installPath);
	WadInstallerResult dirResult = makeSureDirPathExists(installDir);
	if (dirResult.isError())
	{
		return dirResult;
	}

	// Save the file from input stream to the media storage device.
	QString filePath = installDir.absoluteFilePath(fileName);
	QFile file(filePath);

	if (!file.open(QFile::WriteOnly))
	{
		return WadInstallerResult::makeCriticalError(
			tr("Failed to open file \"%1\" for write.")
				.arg(filePath));
	}

	if (!stream->isOpen())
	{
		stream->open(QIODevice::ReadOnly);
	}
	else if (stream->isOpen() && !stream->isReadable())
	{
		return WadInstallerResult::makeCriticalError(
			tr("Developer failure in WadInstaller::installFile(): ")
			+ tr("input stream is open but is not readable."));
	}

	if (stream->size() <= 0)
	{
		return WadInstallerResult::makeError(tr("Attempt to save an empty file."));
	}

	if (!IOUtils::copy(*stream, file))
	{
		return WadInstallerResult::makeCriticalError(
			tr("Failed to save file \"%1\".")
				.arg(filePath));
	}

	file.close();

	return WadInstallerResult::makeSuccess(QFileInfo(filePath));
}

WadInstaller::WadInstallerResult WadInstaller::makeSureDirPathExists(QDir &dir)
{
	if (!dir.exists())
	{
		if (!dir.mkpath("."))
		{
			return WadInstallerResult::makeCriticalError(
				tr("Directory \"%1\", where files are supposed to be saved, doesn't exist and cannot be created.")
					.arg(dir.absolutePath()));
		}
	}

	return WadInstallerResult();
}
