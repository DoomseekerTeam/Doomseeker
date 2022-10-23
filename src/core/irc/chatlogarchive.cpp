//------------------------------------------------------------------------------
// chatlogarchive.cpp
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "chatlogarchive.h"

#include "irc/chatlogs.h"
#include "irc/entities/ircnetworkentity.h"
#include <QDateTime>
#include <QDir>
#include <QFileInfo>

DClass<ChatLogArchive>
{
};

DPointered(ChatLogArchive)

ChatLogArchive::ChatLogArchive()
{
}

ChatLogArchive::~ChatLogArchive()
{
}

QString ChatLogArchive::archiveDirPath(const IRCNetworkEntity &network, const QString &recipient)
{
	ChatLogs logs;
	return QString("%1/backup/%2").arg(logs.networkDirPath(network), logs.logFileName(recipient));
}

QStringList ChatLogArchive::listArchivedLogsSortedByTime(const IRCNetworkEntity &network, const QString &recipient)
{
	return QDir(archiveDirPath(network, recipient)).entryList(QDir::Files, QDir::Time);
}

QString ChatLogArchive::mkArchiveFilePath(const IRCNetworkEntity &network, const QString &recipient)
{
	QDateTime time = QDateTime::currentDateTime();
	return QString("%1/%2.txt").arg(
		archiveDirPath(network, recipient), time.toString("yyyy-MM-dd_hh-mm-ss"));
}
