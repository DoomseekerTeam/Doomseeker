//------------------------------------------------------------------------------
// wadinstaller.h
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
#ifndef WADSEEKER_WADINSTALLER_H
#define WADSEEKER_WADINSTALLER_H

#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QStringList>

class QNetworkReply;
class UnArchive;
class WadDownloadInfo;

// TODO WadInstallerResult should be able to convey errors for more than one
// file at once to indicate partial installs.
/**
 * @brief Result of a WAD install procedure.
 */
class WadInstallerResult
{
public:
	/**
	 * @brief Combine multiple results into one.
	 *
	 * - installedWads are joined, with duplicates removed.
	 * - errors are joined, with duplicates removed.
	 * - The highest errorLevel is set.
	 */
	static WadInstallerResult aggregate(const QList<WadInstallerResult> &results);

	/**
	 * @brief Error reason if bError is true.
	 */
	QStringList errors;

	/**
	 * @brief QFileInfo objects denoting installed WADs.
	 */
	QList<QFileInfo> installedWads;

	static WadInstallerResult makeCriticalError(const QString &error)
	{
		WadInstallerResult result;

		result.errorLevel = Critical;
		result.errors << error;

		return result;
	}

	static WadInstallerResult makeError(const QString &error)
	{
		WadInstallerResult result;

		result.errorLevel = Normal;
		result.errors << error;

		return result;
	}

	static WadInstallerResult makeSuccess(const QFileInfo &file)
	{
		WadInstallerResult result;

		result.installedWads << file;

		return result;
	}

	WadInstallerResult()
	{
		errorLevel = None;
	}

	bool isCriticalError() const
	{
		return errorLevel == Critical;
	}

	bool isError() const
	{
		return errorLevel != None;
	}

private:
	// TODO reuse WadseekerLib::MessageType here?
	enum ErrorLevel
	{
		None = 0,
		Normal = 1,
		Critical = 2,
	};
	ErrorLevel errorLevel;
};


/**
 * @brief Installs files in the specified path in the filesystem.
 */
class WadInstaller : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(WadInstaller);

public:
	WadInstaller(const QString &installPath, const QList<WadDownloadInfo> &wads);

	WadInstallerResult install(const WadDownloadInfo &wad, QNetworkReply &reply);

	/**
	 * @brief Extract contents of an archive.
	 *
	 * It extracts everything that was passed into
	 * the constructor as `wads`.
	 *
	 * @param archive
	 *      UnArchive from which files will be extracted.
	 *
	 * @return WadInstallerResult.
	 */
	WadInstallerResult installArchive(UnArchive &archive);

	/**
	 * @brief Install the specified file directly.
	 *
	 * This may perform a checksum check if checksums are available.
	 * The install will fail if checksums mismatch, because that's
	 * not the file we want.
	 *
	 * @param wad
	 *      WAD (or any other file, really) to install.
	 * @param stream
	 *      QIODevice ready for reading, from which the file's
	 *      bytes will be read.
	 *
	 * @return WadInstallerResult.
	 */
	WadInstallerResult installFile(const WadDownloadInfo &wad, QIODevice *stream);

private:
	class PrivData
	{
	public:
		QString installPath;
		QList<WadDownloadInfo> wads;
	};

	PrivData d;

	/**
	 * @brief Check if dir path exists. If not, create it.
	 *
	 * @return WadInstallerResult object will carry an error if path
	 *         creation fails.
	 */
	WadInstallerResult makeSureDirPathExists(QDir &dir);
};

#endif
