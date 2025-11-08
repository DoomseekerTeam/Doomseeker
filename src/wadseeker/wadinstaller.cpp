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
#include "protocols/http.h"
#include "zip/unarchive.h"

#include <QDir>
#include <QFile>
#include <QList>
#include <QNetworkReply>
#include <QTemporaryFile>

WadInstaller::WadInstaller(const QString &installPath, const QList<WadDownloadInfo> &wads)
{
	d.installPath = installPath;
	d.wads = wads;
}

WadInstallerResult WadInstaller::install(const WadDownloadInfo &wad, QNetworkReply &reply)
{
	// If the stream is sequential, save the contents
	// to a temporary file so it can be seeked freely.
	QTemporaryFile tempFile;
	QIODevice *stream = &reply;
	if (stream->isSequential())
	{
		tempFile.open();
		// We need to be able to seek the IO device because it needs
		// to be reusable. If the current IO device cannot be seeked,
		// we need to copy its contents into a temp file.
		IOUtils::copy(*stream, tempFile);
		tempFile.flush();
		tempFile.seek(0);
		stream = &tempFile;
	}

	// We need to determine the correct filename.
	QString filename = wad.name();
	Http http(&reply);
	const QString attachmentName = http.attachmentName().trimmed();
	if (!attachmentName.isEmpty())
	{
		filename = attachmentName;;
	}
	else
	{
		QFileInfo urlInfo(reply.url().path());
		const QString urlFilename = urlInfo.fileName().trimmed();
		if (!urlFilename.isEmpty())
		{
			filename = urlFilename;
		}
	}

	// Now try to install the file under the seeked filename.
	QList<WadInstallerResult> installResults;
	if (filename.compare(wad.name(), Qt::CaseInsensitive) == 0)
	{
		installResults << installFile(wad, stream);
	}

	// If file is an archive, inspect its contents for possible
	// files we might be seeking.
	QFileInfo fileInfo(filename);
	QScopedPointer<UnArchive> archive(UnArchive::openArchive(fileInfo, stream));
	if (archive != nullptr)
	{
		installResults << installArchive(*archive);
	}

	if (installResults.isEmpty())
	{
		return WadInstallerResult::makeError(
			tr("It's not the desired file nor an archive."));
	}
	return WadInstallerResult::aggregate(installResults);
}

WadInstallerResult WadInstaller::installArchive(UnArchive &archive)
{
	QDir installDir(d.installPath);
	WadInstallerResult dirResult = makeSureDirPathExists(installDir);
	if (dirResult.isError())
	{
		return dirResult;
	}

	WadInstallerResult result;
	FileFind fileFinder(archive.files());

	// We will try to find all requested WADs in the single archive.
	for (const WadDownloadInfo &wad : d.wads)
	{
		const QString &name = wad.name();

		int entryIndex = fileFinder.findFilename(name);
		if (entryIndex >= 0)
		{
			// File was found in the archive. Attempt extraction.
			// Extract first to a temporary location because we will
			// be confirming if the file is valid before we install it.
			QTemporaryFile tempDest;
			tempDest.setAutoRemove(false);
			tempDest.open();
			QString destPath = installDir.absoluteFilePath(name);
			if (archive.extract(entryIndex, tempDest.fileName()))
			{
				if (wad.validFile(tempDest.fileName()))
				{
					// A valid file can be moved to its final installation place.
					QFile destFile(destPath);
					if (destFile.exists())
						destFile.remove();
					if (tempDest.rename(destPath))
						result.installedWads << QFileInfo(destFile);
				}
				else
				{
					tempDest.setAutoRemove(true);
				}
			}
		}
	}

	return result;
}

WadInstallerResult WadInstaller::installFile(const WadDownloadInfo &wad, QIODevice *stream)
{
	// Check for problems.
	QDir installDir(d.installPath);
	WadInstallerResult dirResult = makeSureDirPathExists(installDir);
	if (dirResult.isError())
	{
		return dirResult;
	}

	// Save the file from input stream to the media storage device.
	QString filePath = installDir.absoluteFilePath(wad.name());
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
	if (!wad.validFile(filePath))
	{
		file.remove();
		return WadInstallerResult::makeError(
			tr("The checksum of the installed file \"%1\" didn't "
				"match the expected one.").arg(wad.name()));
	}

	return WadInstallerResult::makeSuccess(QFileInfo(filePath));
}

WadInstallerResult WadInstaller::makeSureDirPathExists(QDir &dir)
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

/*
  WadInstallerResult
*/
WadInstallerResult WadInstallerResult::aggregate(const QList<WadInstallerResult> &results)
{
	WadInstallerResult aggregate;
	auto it = results.begin();
	if (it != results.end())
	{
		aggregate = *it;
		++it;
	}
	for (; it != results.end(); ++it)
	{
		for (const QFileInfo &installedWad : it->installedWads)
		{
			if (!aggregate.installedWads.contains(installedWad))
				aggregate.installedWads << installedWad;
		}

		aggregate.errors.append(it->errors);
		aggregate.errors.removeDuplicates();

		aggregate.errorLevel = std::max(aggregate.errorLevel, it->errorLevel);
	}
	return aggregate;
}
