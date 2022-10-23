//------------------------------------------------------------------------------
// untar.cpp
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

#include "untar.h"

#include <QBuffer>
#include <QFile>
#include <QStringList>

UnTar::UnTar(QIODevice *device) : UnArchive(device), valid(true)
{
	scanTarFile();
}

UnTar::~UnTar()
{
}

bool UnTar::extract(int file, const QString &where)
{
	if (!stream->open(QIODevice::ReadOnly))
		return false;

	stream->seek(directory[file].offset);
	QByteArray fileData = stream->read(directory[file].size);
	stream->close();

	if (static_cast<unsigned>(fileData.size()) == directory[file].size)
	{
		QFile outputFile(where);
		outputFile.open(QFile::WriteOnly);
		outputFile.write(fileData);
		outputFile.close();
		return true;
	}
	return false;
}

QString UnTar::fileNameFromIndex(int file)
{
	if (file >= directory.size())
		return QString();
	return directory[file].filename;
}

QStringList UnTar::files()
{
	QStringList files;
	for (const TarFile &tarfile : directory)
	{
		files << tarfile.filename;
	}
	return files;
}

int UnTar::findFileEntry(const QString &entryName)
{
	for (int i = 0; i < directory.count(); i++)
	{
		if (directory[i].filename == entryName)
			return i;
	}
	return -1;
}

void UnTar::scanTarFile()
{
	TarFile file;
	char buffer[512];
	qint64 read;
	unsigned int offset = 512;

	if (!stream->open(QIODevice::ReadOnly))
	{
		valid = false;
		return;
	}

	while (valid && (read = stream->read(buffer, 512)) > 0)
	{
		if (read < 512)
		{
			valid = false;
			break;
		}

		file.filename = QString::fromUtf8(buffer, qMin(static_cast<int>(strlen(buffer)), 100));
		if (file.filename.isEmpty())
			break;
		file.size = QString(&buffer[124]).left(12).toUInt(&valid, 8);
		file.offset = offset;
		// Tar files are aligned along 512 blocks
		offset += 512 + file.size + (file.size % 512 != 0 ? 512 - (file.size % 512) : 0);
		stream->seek(offset - 512);

		directory.append(file);
	}
	stream->close();
}
