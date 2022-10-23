//------------------------------------------------------------------------------
// unzip.cpp
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
#include "unzip.h"
#include <zlib.h>

#include "ioutils.h"
#include <QBuffer>
#include <QDebug>
#include <QFileInfo>

/**
 * @brief A try-finally block for QIODevice.
 */
class IODeviceCloser
{
public:
	IODeviceCloser(QIODevice *d)
	{
		this->d = d;
	}

	~IODeviceCloser()
	{
		this->d->close();
	}

private:
	QIODevice *d;
};

UnZip::UnZip(QIODevice *device)
	: UnArchive(device)
{
	if (device->open(QIODevice::ReadOnly))
	{
		centralDirectory = ZipFile::CentralDirectory::find(device);
		device->close();
	}
}

UnZip::~UnZip()
{
}

bool UnZip::extract(int file, const QString &where)
{
	if (!isValid())
	{
		qDebug() << "UnZip::extract(): Archive is invalid";
		return false;
	}

	ZipFile::LocalFileHeader header;
	if (!stream->open(QFile::ReadOnly))
	{
		qDebug() << "UnZip::extract(): Failed to open archive for reading.";
		return false;
	}
	IODeviceCloser ioDevCloser(stream);

	ZipFile::HeaderError headerResult = readHeaderFromFileIndex(file, header);
	if (headerResult != ZipFile::NoError)
	{
		qDebug() << "UnZip::extract(): Failed to extract file" << file
			<< ", result:" << headerResult;
		return false;
	}
	qint64 pos = header.headerPosition + header.howManyBytesTillData();

	if (!stream->seek(pos))
	{
		qDebug() << "UnZip::extract(): Failed to seek archive to pos" << pos
			<< "for file:" << file << ", result:" << headerResult;
		return false;
	}

	QFile outputFile(where);
	outputFile.open(QFile::WriteOnly);
	bool bOk = true;
	if (header.compressionMethod != 0)
	{
		int err = uncompress(*stream, outputFile, header.compressedSize);
		bOk = (err == Z_OK);
	}
	else
	{
		// No compression, simply copy.
		bOk = IOUtils::copy(*stream, outputFile, header.uncompressedSize);
	}
	outputFile.close();
	return bOk;
}

int UnZip::findFileEntry(const QString &entryName)
{
	return centralDirectory.fileIndex(entryName);
}

QString UnZip::fileNameFromIndex(int file)
{
	return centralDirectory[file].fileName;
}

QStringList UnZip::files()
{
	QStringList files;
	for (int i = 0; i < centralDirectory.fileCount(); ++i)
		files << centralDirectory[i].fileName;
	return files;
}

bool UnZip::isZip()
{
	return centralDirectory.isValid();
}

ZipFile::HeaderError UnZip::readHeader(qint64 pos, ZipFile::LocalFileHeader &zip)
{
	// This expects the file to be already open and doesn't close it.
	if (pos >= stream->size())
		return ZipFile::EndOfFileReached;

	if (!stream->seek(pos))
		return ZipFile::Corrupted;

	QByteArray array = stream->read(ZIP_LOCAL_FILE_HEADER_SIZE);

	ZipFile::HeaderError readErr = zip.fromByteArray(array);
	if (readErr != ZipFile::NoError)
		return readErr;

	array = stream->read(zip.fileNameLength);
	if (array.size() < zip.fileNameLength)
		return ZipFile::Corrupted;

	zip.fileName = array.constData();

	array = stream->read(zip.extraFieldLength);
	if (array.size() < zip.extraFieldLength)
		return ZipFile::Corrupted;

	zip.extraField = array;

	zip.headerPosition = pos;

	return ZipFile::NoError;
}

ZipFile::HeaderError UnZip::readHeaderFromFileIndex(int file, ZipFile::LocalFileHeader &zip)
{
	zip = ZipFile::LocalFileHeader();
	ZipFile::LocalFileHeader tempHeader;
	ZipFile::CentralDirectoryFileHeader centralDirectoryHeader = centralDirectory[file];
	if (!centralDirectoryHeader.isValid())
		return ZipFile::Corrupted;

	qint64 pos = centralDirectoryHeader.localFileHeaderOffset;
	ZipFile::HeaderError errorCode = readHeader(pos, tempHeader);
	if (errorCode != ZipFile::NoError)
		return errorCode;
	tempHeader.compressedSize = centralDirectoryHeader.compressedSize;

	zip = tempHeader;
	return ZipFile::NoError;
}

int UnZip::uncompress(QIODevice &streamIn, QIODevice &streamOut, unsigned long compressedSize)
{
	Q_UNUSED(compressedSize);
	const unsigned long BUFFER_SIZE = 2 * 1024 * 1024;
	char *out = new char[BUFFER_SIZE];

	z_stream zstream;
	zstream.next_out = (unsigned char *)out;
	zstream.avail_out = BUFFER_SIZE;
	zstream.zalloc = Z_NULL;
	zstream.zfree = Z_NULL;
	unsigned int err = inflateInit2(&zstream, -15);

	int ret = Z_OK;
	bool bOk = true;
	do
	{
		QByteArray inData = streamIn.read(BUFFER_SIZE);
		if (inData.isEmpty())
		{
			err = Z_STREAM_END;
			break;
		}
		zstream.avail_in = inData.size();
		zstream.next_in = (Bytef *)inData.data();

		// run inflate() on input until output buffer not full
		do
		{
			zstream.avail_out = BUFFER_SIZE;
			zstream.next_out = (Bytef *)out;
			ret = inflate(&zstream, Z_NO_FLUSH);

			switch (ret)
			{
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;
				[[gnu::fallthrough]];
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				bOk = false;
				break;
			}

			if (!bOk)
				break;

			int have = BUFFER_SIZE - zstream.avail_out;
			if (streamOut.write(out, have) != have)
			{
				bOk = false;
				ret = Z_ERRNO;
				break;
			}
		} while (zstream.avail_out == 0 && bOk);

		// done when inflate() says it's done
	} while (ret != Z_STREAM_END && bOk);

	int inflateEndErr = inflateEnd(&zstream);
	if (err != Z_STREAM_END)
		return err;

	return inflateEndErr;
}
