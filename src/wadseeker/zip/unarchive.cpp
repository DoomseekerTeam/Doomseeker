//------------------------------------------------------------------------------
// unarchive.cpp
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
// Copyright (C) 2011 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------

#include "un7zip.h"
#include "unarchive.h"
#include "untar.h"
#include "unzip.h"

#include <bzlib.h>
#include <QBuffer>
#include <QIODevice>
#include <zlib.h>

#define BUFFER_SIZE 8192

/**
 * Quick wrapper which allows a compressed stream to be seeked to a point.
 * This is done through storing the uncompressed data into an array.
 * WARNING: For now at least, we only care about reading.
 */
class CompressedIODevice : public QIODevice
{
public:
	CompressedIODevice() : QIODevice(), position(0)
	{
		stream = new QBuffer(&bufferedData);
		stream->open(QIODevice::ReadWrite);
	}
	~CompressedIODevice() override
	{
	}

	bool isSequential() const override { return false; }

	bool seek(qint64 pos) override
	{
		// Beware of the QIODevice buffer!
		// We must seek both our QBuffer stream and the QIODevice.
		if (pos > bufferedData.size())
		{
			int neededDataSize = pos - bufferedData.size();
			position = bufferedData.size();
			stream->seek(bufferedData.size());
			QIODevice::seek(bufferedData.size());
			char *tmp = new char[neededDataSize];
			read(tmp, neededDataSize);
			delete[] tmp;
		}
		else
		{
			position = pos;
			stream->seek(pos);
			QIODevice::seek(pos);
		}
		return true;
	}

	qint64 size() const override
	{
		return bufferedData.size();
	}

protected:
	virtual qint64 readCompressedData(char *data, qint64 maxSize) = 0;

	qint64 readData(char *data, qint64 maxSize) override
	{
		qint64 bytesRead = 0;
		if (position < bufferedData.size())
			bytesRead = stream->read(data, maxSize);
		position += bytesRead;

		if (bytesRead != maxSize)
		{
			qint64 compressedBytesRead = readCompressedData(data + bytesRead, maxSize - bytesRead);
			if (compressedBytesRead == -1)
				return -1;
			stream->seek(bufferedData.size());
			stream->write(data + bytesRead, compressedBytesRead);
			position += compressedBytesRead;
			return compressedBytesRead + bytesRead;
		}
		return bytesRead;
	}

	qint64 writeData(const char *data, qint64 maxSize) override
	{
		Q_UNUSED(data);
		Q_UNUSED(maxSize);
		return -1;
	}
private:
	qint64 position;
	QByteArray bufferedData;
	QBuffer *stream;
};

/**
 * GZip Wrapper
 */
class GZDevice : public CompressedIODevice
{
public:
	GZDevice(QIODevice *wrap) : CompressedIODevice(), wrap(wrap)
	{
		gzCompress.zalloc = gzDecompress.zalloc = Z_NULL;
		gzCompress.zfree = gzDecompress.zfree = Z_NULL;
		gzCompress.opaque = gzDecompress.opaque = Z_NULL;
	}
	~GZDevice() override
	{
	}

	void close() override
	{
		if (openMode() | QIODevice::ReadOnly)
			inflateEnd(&gzDecompress);
		if (openMode() | QIODevice::WriteOnly)
			deflateEnd(&gzCompress);

		wrap->close();
		CompressedIODevice::close();
	}

	bool open(OpenMode mode) override
	{
		if (CompressedIODevice::open(mode) && wrap->open(mode))
		{
			if (mode | QIODevice::ReadOnly)
			{
				init(gzDecompress);
				if (inflateInit2(&gzDecompress, 31) != Z_OK)
					return false;
			}
			if (mode | QIODevice::WriteOnly)
			{
				init(gzCompress);
				if (deflateInit(&gzCompress, 9) != Z_OK)
					return false;
			}
			return true;
		}
		return false;
	}

protected:
	static void init(z_stream &stream)
	{
		stream.next_out = stream.next_in = Z_NULL;
		stream.avail_out = stream.avail_in = 0;
	}

	qint64 readCompressedData(char *data, qint64 len) override
	{
		gzDecompress.next_out = (Bytef *)data;
		gzDecompress.avail_out = len;

		while (gzDecompress.avail_out > 0)
		{
			gzDecompress.next_in = (Bytef *)buffer;
			gzDecompress.avail_in = wrap->read(buffer, BUFFER_SIZE);

			int ret = inflate(&gzDecompress, Z_NO_FLUSH);
			if (ret == Z_STREAM_END)
				break;
			if (ret != Z_OK)
				return -1;

			if (gzDecompress.avail_in > 0)
			{
				wrap->seek(wrap->pos() - gzDecompress.avail_in);
				break;
			}
		}

		return len - gzDecompress.avail_out;
	}

private:
	char buffer[BUFFER_SIZE];
	z_stream gzDecompress;
	z_stream gzCompress;

