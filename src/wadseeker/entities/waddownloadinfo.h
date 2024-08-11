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

#include "../dptr.h"
#include "../wadseekerexportinfo.h"

#include <QByteArray>
#include <QList>
#include <QString>

class ModFile;
class Checksum;

/**
 * @brief Tracks WAD info and provides tools useful for WAD download.
 *
 * @deprecated This class is not useful outside the Wadseeker library,
 * and will be unexported in Wadseeker 3.0.
 */
class WADSEEKER_API WadDownloadInfo
{
public:
	WadDownloadInfo();
	WadDownloadInfo(const QString &name);

	WadDownloadInfo(const ModFile &modFile);
	operator ModFile() const;

	/**
	 * @brief Get name for the archive that may contain the mentioned file.
	 *
	 * **NOTE:** If isArchive() returns true, this will return the same value
	 * as name().
	 *
	 * @param suffix
	 *     Archive's extensions without the '.' character. For example
	 *     "zip" or "7z".
	 */
	QString archiveName(const QString &suffix) const;

	/**
	 * @brief Get the name() minus the extension.
	 */
	QString basename() const;

	/**
	 * @brief Recognize if download is an archive basing on the extension
	 * in the name().
	 */
	bool isArchive() const;

	bool isValid() const;

	// TODO doing the thing as described here is a bad idea, because
	// it will match `file.wad == file.pk3`, so let's remove this function
	// once this class is unexported.
	/**
	 * @brief Check if filename is the same WAD.
	 *
	 * The check is done by comparing the basenames parts of the filename
	 * with suffixes stripped.
	 */
	bool isFilenameIndicatingSameWad(const QString &filename) const;

	/**
	 * @brief The name of the WAD, as specified in the constructor.
	 */
	const QString &name() const;

	/**
	 * @brief Check if name() values match.
	 *
	 * The check is case-insensitive.
	 */
	bool operator==(const WadDownloadInfo &other) const;

	/**
	 * @brief Check if name() values mismatch.
	 *
	 * The check is case-insensitive.
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
	 * @brief Return all possible names under which this file could be found.
	 *
	 * This includes possibleArchiveNames() and the name(), without duplicates.
	 */
	QStringList possibleFilenames() const;

	/**
	 * @brief Set the size of the file.
	 *
	 * @param size
	 *     Size of the file in bytes if known. If unknown,
	 *     then a negative value should be set.
	 */
	void setSize(qint64 size);

	/**
	 * @brief Size of the file, if known.
	 *
	 * @return Size of the file in bytes if known. If unknown
	 *         then a negative value is returned.
	 */
	qint64 size() const;

	/**
	 * @brief List of checksums known.
	 *
	 * @return All known checksums for this WAD.
	 */
	const QList<Checksum> &checksums() const;

	/**
	 * @brief Verify if a file has the same checksums as the WAD.
	 *
	 * If the current WAD has no checksums, then this function will
	 * always return `true`.
	 *
	 * @param path - Path to the file.
	 *
	 * @return False if the file at path has a different checksum.
	 */
	bool validFile(const QString &path) const;
private:
	DPtr<WadDownloadInfo> d;
};

#endif
