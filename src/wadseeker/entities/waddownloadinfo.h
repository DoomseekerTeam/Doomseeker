//------------------------------------------------------------------------------
// waddownloadinfo.h
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
#ifndef __WADDOWNLOADINFO_H__
#define __WADDOWNLOADINFO_H__

#include "../wadseekerexportinfo.h"
#include "../dptr.h"

#include <QByteArray>
#include <QList>
#include <QString>

class ModFile;
class Checksum;

/**
 * @brief Holds information about unique WAD download.
 */
class WADSEEKER_API WadDownloadInfo
{
public:
	WadDownloadInfo();
	WadDownloadInfo(const QString &name);

	WadDownloadInfo(const ModFile &modFile);
	operator ModFile() const;

	/**
	 * @brief Gets name for the archive that may contain the mentioned file.
	 *
	 * @b NOTE: If isArchive() returns true this will return the same value
	 * as name().
	 *
	 * @param suffix
	 *      Archive's extensions without the '.' character. For example
	 *      "zip" or "7z".
	 */
	QString archiveName(const QString &suffix) const;

	/**
	 * @brief Gets the name() minus the extension.
	 */
	QString basename() const;

	/**
	 * @brief Recognizes if download is an archive basing on the extension
	 * in the name specified in the constructor.
	 */
	bool isArchive() const;

	bool isValid() const;

	/**
	 * @brief Checks if filename is the same WAD as this object.
	 *
	 * The check is done by comparing the basenames parts of the filename
	 * with suffixes stripped.
	 */
	bool isFilenameIndicatingSameWad(const QString &filename) const;

	const QString &name() const;

	/**
	 * @brief Check if name() values match.
	 *
	 * Check is case-insensitive.
	 */
	bool operator==(const WadDownloadInfo &other) const;

	/**
	 * @brief Check if name() values mismatch.
	 *
	 * Check is case-insensitive.
	 */
	bool operator!=(const WadDownloadInfo &other) const;

	/**
	 * @brief Return possible archive names.
	 *
	 * @return If isArchive() returns true, list will contain only the
	 *         name() value. Otherwise, the returned list will contain
	 *         filenames for all supported archives, where the archive
	 *         extension will replace the extension from name().
	 */
	QStringList possibleArchiveNames() const;

	/**
	 * @brief Size of the file.
	 *
	 * @param size
	 *      Size of the file in bytes if known. If unknown
	 *      then a negative value should be set.
	 */
	void setSize(qint64 size);

	/**
	 * @brief Size of the file if known.
	 *
	 * @return Size of the file in bytes if known. If unknown
	 *         then a negative value is returned.
	 */
	qint64 size() const;

	/**
	 * @brief List of checksums known.
	 *
	 * @return List of all known checksums for the desired wad.
	 */
	const QList<Checksum> &checksums() const;

	/**
	 * @brief Verifies if a file has the same checksums as the wad.
	 *
	 * @param path - Path to the file.
	 */
	const bool validFile(const QString &path) const;
private:
	DPtr<WadDownloadInfo> d;
};

#endif
