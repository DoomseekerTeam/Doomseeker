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
#ifndef __WADINSTALLER_H__
#define __WADINSTALLER_H__

#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QStringList>

class UnArchive;
class WadDownloadInfo;

/**
 * @brief Handles installation of WADs in specified path on a permanent
 *        storage media.
 */
class WadInstaller : public QObject
{
public:
	/**
	 * @brief Result of a WAD install procedure.
	 */
	class WadInstallerResult
	{
	public:
		/**
		 * @brief Error reason if bError is true.
		 */
		QString error;

		/**
		 * @brief QFileInfo objects denoting installed WADs.
		 */
		QList<QFileInfo> installedWads;

		static WadInstallerResult makeCriticalError(const QString &error)
		{
			WadInstallerResult result;

			result.errorLevel = Critical;
			result.error = error;

			return result;
		}

		static WadInstallerResult makeError(const QString &error)
		{
			WadInstallerResult result;

			result.errorLevel = Normal;
			result.error = error;

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
		enum ErrorLevel
		{
			None,
			Normal,
			Critical
		};
		ErrorLevel errorLevel;
	};

	WadInstaller(const QString &installPath);

	/**
	 * @brief Extracts contents of an archive.
	 *
	 * @param pArchive
	 *      UnArchive object from which files will be extracted.
	 * @param requestedFilenames
	 *      WadDownloadInfo objects describing requested WADs..
	 *
	 * @return WadInstallerResult object.
	 */
	WadInstallerResult installArchive(UnArchive &archive, const QList<WadDownloadInfo *> &requestedWads);

	/**
	 * @brief Saves specified file.
	 *
	 * @param fileName
	 *      Name under which the file will be saved.
	 * @param stream
	 *      QIODevice ready for read operations.
	 *
	 * @return WadInstallerResult object.
	 */
	WadInstallerResult installFile(const QString &fileName, QIODevice *stream);

private:
	class PrivData
	{
	public:
		QString installPath;
	};

	PrivData d;

	/**
	 * @brief Checks if dir path exists. If not, creates it.
	 *
	 * @return WadInstallerResult object will carry an error if path
	 *         creation fails.
	 */
	WadInstallerResult makeSureDirPathExists(QDir &dir);
};

#endif
