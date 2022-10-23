//------------------------------------------------------------------------------
// unzip.h
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
// Copyright (C) 2009 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __UNZIP_H_
#define __UNZIP_H_

#include "../wadseeker.h"
#include "unarchive.h"
#include "zipfile.h"
#include <QFile>
#include <QFileInfo>
#include <QObject>

class UnZip : public UnArchive
{
	Q_OBJECT

public:
	UnZip(QIODevice *device);
	~UnZip() override;

	bool extract(int file, const QString &where) override;
	int findFileEntry(const QString &entryName) override;
	QString fileNameFromIndex(int file) override;
	QStringList files() override;

	/**
	 * @brief true if the QIODevice specified in the constructor
	 *        is a valid ZIP file.
	 */
	bool isValid() override
	{
		return isZip();
	}

	/**
	 * @brief true if the QIODevice specified in the constructor
	 *        is a valid ZIP file.
	 */
	bool isZip();

private:
	ZipFile::CentralDirectory centralDirectory;

	/**
	 * @brief This method expects the iodevice to be already open
	 *        and it won't close it.
	 *
	 * @return ZipLocalFileHeader::HeaderError value
	 */
	ZipFile::HeaderError readHeader(qint64 pos, ZipFile::LocalFileHeader &zip);

	/**
	 * @brief This method expects the iodevice to be already open
	 *        and it won't close it.
	 *
	 * @return ZipLocalFileHeader::HeaderError value
	 */
	ZipFile::HeaderError readHeaderFromFileIndex(int file, ZipFile::LocalFileHeader &zip);

	int uncompress(QIODevice &streamIn, QIODevice &streamOut, unsigned long compressedSize);
};

#endif