	QIODevice *wrap;
};

/**
 * BZip2 Wrapper
 */
class BZ2Device : public CompressedIODevice
{
public:
	BZ2Device(QIODevice *wrap) : CompressedIODevice(), wrap(wrap)
	{
		bzCompress.bzalloc = bzDecompress.bzalloc = nullptr;
		bzCompress.bzfree = bzDecompress.bzfree = nullptr;
		bzCompress.opaque = bzDecompress.opaque = nullptr;
	}
	~BZ2Device() override
	{
	}

	void close() override
	{
		if (openMode() | QIODevice::ReadOnly)
			BZ2_bzDecompressEnd(&bzDecompress);
		if (openMode() | QIODevice::WriteOnly)
			BZ2_bzCompressEnd(&bzCompress);

		wrap->close();
		CompressedIODevice::close();
	}

	bool open(OpenMode mode) override
	{
		if (CompressedIODevice::open(mode) && wrap->open(mode))
		{
			if (mode | QIODevice::ReadOnly)
			{
				if (BZ2_bzDecompressInit(&bzDecompress, 0, 0) != BZ_OK)
					return false;
			}
			if (mode | QIODevice::WriteOnly)
			{
				if (BZ2_bzCompressInit(&bzCompress, 9, 0, 30) != BZ_OK)
					return false;
			}
			return true;
		}
		return false;
	}

protected:
	qint64 readCompressedData(char *data, qint64 len) override
	{
		bzDecompress.next_out = data;
		bzDecompress.avail_out = len;

		while (bzDecompress.avail_out > 0)
		{
			bzDecompress.next_in = buffer;
			bzDecompress.avail_in = wrap->read(buffer, BUFFER_SIZE);

			int err = BZ2_bzDecompress(&bzDecompress);
			if (err != BZ_OK && err != BZ_STREAM_END)
				return -1;

			if (err == BZ_STREAM_END)
			{
				// If we didn't hit the end of the file then we need to reinit and keep going
				if (!wrap->atEnd())
				{
					BZ2_bzDecompressEnd(&bzDecompress);
					if (BZ2_bzDecompressInit(&bzDecompress, 0, 0) != BZ_OK)
						return -1;
				}
			}

			if (bzDecompress.avail_in > 0)
				wrap->seek(wrap->pos() - bzDecompress.avail_in);
		}

		return len - bzDecompress.avail_out;
	}

private:
	char buffer[BUFFER_SIZE];
	bz_stream bzDecompress;
	bz_stream bzCompress;

	QIODevice *wrap;
};

////////////////////////////////////////////////////////////////////////////////

UnArchive::UnArchive(QIODevice *dataStream)
{
	this->stream = dataStream;
	bHasIODeviceOwnership = false;
}

UnArchive::~UnArchive()
{
	if (bHasIODeviceOwnership && this->stream != nullptr)
	{
		delete this->stream;
	}
}

UnArchive *UnArchive::detectArchive(const QFileInfo &fi, QIODevice * &device)
{
	QFileInfo file = fi;
	if (file.suffix().compare("bz2", Qt::CaseInsensitive) == 0)
	{
		device = new BZ2Device(device);
		file = QFileInfo(file.completeBaseName());
	}
	else if (file.suffix().compare("gz", Qt::CaseInsensitive) == 0)
	{
		device = new GZDevice(device);
		file = QFileInfo(file.completeBaseName());
	}

	if (file.suffix().compare("zip", Qt::CaseInsensitive) == 0 ||
		file.suffix().compare("pk3", Qt::CaseInsensitive) == 0)
	{
		return new UnZip(device);
	}
	else if (file.suffix().compare("7z", Qt::CaseInsensitive) == 0 ||
		file.suffix().compare("pk7", Qt::CaseInsensitive) == 0)
	{
		return new Un7Zip(device);
	}
	else if (file.suffix().compare("tar", Qt::CaseInsensitive) == 0)
	{
		return new UnTar(device);
	}
	return nullptr;
}

UnArchive *UnArchive::openArchive(const QFileInfo &fi, QIODevice *dataStream)
{
	UnArchive *ret = detectArchive(fi, dataStream);
	return ret;
}

UnArchive *UnArchive::openArchive(const QString &filename)
{
	QFileInfo fi(filename);
	if (!fi.isReadable())
		return nullptr;

	QIODevice *stream = new QFile(filename);
	UnArchive *ret = detectArchive(fi, stream);
	if (ret != nullptr)
	{
		ret->bHasIODeviceOwnership = true;
		return ret;
	}

	delete stream;
	return nullptr;
}
