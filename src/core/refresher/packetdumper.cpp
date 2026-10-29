//------------------------------------------------------------------------------
// packetdumper.cpp
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
// Copyright (C) 2026 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "packetdumper.h"

#include "log.h"
#include <QDateTime>
#include <QDir>
#include <QTemporaryFile>

namespace
{
QString filenamePart(QString value)
{
	for (int i = 0; i < value.size(); ++i)
	{
		const QChar c = value[i];
		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9') || c == '.' || c == '-' || c == '_'))
		{
			value[i] = '_';
		}
	}
	return value;
}
}

QString PacketDumper::defaultDirectory()
{
	return QDir(QDir::tempPath()).filePath("doomseeker-packet-dumps");
}

QString PacketDumper::dump(const QHostAddress &address, quint16 port, Direction direction,
	const QByteArray &data, const Options &options)
{
	const QDir dir(options.directory.isEmpty() ? defaultDirectory() : options.directory);
	if (!dir.mkpath("."))
	{
		gLog << QString("Packet dump: cannot create directory '%1'.").arg(dir.absolutePath());
		return QString();
	}

	// Display IPv4 consistently even when a dual-stack socket reports it as
	// IPv4-mapped IPv6. Genuine IPv6 addresses retain their representation.
	const QHostAddress v4(address.toIPv4Address());
	const QString host = address.isEqual(v4, QHostAddress::ConvertV4MappedToIPv4) ?
		v4.toString() : address.toString();
	const QDateTime timestamp = options.timestamp.isValid() ?
		options.timestamp.toUTC() : QDateTime::currentDateTimeUtc();
	const QString name = QString("%1_%2_%3_%4_%5_XXXXXX.bin")
		.arg(filenamePart(host))
		.arg(port)
		.arg(direction == Direction::Recv ? "recv" : "sent")
		.arg(filenamePart(options.label))
		.arg(timestamp.toString("yyyyMMdd'T'HHmmsszzz'Z'"));
	// QTemporaryFile creates the file exclusively. The timestamp alone would
	// not distinguish several datagrams received in the same millisecond.
	QTemporaryFile file(dir.absoluteFilePath(name));
	if (!file.open())
	{
		gLog << QString("Packet dump: cannot create file in '%1': %2")
			.arg(dir.absolutePath(), file.errorString());
		return QString();
	}
	if (file.write(data) != data.size() || !file.flush())
	{
		gLog << QString("Packet dump: cannot write '%1': %2")
			.arg(file.fileName(), file.errorString());
		// Automatic removal remains enabled for incomplete captures.
		return QString();
	}

	file.close();
	file.setAutoRemove(false);
	gLog << QString("Packet dump saved: %1 (%2 bytes)").arg(file.fileName()).arg(data.size());
	return file.fileName();
}
