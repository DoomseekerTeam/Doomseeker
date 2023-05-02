//------------------------------------------------------------------------------
// zipfile.cpp
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
// Copyright (C) 2016 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "zipfile.h"

#include <cassert>

#define READINT32(pointer) \
	((quint32((quint8)(*(pointer)))) | \
	(quint32(quint8(*((pointer) + 1))) << 8) | \
	(quint32(quint8(*((pointer) + 2))) << 16) | \
	(quint32(quint8(*((pointer) + 3))) << 24))

#define READINT16(pointer) ((quint16((quint8)(*(pointer)))) | (quint16(quint8(*((pointer) + 1))) << 8))

#define LOCAL_FILE_HEADER_SIGNATURE 0x04034b50
#define DESCRIPTOR_EXISTS_FLAG 0x0004 // (3rd bit)


DClass<ZipFile::CentralDirectory>
{
public:
	ZipFile::EndOfCentralDirectory eocd;
	QList<ZipFile::CentralDirectoryFileHeader> fileHeaders;
};
DPointered(ZipFile::CentralDirectory)

namespace ZipFile
{
static const quint32 INVALID_SIGNATURE = 0xcccccccc;

///////////////////////////////////////////////////////////////////////////

CentralDirectory::CentralDirectory()
{
}

CentralDirectory::CentralDirectory(const EndOfCentralDirectory &eocd)
{
	d->eocd = eocd;
}

int CentralDirectory::fileCount() const
{
	return d->fileHeaders.size();
}

int CentralDirectory::fileIndex(const QString &file) const
{
	for (int index = 0; index < d->fileHeaders.size(); ++index)
	{
		if (d->fileHeaders[index].fileName.compare(file, Qt::CaseInsensitive) == 0)
			return index;
	}
	return -1;
}

CentralDirectoryFileHeader CentralDirectory::operator[](int index) const
{
	if (index < 0 || index >= d->fileHeaders.size())
		return CentralDirectoryFileHeader();
	return d->fileHeaders[index];
}

CentralDirectory CentralDirectory::find(QIODevice *io)
{
	if (io->size() < EndOfCentralDirectory::MINIMAL_SIZE)
		return invalid();
	qint64 maxSeekBack = qMin<qint64>(1024 * 1024, io->size() - EndOfCentralDirectory::MINIMAL_SIZE);
	for (qint64 seekBack = 0; seekBack < maxSeekBack; ++seekBack)
	{
		io->seek(io->size() - (EndOfCentralDirectory::MINIMAL_SIZE + seekBack));
		EndOfCentralDirectory eocd;
		if (EndOfCentralDirectory::read(io, eocd) == NoError)
		{
			CentralDirectory directory = CentralDirectory(eocd);
			if (directory.readCentralDirectory(io) == NoError)
				return directory;
			else
				return invalid();
		}
	}
	return invalid();
}

bool CentralDirectory::isValid() const
{
	return d->eocd.isValid();
}

CentralDirectory CentralDirectory::invalid()
{
	return CentralDirectory();
}

HeaderError CentralDirectory::readCentralDirectory(QIODevice *io)
{
	assert(isValid());
	if (d->eocd.centralDirectoryOffset >= io->size())
		return Corrupted;
	io->seek(d->eocd.centralDirectoryOffset);
	HeaderError headerError = NoError;
	while (headerError == NoError)
	{
		CentralDirectoryFileHeader fileHeader;
		headerError = CentralDirectoryFileHeader::read(io, fileHeader);
		if (headerError == NoError)
			d->fileHeaders << fileHeader;
	}
	return NoError;
}

///////////////////////////////////////////////////////////////////////////

EndOfCentralDirectory::EndOfCentralDirectory()
{
	signature = INVALID_SIGNATURE;
}

HeaderError EndOfCentralDirectory::read(QIODevice *io, EndOfCentralDirectory &out)
{
	out.signature = INVALID_SIGNATURE;
	QByteArray rawSignature = io->read(4);
	if (rawSignature.size() < 4)
		return EndOfFileReached;

	quint32 signature = READINT32(rawSignature.constData());
	if (signature != SIGNATURE)
		return NotHeader;

	QByteArray header = QByteArray(4, static_cast<char>(0xccU)) + io->read(MINIMAL_SIZE - 4);
	if (static_cast<unsigned>(header.size()) < MINIMAL_SIZE)
		return EndOfFileReached;

	const char *data = header.constData();
	out.centralDirectorySize = READINT32(data + 12);
	out.centralDirectoryOffset = READINT32(data + 16);
	out.commentLength = READINT16(data + 20);

	qint32 extraSize = out.commentLength;
	QByteArray extra = io->read(extraSize);
	if (extra.size() < extraSize)
		return EndOfFileReached;

	out.signature = signature;
	return NoError;
}

bool EndOfCentralDirectory::isValid() const
{
	return signature == SIGNATURE;
}

///////////////////////////////////////////////////////////////////////////

CentralDirectoryFileHeader::CentralDirectoryFileHeader()
{
	signature = INVALID_SIGNATURE;
}

HeaderError CentralDirectoryFileHeader::read(QIODevice *io, CentralDirectoryFileHeader &out)
{
	out.signature = INVALID_SIGNATURE;
	QByteArray rawSignature = io->read(4);
	if (rawSignature.size() < 4)
		return EndOfFileReached;

	quint32 signature = READINT32(rawSignature.constData());
	if (signature != SIGNATURE)
		return NotHeader;

	QByteArray header = QByteArray(4, static_cast<char>(0xccU)) + io->read(MINIMAL_SIZE - 4);
	if (header.size() < MINIMAL_SIZE)
		return EndOfFileReached;

	const char *data = header.constData();

	out.compressedSize = READINT32(data + 20);
	out.fileNameLength = READINT16(data + 28);
	out.fileCommentLength = READINT16(data + 30);
	out.extraFieldLength = READINT16(data + 32);
	out.localFileHeaderOffset = READINT32(data + 42);
	QByteArray fileName = io->read(out.fileNameLength);
	if (fileName.size() < out.fileNameLength)
		return EndOfFileReached;

	out.fileName = QString::fromUtf8(fileName);

	// Skip past the extra data in the header.
	qint32 extraSize = out.fileCommentLength + out.extraFieldLength;
	QByteArray extra = io->read(extraSize);
	if (extra.size() < extraSize)
		return EndOfFileReached;

	out.signature = signature;
	return NoError;
}

bool CentralDirectoryFileHeader::isValid() const
{
	return signature == SIGNATURE;
}

///////////////////////////////////////////////////////////////////////////

LocalFileHeader::LocalFileHeader()
{
	localFileHeaderSignature = 0;
}

unsigned long LocalFileHeader::fileEntrySize() const
{
	// Calculated depending on variables below.
	long dataDescriptorSize = 0;
	if ((generalPurposeBitFlag & DESCRIPTOR_EXISTS_FLAG) == DESCRIPTOR_EXISTS_FLAG)
		dataDescriptorSize = 12;

	return howManyBytesTillData() + dataDescriptorSize + compressedSize;
}

HeaderError LocalFileHeader::fromByteArray(const QByteArray &array)
{
	const char *data = array.constData();

	localFileHeaderSignature = 0;

	if (array.size() < 4)
		return NotHeader;

	localFileHeaderSignature = READINT32(data);

	if (localFileHeaderSignature != LOCAL_FILE_HEADER_SIGNATURE)
		return NotHeader;

	// The header is correct but the data size is not enough.
	if (array.size() != 30)
		return Corrupted;

	versionNeededToExtract = READINT16(&data[4]);
	generalPurposeBitFlag = READINT16(&data[6]);
	compressionMethod = READINT16(&data[8]);
	lastModFileTime = READINT16(&data[10]);
	lastModFileDate = READINT16(&data[12]);
	crc32 = READINT32(&data[14]);
	compressedSize = READINT32(&data[18]);
	uncompressedSize = READINT32(&data[22]);
	fileNameLength = READINT16(&data[26]);
	extraFieldLength = READINT16(&data[28]);

	return NoError;
}

unsigned long LocalFileHeader::howManyBytesTillData() const
{
	return ZIP_LOCAL_FILE_HEADER_SIZE + fileNameLength + extraFieldLength;
}
} // end of ZipFile namespace
