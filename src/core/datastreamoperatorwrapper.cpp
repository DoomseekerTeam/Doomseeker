//------------------------------------------------------------------------------
// datastreamoperatorwrapper.h
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "datastreamoperatorwrapper.h"

#include "strings.hpp"
#include <cassert>
#include <QIODevice>

#define RETTYPE(type) \
	type tmp; \
	(*d->s) >> tmp; \
	return tmp;

DClass<DataStreamOperatorWrapper>
{
public:
	QDataStream *s;
};

DPointered(DataStreamOperatorWrapper)

DataStreamOperatorWrapper::DataStreamOperatorWrapper(QDataStream *stream)
{
	assert(stream != nullptr);
	d->s = stream;
}

DataStreamOperatorWrapper::~DataStreamOperatorWrapper()
{
}

QDataStream *DataStreamOperatorWrapper::dataStream()
{
	return d->s;
}

const QDataStream *DataStreamOperatorWrapper::dataStream() const
{
	return d->s;
}

qint8 DataStreamOperatorWrapper::readQInt8()
{
	RETTYPE(qint8);
}

bool DataStreamOperatorWrapper::readBool()
{
	RETTYPE(bool);
}

quint8 DataStreamOperatorWrapper::readQUInt8()
{
	RETTYPE(quint8);
}

quint16 DataStreamOperatorWrapper::readQUInt16()
{
	RETTYPE(quint16);
}

qint16 DataStreamOperatorWrapper::readQInt16()
{
	RETTYPE(qint16);
}

quint32 DataStreamOperatorWrapper::readQUInt32()
{
	RETTYPE(quint32);
}

qint32 DataStreamOperatorWrapper::readQInt32()
{
	RETTYPE(qint32);
}

quint64 DataStreamOperatorWrapper::readQUInt64()
{
	RETTYPE(quint64);
}

qint64 DataStreamOperatorWrapper::readQInt64()
{
	RETTYPE(qint64);
}

float DataStreamOperatorWrapper::readFloat()
{
	RETTYPE(float);
}

double DataStreamOperatorWrapper::readDouble()
{
	RETTYPE(double);
}

QByteArray DataStreamOperatorWrapper::readRaw(qint64 length)
{
	return d->s->device()->read(length);
}

QByteArray DataStreamOperatorWrapper::readRawAll()
{
	return d->s->device()->readAll();
}

QByteArray DataStreamOperatorWrapper::readRawUntilByte(char stopByte)
{
	return readRawMaxUntilByte(stopByte, -1);
}

QByteArray DataStreamOperatorWrapper::readRawMaxUntilByte(char stopByte, qint64 length)
{
	QByteArray result;
	qint64 counter = 0;
	while (!d->s->atEnd() && (length < 0 || counter < length))
	{
		quint8 rByte;
		*d->s >> rByte;
		result += rByte;
		++counter;

		if (rByte == stopByte)
			break;
	}
	return result;
}

QString DataStreamOperatorWrapper::readUtf8CString()
{
	auto raw = readRawUntilByte('\0');
	// Qt6 no longer strips the null terminator when converting from QByteArray
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	if (raw.isEmpty())
		return {};
	if (raw.back() == '\0')
		return QString::fromUtf8(QByteArrayView(raw).chopped(1));
#endif
	return QString::fromUtf8(raw);
}

qint64 DataStreamOperatorWrapper::remaining() const
{
	return d->s->device()->size() - d->s->device()->pos();
}

int DataStreamOperatorWrapper::skipRawData(int len)
{
	return d->s->skipRawData(len);
}
