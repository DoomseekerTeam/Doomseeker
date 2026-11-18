//------------------------------------------------------------------------------
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//------------------------------------------------------------------------------
// Copyright (C) 2026 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_ODAMEX_QUERY_READER_H
#define DOOMSEEKER_ODAMEX_QUERY_READER_H

#include "datastreamoperatorwrapper.h"
#include <QByteArray>
#include <QDataStream>
#include <QString>

namespace OdamexQuery
{
inline QString readCString(DataStreamOperatorWrapper &in)
{
	const QByteArray raw = in.readRawUntilByte('\0');
	if (raw.isEmpty() || raw.at(raw.size() - 1) != '\0')
	{
		in.dataStream()->setStatus(QDataStream::ReadPastEnd);
		return {};
	}
	return QString::fromUtf8(raw.constData(), raw.size() - 1);
}

inline QByteArray readMd5(DataStreamOperatorWrapper &in)
{
	const quint8 length = in.readQUInt8();
	if (in.dataStream()->status() != QDataStream::Ok)
		return {};
	if (length != 0 && length != 16)
	{
		in.dataStream()->setStatus(QDataStream::ReadCorruptData);
		return {};
	}
	if (length == 0)
		return {};

	// readRaw() reads from the device and does not update QDataStream status.
	const QByteArray digest = in.readRaw(length);
	if (digest.size() != length)
		in.dataStream()->setStatus(QDataStream::ReadPastEnd);
	return digest;
}
}

#endif
