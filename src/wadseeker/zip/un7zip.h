//------------------------------------------------------------------------------
// un7zip.h
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

#ifndef __UN7ZIP_H__
#define __UN7ZIP_H__

#include <QByteArray>
#include <QIODevice>
#include <QObject>

#include "unarchive.h"

extern "C"
{
#include "7z.h"
#include "7zCrc.h"
#include "7zTypes.h"
}

class SZByteStream
{
public:
	// [BL] Ugh, I really don't see a way around copying this ZDoom hack.
	//      This must be the first variable.
	ISeekInStream stream;

	QIODevice *buffer;

	SZByteStream(QIODevice *buffer);
	~SZByteStream();

	static SRes Read(void *p, void *buf, size_t *size);
	static SRes Seek(void *p, Int64 *pos, ESzSeek origin);
};

class Un7Zip : public UnArchive
{
	Q_OBJECT

public:
	Un7Zip(QIODevice *device);
	~Un7Zip() override;

	bool extract(int file, const QString &where) override;
	QString fileNameFromIndex(int file) override;
	int findFileEntry(const QString &entryName) override;
	QStringList files() override;
	bool isValid() override
	{
		return valid;
	}


protected:
	static void *SzAlloc(void *p, size_t size);
	static void SzFree(void *p, void *address);

	void Init();

	SZByteStream *byteStream;
	CLookToRead lookStream;

	CSzArEx db;
	static ISzAlloc alloc;
	Byte *out;
	size_t outSize;

	bool valid;
};

#endif
