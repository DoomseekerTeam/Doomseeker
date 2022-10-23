//------------------------------------------------------------------------------
// zipfile.h
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
#ifndef __ZIPFILE_H_
#define __ZIPFILE_H_

#include "dptr.h"
#include <QByteArray>
#include <QIODevice>
#include <QString>
#include <QVector>

#define ZIP_LOCAL_FILE_HEADER_SIZE 30

namespace ZipFile
{
enum HeaderError
{
	NoError,
	Corrupted,
	NotHeader,
	EndOfFileReached
};

struct CentralDirectoryFileHeader;
struct EndOfCentralDirectory;

class CentralDirectory
{
public:
	static CentralDirectory find(QIODevice *io);

	CentralDirectory();

	int fileCount() const;
	int fileIndex(const QString &file) const;
	bool isValid() const;
	CentralDirectoryFileHeader operator[](int index) const;

private:
	DPtr<CentralDirectory> d;

	static CentralDirectory invalid();
	CentralDirectory(const EndOfCentralDirectory &eocd);

	HeaderError readCentralDirectory(QIODevice *io);
};

struct EndOfCentralDirectory
{
	friend class CentralDirectory;

	quint32 signature;
	quint32 centralDirectorySize;
	quint32 centralDirectoryOffset;
	quint16 commentLength;

	static HeaderError read(QIODevice *io, EndOfCentralDirectory &out);

	EndOfCentralDirectory();

	bool isValid() const;

private:
	static const quint32 SIGNATURE = 0x06054b50;
	static const quint32 MINIMAL_SIZE = 22;
};

struct CentralDirectoryFileHeader
{
	quint32 signature;
	quint32 compressedSize;
	quint16 fileNameLength;
	quint16 extraFieldLength;
	quint16 fileCommentLength;
	quint32 localFileHeaderOffset;
	QString fileName;

	static HeaderError read(QIODevice *io, CentralDirectoryFileHeader &outHeader);

	CentralDirectoryFileHeader();
	bool isValid() const;

private:
	static const quint32 SIGNATURE = 0x02014b50;
	static const int MINIMAL_SIZE = 46;
};

struct LocalFileHeader
{
	LocalFileHeader();

	/**
	 * @brief Size of entire file entry. Header size + data size.
	 */
	unsigned long fileEntrySize() const;

	/**
	 * @param data - byte array
	 * @param dataSize - size of the byte array, function will return true if this is equal to 30.
	 * @return If incorrect data is passed (this could happen if the zip file is corrupted)
	 * it will set localFileHeaderSignature to correct value but still return false.
	 * If given data is not a local file header it will also return false but localFileHeaderSignature
	 * will be set to other value.
	 */
	HeaderError fromByteArray(const QByteArray &array);
	unsigned long howManyBytesTillData() const;

	/* FROM ZIP FILE FORMAT DESCRIPTION */
	unsigned long localFileHeaderSignature; // (0x04034b50), if anything fails this will be set to 0
	unsigned short versionNeededToExtract;
	unsigned short generalPurposeBitFlag;
	unsigned short compressionMethod;
	unsigned short lastModFileTime;
	unsigned short lastModFileDate;
	unsigned long crc32;
	unsigned long compressedSize;
	unsigned long uncompressedSize;
	unsigned short fileNameLength;
	unsigned short extraFieldLength;

	QString fileName;
	QByteArray extraField;
	/* END OF ZIP FILE FORMAT DESCRIPTION */

	/**
	 * Position in the ZIP file at which the header begins.
	 * Data position can be calculated from this using
	 * howManyBytesTillData() method.
	 */
	unsigned long headerPosition;
};
}

#endif
