//------------------------------------------------------------------------------
// unarchive.h
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
// Copyright (C) 2010 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------

#ifndef __UNARCHIVE_H__
#define __UNARCHIVE_H__

#include "../wadseekerexportinfo.h"
#include <QObject>

class QByteArray;
class QIODevice;
class QFileInfo;

class WADSEEKER_API UnArchive : public QObject
{
	Q_OBJECT

public:
	UnArchive(QIODevice *dataStream);
	virtual ~UnArchive() override;

	/**
	 * @brief Extracts file from specified entry to a specified path.
	 *
	 * @param file
	 *      File index retrieved by findFileEntry() method.
	 * @param where
	 *      Path to the file name under which the extracted file will be
	 *      saved.
	 *
	 * @return true if extract was successful.
	 */
	virtual bool extract(int file, const QString &where) = 0;

	/**
	 * @brief File name from given index.
	 *
	 * @see extract()
	 * @see files()
	 */
	virtual QString fileNameFromIndex(int file) = 0;

	/**
	 * @brief Finds index of file entry basing on specified entry name.
	 *
	 * The extracted index can be used in extract() method to retrieve the
	 * file's data.
	 *
	 * @return The index is negative if entry was not found.
	 */
	virtual int findFileEntry(const QString &entryName) = 0;

	/**
	 * @brief Return list of all files in the archive.
	 *
	 * The order of the files on the returned list must be consistent with
	 * file indices used in other methods. extract() and fileNameFromIndex()
	 * must work with indices of the returned list.
	 */
	virtual QStringList files() = 0;

	virtual bool isValid() = 0;

	/**
	 * @brief Opens an archive stored in undefined QIODevice.
	 *
	 * @b WARNING:
	 * UnArchive will not take ownership of the dataStream QIODevice.
	 * The device needs to be deleted manually.
	 */
	static UnArchive *openArchive(const QFileInfo &fi, QIODevice *dataStream);

	/**
	 * @brief Opens an archive stored in the known path in the
	 *        file system.
	 */
	static UnArchive *openArchive(const QString &filename);

signals:
	void message(const QString &, int type);

protected:
	QIODevice *stream;

private:
	static UnArchive *detectArchive(const QFileInfo &fi, QIODevice * &device);

	/**
	 * @brief If true will cause delete of stream QIODevice.
	 */
	bool bHasIODeviceOwnership;
};

#endif /* __UNARCHIVE_H__ */
